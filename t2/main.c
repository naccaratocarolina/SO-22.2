#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define PAGE_SIZE 64
#define TLB_SIZE 32
#define FRAME_SIZE 64
#define WORKING_SET_LIMIT 4
#define RAM_SIZE FRAME_SIZE * WORKING_SET_LIMIT

FILE *file; // Arquivos com dados de entrada (enderecos)

/* Variaveis globais */
int page_table[PAGE_SIZE]; // Tabela de paginas --page_table[id_page]
int tlb[TLB_SIZE][2]; // Translation Lookaside Buffer (armazena os tids locais) --tlb[id_page][cur_frame]
signed char buffer[RAM_SIZE]; // Buffer
int ram[FRAME_SIZE][RAM_SIZE]; // Memoria fisica
pthread_t tid_tlb[TLB_SIZE]; // Identificadores das threads no sistema
int lru[FRAME_SIZE]; // index = frame, conteudo = tempo que foi utilizado
int lru_tlb[TLB_SIZE][2]; // 0 = frame, 1 = tempo que foi utilizado

long long page_id; // Identificador da pagina na tabela de paginas
int found_tlb = 0, hits = 0, frame; 
int frame_id = 0, thread_id = 0, tlb_id = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Inicializa o mutex (lock de exclusao mutua)

/* Handler das threads */
void *accessTLB (void *arg)  {
  // --Inicio SC   
  pthread_mutex_lock(&mutex);

  // Acessa a TLB. Se true, address encontrado na TLB
  if (tlb[thread_id][0] == page_id) {
    hits++;
    frame = tlb[thread_id][1];
    found_tlb = 1; // Registra que o address foi encontrado na TLB
  }
  thread_id++;

  // --Fim SC
  pthread_mutex_unlock(&mutex);
  return arg;
}

void updateLru (void)  {
  for (int i = 0; i < FRAME_SIZE; i++) {
    if (i == frame) {
      // O frame que acabou de ser usado é zerado
      lru[i] = 0;
    } else if (lru[i] != 1024) {
      // Os demais frames "envelhecem"
      lru[i]++;
    }
  }
}

void atualizaLRUTLB (void)  {
  for (int i = 0; i < TLB_SIZE; i++) {
    // Iguala o frame tlb lru com o tlb atual
    lru_tlb[i][0] = tlb[i][1];
    if (tlb[i][1] == frame) {
      // A idade do frame adicionado é igualado a 0
      lru_tlb[i][1] = 0;
    } else if (lru_tlb[i][1] != 1024) {
      // Os demais frames "envelhecem"
      lru_tlb[i][1]++;
    }
  }
}

/* Funcao principal */
int main(int argc, char *argv[]) {
  // Leitura e avaliacao dos parametros de entrada
  if (argc < 2) {
    printf("Digite: %s <file.txt>\n", argv[0]);
    return 1;
  }

  // Inicializa estruturas
  memset(page_table, -1, sizeof(page_table));
  for (int i = 0; i < FRAME_SIZE; i++) {
    lru[i] = 1024;
    ram[i][0] = -1;
  }
  for (int i = 0; i < TLB_SIZE; i++) {
    lru_tlb[i][1] = 1024;
    tlb[i][0] = -1;
  }

  // Abre file com dados de entrada e faz tratamento de erro
  file = fopen(argv[1], "r");
  if (file == NULL) {
      printf("--ERRO: fopen: Arquivo não encontrado\n");
      return 2;
  }
    
    if (file == NULL) {
      printf("Arquivo não encontrado\n");
      return 0;
  }

  int address;
  long long offset;
  int translated = 0, logical_address, j = 0, physical_address = 0;
  int faults = 0, found_pt = 0;
  int valor = 0, older = -1, older_id = 0; 

  while ((fscanf(file, "%d", &address) != EOF)) {
    // Inicializa tradução de endereços
    translated++;

    logical_address = address;

    offset = address;
    offset = offset & 255;

    page_id = address;           
    page_id = page_id >> 6;
    page_id = page_id & 255;

    found_tlb = 0, found_pt = 0, thread_id = 0;

    // Cria as threads
    for (int i = 0; i < TLB_SIZE; i++) {
      if (pthread_create(&(tid_tlb[i]), NULL, accessTLB, NULL)) {
        fprintf(stderr, "--ERRO: pthread_create\n");
        return 3;
      }
    }

    // Aguarda o termino das threads
    for (int i = 0; i < TLB_SIZE; i++) {
      if (pthread_join(tid_tlb[i], NULL)) {
        fprintf(stderr, "--ERRO: pthread_join\n");
        return 3;
      }
    }

    // Caso o endereco requisitado nao seja encontrado na TLB
    if (found_tlb == 0) {
      // Procura na tabela de paginas
      for (int i = 0; i < PAGE_SIZE; i++) {
        // Acessa a Tabela de paginas. Se true, endereco
        // encontrado na Tabela de paginas
        if (page_table[i] == page_id) {
          frame = i;
          found_pt = 1;
          break;
        }
      }
      // Houve page fault
      faults++;
      older = -1;
      for (int i = 0; i < FRAME_SIZE; i++) {
        // Busca o mais velho
        if (lru[i] > older) {
          older = lru[i];
          older_id = i;
        }
      }

      // Atualiza TLB
      frame = older_id;
      older = -1, j = 0;
      for (int i = 0; i < TLB_SIZE; i++) {
        if (lru_tlb[i][1] > older) {
          older = lru_tlb[i][1];
          older_id = i;
        }  
      }
      tlb[older_id][0] = page_id; // Substitui pelo mais velho
      tlb[older_id][1] = frame;
    }
    updateLru();
    atualizaLRUTLB();

    page_table[frame] = page_id; // Salva na tabela de paginas
    physical_address = frame * 256 + offset; // Calcula o endereço fisico
    valor = ram[frame][offset]; // Obtem o valor
    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, valor);
  }
  // Calcula estatisticas
  double tblrate = (double) hits / translated;
  double pagerate = (double) faults / translated;
  printf("Number of Translated Addresses = %d\nPage Faults = %d\nPage Fault Rate = %.3f\nTLB Hits = %d\nTLB Hit Rate = %.3f", 
    translated, faults, pagerate, hits, tblrate);

  // Desaloca variaveis e termina
  pthread_mutex_destroy(&mutex);
  fclose(file);

  return 0;
}