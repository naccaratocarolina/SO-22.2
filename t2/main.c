#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Tamanho da pagina
#define PAGE_SIZE 64

// Numero de threads a serem criadas
#define TLB_SIZE 16

// Quantidade de frames dedicados para programas de usuario
#define N_FRAMES 64

// Quantidade de frames dedicado a cada processo de usuario
#define N_WSL 4 // Working Set Limit

/* Variaveis globais */
pthread_mutex_t mutex;
FILE *arquivo; // Arquivos com dados de entrada (enderecos)
long int id_page; // Identificador da pagina na tabela de paginas
int cur_frame; // Frame atual
int found_tlb = 0, found_pt = 0; // Endereco encontrado na TLB ou tabela de paginas
int tlb[TLB_SIZE][2]; // Translation Lookaside Buffer (armazena os tids locais) --tlb[id_page][cur_frame]
int hits = 0, faults = 0; // Acertos e erros no acesso de paginas na TLB

// lru[i] = temp_i
//   i: frame i
//   temp_i: tempo gasto pelo frame i
int lru[N_FRAMES];
int lru_tlb[TLB_SIZE][2];

/* Handler das threads */
void *accessTLB (void *arg) {
  // Identificador da thread
  long int id_thread = (long int) arg;

  // --Inicio SC
  pthread_mutex_lock(&mutex);

  // Acessa a TLB. Se true, endereco encontrado na TLB
  if (tlb[id_thread][0] == id_page) {
    hits++;
    cur_frame = tlb[id_thread][1];
    found_tlb = 1; // Registra que o endereco foi encontrado na TLB
  }

  // --Fim SC
  pthread_mutex_unlock(&mutex);

  pthread_exit(NULL);
}

void updateLru (void) {
  for (int i=0; i<N_FRAMES; i++) {
    if (i == cur_frame) {
      // O cur_frame que acabou de ser usado é zerado
      lru[i] = 0;
    } else if (lru[i] != 1024) {
      // Os demais frames "envelhecem"
      lru[i]++;
    }
  }
}

void updateLruTlb (void) {
  for (long int i=0; i<TLB_SIZE; i++) {
    // Iguala o cur_frame tlb lru com o tlb atual
    lru_tlb[i][0] = tlb[i][1];
    if (tlb[i][1] == cur_frame) {
      // A idade do cur_frame adicionado é igualado a 0
      lru_tlb[i][1] = 0;
    } else if (lru_tlb[i][1] != 1024) {
      // Os demais frames "envelhecem"
      lru_tlb[i][1]++;
    }
  }
}

/* Funcao principal */
int main (int argc, char *argv[]) {
  pthread_t tid_tlb[TLB_SIZE]; // Identificadores das threads no sistema (ou seja, na TLB)
  int page_table[PAGE_SIZE]; // Tabela de paginas --page_table[id_page]
  int ram[N_FRAMES][N_FRAMES * 2]; // Memoria fisica
  signed char buffer[N_FRAMES * 2]; // Buffer

  // Leitura e avaliacao dos parametros de entrada
  if (argc < 2) {
    printf("Digite: %s <arquivo.txt>\n", argv[0]);
    return 1;
  }

  // Inicializa o mutex (lock de exclusao mutua) de forma
  // dinamica em tempo de execucao
  pthread_mutex_init(&mutex, NULL);

  // Inicializa estruturas
  memset(page_table, 0, N_FRAMES * sizeof(page_table[0]));
  for (int i=0; i<N_FRAMES; i++) {
    ram[i][0] = -1;
    lru[i] = 1024;
  }
  for (int i=0; i<TLB_SIZE; i++) {
    tlb[i][0] = -1;
    lru_tlb[i][1] = 1024;
  }

  // Abre arquivo com dados de entrada e faz tratamento de erro
  arquivo = fopen(argv[1], "r");
  if (arquivo == NULL) {
      printf("--ERRO: fopen: Arquivo não encontrado\n");
      return 2;
  }

  int address;
  long long offset;
  int logical_address = 0, physical_address = 0;
  int translated = 0; // Contador de enderecos examinados (ou seja, traduzidos)
  int j = 0; // Variavel de controle
  // Variaveis para o lru
  int value = 0, least_occurred_frame = 0, older = -1, id_older = 0;

  while ((fscanf(arquivo, "%d", &address) != EOF)) {
    // Inicializa tradução de endereços
    translated++;

    logical_address = address;

    offset = address;
    offset = offset & 127;

    id_page = address;           
    id_page = id_page >> 8;
    id_page = id_page & 127;

    found_tlb = 0, found_pt = 0;

    // Cria as threads
    for (long int i=0; i<TLB_SIZE; i++) {
      if (pthread_create(&tid_tlb[i], NULL, accessTLB, NULL)) {
        fprintf(stderr, "--ERRO: pthread_create\n");
        return 3;
      }
    }

    // Aguarda o termino das threads
    for (long int i=0; i<TLB_SIZE; i++) {
      if (pthread_join(tid_tlb[i], NULL)) {
        fprintf(stderr, "--ERRO: pthread_join\n");
        return 3;
      }
    }

    // Caso o endereco requisitado nao seja encontrado na TLB
    if (found_tlb == 0) {
      // Procura na tabela de paginas
      for (int i=0; i<PAGE_SIZE; i++) {
        // Acessa a Tabela de paginas. Se true, endereco
        // encontrado na Tabela de paginas
        if (page_table[i] == id_page) {
          cur_frame = i;
          found_pt = 1;
          break;
        }
      }

      // Houve page fault
      faults++;
      older = -1;
      for (int i=0; i<N_FRAMES; i++) {
        // Busca o mais velho
        if (lru[i] > older) {
          older = lru[i];
          id_older = i;
        }
      }
      cur_frame = id_older;

      // Atualiza TLB
      older = -1, j = 0;
      for (long int i=0; i<TLB_SIZE; i++) {
        if (lru_tlb[i][1] > older) {
          older = lru_tlb[i][1];
          id_older = i;
        }
      }
      tlb[id_older][0] = id_page; // Substitui pelo mais velho
      tlb[id_older][1] = cur_frame;
    }

    // Atualiza filas
    updateLru();
    updateLruTlb();

    page_table[cur_frame] = id_page; // Salva na tabela de paginas
    physical_address = cur_frame * (N_FRAMES * 2) + offset; // Calcula o endereço fisico
    value = ram[cur_frame][offset];
    printf("Endereço virtual: %d Endereço fisico: %d Valor: %d\n", logical_address, physical_address, value);
  }
  
  double pagerate = (double) faults / translated;
  double tblrate = (double) hits / translated;
  printf("Numero de endereços traduzidos: %d\n", translated);
  printf("Page Faults: %d\nTaxa de Erro na Tabela de Paginas: %.3f\nAcertos TLB: %d\nTaxa de acerto na TLB %.3f\n",
      faults, pagerate, hits, tblrate);

  // Desaloca variaveis e termina
  pthread_mutex_destroy(&mutex);
  fclose(arquivo);

  return 0;
}