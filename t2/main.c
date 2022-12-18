#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define FRAME_SIZE 4096     // tamanho de quadro de página (frame) igual a 4KB, tamanho de uma página é igual a de um frame
                            // tamanho de uma página é igual a de um frame
#define TLB_SIZE 4          // TLB comporta 4 frames
#define RAM_SIZE 64         // a memória é limitada em 64 frames (256KB)
#define PAGE_TABLE_SIZE 128 // tamnho da tabela da páginas 128 frames (512KB, endereços de 19 bits, 0 à 524287)

/* Variaveis globais */
int page_table[PAGE_TABLE_SIZE]; // Tabela de paginas --page_table[id_page]
int tlb[TLB_SIZE][2];            // Translation Lookaside Buffer (armazena os tids locais) --tlb[id_page][cur_frame]
signed char buffer[RAM_SIZE];    // Buffer
int ram[FRAME_SIZE][RAM_SIZE];   // Memoria fisica
int mapped_frames[RAM_SIZE];
int virtual_address_list[PAGE_TABLE_SIZE * FRAME_SIZE];
pthread_t tid_tlb[TLB_SIZE]; // Identificadores das threads no sistema
int lru[PAGE_TABLE_SIZE];    // index = frame, conteudo = tempo que foi utilizado
int lru_tlb[TLB_SIZE][2];    // 0 = frame, 1 = tempo que foi utilizado

int page_id; // Identificador da pagina na tabela de paginas
int found_tlb = 0, hits = 0, frame;
int frame_id = 0, thread_id = 0, tlb_id = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Inicializa o mutex (lock de exclusao mutua)

/* Handler das threads */
void *accessTLB(void *arg)
{
  // --Inicio SC
  pthread_mutex_lock(&mutex);

  // Acessa a TLB. Se true, address encontrado na TLB
  if (tlb[thread_id][0] == page_id)
  {
    hits++;
    frame = tlb[thread_id][1];
    found_tlb = 1; // Registra que o address foi encontrado na TLB
  }
  thread_id++;

  // --Fim SC
  pthread_mutex_unlock(&mutex);
  return arg;
}

void updateLru(void)
{
  for (int i = 0; i < PAGE_TABLE_SIZE; i++)
  {
    if (i == frame)
    {
      // O frame que acabou de ser usado é zerado
      lru[i] = 0;
    }
    else if (lru[i] != 1024)
    {
      // Os demais frames "envelhecem"
      lru[i]++;
    }
  }
}

void updateLRUTLB(void)
{
  for (int i = 0; i < TLB_SIZE; i++)
  {
    // Iguala o frame tlb lru com o tlb atual
    lru_tlb[i][0] = tlb[i][1];
    if (tlb[i][1] == frame)
    {
      // A idade do frame adicionado é igualado a 0
      lru_tlb[i][1] = 0;
    }
    else if (lru_tlb[i][1] != 1024)
    {
      // Os demais frames "envelhecem"
      lru_tlb[i][1]++;
    }
  }
}

int acessVirtualAdress(int offset)
{
  int adress, older = 0, older_id = 0;
  found_tlb = 0, thread_id = 0;

  /* Verifica se está presente na TLB */

  // Cria as threads
  for (int i = 0; i < TLB_SIZE; i++)
  {
    if (pthread_create(&(tid_tlb[i]), NULL, accessTLB, NULL))
    {
      fprintf(stderr, "--ERRO: pthread_create\n");
      return 3;
    }
  }

  // Aguarda o termino das threads
  for (int i = 0; i < TLB_SIZE; i++)
  {
    if (pthread_join(tid_tlb[i], NULL))
    {
      fprintf(stderr, "--ERRO: pthread_join\n");
      return 3;
    }
  }

  // acessa tabela de páginas
  if (found_tlb == 0)
    frame = page_table[page_id];

  // Houve page fault
  if (frame == -1)
  {
    older = -1;
    // Busca o mais velho
    for (int i = 0; i < PAGE_TABLE_SIZE; i++)
    {
      if (lru[i] > older)
      {
        older = lru[i];
        older_id = i;
      }
    }

    for (int i = 0; i < RAM_SIZE; i++)
    {
      if (mapped_frames[i] == 0)
      {
        frame = mapped_frames[i];
        break;
      }
      else if (i == RAM_SIZE - 1)
        return -3;
    }
    lru[older_id] = 0;
    page_table[older_id] = frame;
  }

  // Atualiza TLB
  if (found_tlb == 0)
  {
    older_id = 0;
    older = -1;
    for (int i = 0; i < TLB_SIZE; i++)
    {
      if (lru_tlb[i][1] > older)
      {
        older = lru_tlb[i][1];
        older_id = i;
      }
    }
    tlb[older_id][0] = page_id; // Substitui pelo mais velho
    tlb[older_id][1] = frame;
  }

  // resolve enderço físico
  adress = offset + (frame * FRAME_SIZE);
  return adress;
}

void init()
{
  memset(page_table, -1, sizeof(page_table));
  for (int i = 0; i < RAM_SIZE; i++)
    mapped_frames[i] = 0;
  for (int i = 0; i < PAGE_TABLE_SIZE; i++)
  {
    lru[i] = 1024;
    ram[i][0] = -1;
  }
  for (int i = 0; i < TLB_SIZE; i++)
  {
    lru_tlb[i][1] = 1024;
    tlb[i][0] = -1;
  }
  for (int i = 0; i < FRAME_SIZE * PAGE_TABLE_SIZE; i++)
    virtual_address_list[i] = i;
}

/* Funcao principal */
int main(int argc, char *argv[])
{
  int offset,
      logical_address, physical_address;
  int valor = 0;

  // Inicializa estruturas
  init();

  /**
   * Um processo vai tentar alocar endereço, o endereço vai ser mateado na ram
   */

  for (int i = 0; i < 1; i++)
  {
    // logical_address = randint((128 * FRAME_SIZE) - 1); // process faz isso
    logical_address = 4096;
    page_id = logical_address / FRAME_SIZE;
    offset = logical_address / PAGE_TABLE_SIZE;
    physical_address = acessVirtualAdress(offset);
    if (physical_address == -3)
      exit(0);

    updateLru();
    updateLRUTLB();

    page_table[page_id] = frame; // Salva na tabela de paginas
    valor = ram[offset][frame];  // Obtem o valor
    printf("virtual_address: %d page_index: %d frame: %d physical_address: %d value: %d\n", logical_address, page_id, frame, physical_address, valor);
  }

  // Desaloca variaveis e termina
  pthread_mutex_destroy(&mutex);

  return 0;
}