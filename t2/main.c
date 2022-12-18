#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define FRAME_SIZE 4096     // tamanho de quadro de página (frame) igual a 4KB, tamanho de uma página é igual a de um frame
                            // tamanho de uma página é igual a de um frame
#define TLB_SIZE 4          // TLB comporta 4 frames
#define RAM_SIZE 64         // a memória é limitada em 64 frames (256KB)
#define PAGE_TABLE_SIZE 128 // tamnho da tabela da páginas 128 frames (512KB, endereços de 19 bits, 0 à 524287)
#define PROCESS 20          // Número de processos
#define WORK_SET_LIMIT 4    // Limite de páginas em memória pro processo

typedef struct linkedList
{
  int data[2];
  struct linkedList *previus;
  struct linkedList *next;
} Node;

typedef struct process
{
  int id;
  int work_set[4][2]; // int[page_id][time_count]
  int work_set_count;
  int pages;
  Node *swap;
  int swap_count;
} Process;

/* Variaveis globais */
int page_table[PAGE_TABLE_SIZE]; // Tabela de paginas --page_table[id_page]
int tlb[TLB_SIZE][2];            // Translation Lookaside Buffer (armazena os tids locais) --tlb[id_page][cur_frame]
signed char buffer[RAM_SIZE];    // Buffer
int ram[FRAME_SIZE][RAM_SIZE];   // Memoria fisica
int mapped_frames[RAM_SIZE];
pthread_t tid_tlb[TLB_SIZE]; // Identificadores das threads no sistema
int lru[PAGE_TABLE_SIZE];    // index = frame, conteudo = tempo que foi utilizado
int lru_tlb[TLB_SIZE][2];    // 0 = frame, 1 = tempo que foi utilizado

Process process[PROCESS]; // Lista d processos

int page_id; // Identificador da pagina na tabela de paginas
int found_tlb = 0, hits = 0, frame;
int frame_id = 0, thread_id = 0, tlb_id = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Inicializa o mutex (lock de exclusao mutua)

int acessVirtualAdress(int offset);
void createNewPage(int processId);
void swappingAndCreatePage(int processId);
void tableOutput(int processId);
void init();

void delay(int milliseconds)
{
  long pause;
  clock_t now, then;

  pause = milliseconds * (CLOCKS_PER_SEC / 1000);
  now = then = clock();
  while ((now - then) < pause)
    now = clock();
}

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
        frame = i;
        mapped_frames[i] = 1;
        break;
      }
      else if (i == RAM_SIZE - 1)
        return -3;
    }
    lru[older_id] = 0;
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
}

void tableOutput(int processId)
{
  printf("Processo %d ", processId);
  printf("---------------------------------------------------------------------------------------------\n");
  printf("ID\t-\tFRAME\t|\tID\t-\tFRAME\t|\tID\t-\tFRAME\t|\tID\t-\tFRAME\n");
  int sv = PAGE_TABLE_SIZE / 4;
  for (int i = 0; i < sv; i++)
  {
    printf("%d\t-\t%d\t|\t", i, page_table[i]);
    printf("%d\t-\t%d\t|\t", i + sv, page_table[i + sv]);
    printf("%d\t-\t%d\t|\t", i + 2 * sv, page_table[i + 2 * sv]);
    printf("%d\t-\t%d\n", i + 3 * sv, page_table[i + 3 * sv]);
  }

  printf("Obs: Frames com valores -1 não estão mapeados (presentes) na memoria virtual.\n");
  printf("--------------------------------------------------------------------\n");
}

void createNewPage(int processId)
{
  int offset, physical_address, logical_address, valor = 0;

  do
  {
    logical_address = rand() % PAGE_TABLE_SIZE * FRAME_SIZE;
    page_id = logical_address / FRAME_SIZE;
  } while (page_table[page_id] != -1);

  offset = logical_address / PAGE_TABLE_SIZE;

  physical_address = acessVirtualAdress(offset);

  if (physical_address == -3)
  {
    // printf("Não há memória suficiente para concluir esta operação.\n");
    swappingAndCreatePage(processId);
    // exit(0);
  }

  updateLru();
  updateLRUTLB();

  page_table[page_id] = frame; // Salva na tabela de paginas
  valor = ram[offset][frame];  // Obtem o valor

  printf("virtual_address: %d page_index: %d frame: %d physical_address: %d value: %d\n", logical_address, page_id, frame, physical_address, valor);
  printf("--------------------------------------------------------------------\n");
}

void swappingAndCreatePage(int processId)
{
  Node *new, *temp;
  int longerTime = -1, longerTimeId = 0, old_page_id, old_mapped_frame, time;
  int work_set_count = process[processId].work_set_count;

  new = (Node *)malloc(sizeof(Node));
  temp = (Node *)malloc(sizeof(Node));

  for (int w = 0; w < work_set_count; w++)
  {
    time = process[processId].work_set[w][1];
    // new->data = process[processId].work_set[processId];
    memcpy(new->data, process[processId].work_set[w], sizeof(process[processId].work_set[w]));
    new->next = NULL;
    new->previus = NULL;
    if (time > longerTime)
    {
      longerTime = time;
      longerTimeId = w;
    }
  }

  // adiciona à swap quando a swap está vazia
  if (process[processId].swap == NULL)
    process[processId].swap = new;
  else
  {
    temp = process[processId].swap; // adiciona a referencia da swap para temp
    while (temp->next != NULL)
      temp = temp->next; // busca ultimo elemento lincado
    new->previus = temp; // linca penultima página como previus
    temp->next = new;    // linca últoma página como next.
  }

  // remove página da memória
  old_page_id = process[processId].work_set[longerTimeId][0];
  old_mapped_frame = page_table[old_page_id];
  mapped_frames[old_mapped_frame] = 0;
  createNewPage(process[processId].id);
  page_table[old_page_id] = -1;
  process[processId].work_set[longerTimeId][0] = page_id;
  process[processId].work_set[longerTimeId][1] = 0;
  process[processId].swap_count++;
}

/* Funcao principal */
int main(int argc, char *argv[])
{
  int completed_process = 0;

  // Inicializa estruturas
  init();

  // Criando processos
  for (int i = 0; i < PROCESS; i++)
  {
    process[i].id = i;
    process[i].work_set_count = 0;
    // createNewPage(i);
    // process[i].work_set[0][0] = page_id;
    process[i].work_set[0][1] = 0;
    for (int j = 0; j < WORK_SET_LIMIT; j++)
    {
      process[i].work_set[j][0] = -1;
      process[i].work_set[j][1] = 0;
    }
    process[i].pages = 10;
    process[i].swap = NULL;
    process[i].swap_count = 0;
    // delay(3000);
  }

  while (completed_process != PROCESS)
  {
    int work_set_count;

    for (int i = 0; i < PROCESS; i++)
    {
      if (process[i].swap_count + process[i].work_set_count >= process[i].pages)
      {
        completed_process++;
        continue;
      }

      work_set_count = process[i].work_set_count;
      // atualiza tempo da página na memória
      for (int w = 0; w < work_set_count; w++)
        process[i].work_set[w][1]++;

      if (process[i].work_set_count < WORK_SET_LIMIT)
      {
        process[i].work_set_count++;

        tableOutput(i);
        createNewPage(i);

        process[i].work_set[work_set_count][0] = page_id;
        process[i].work_set[work_set_count][1] = 0;
      }
      else
      {
        tableOutput(i);
        swappingAndCreatePage(i);
      }
      delay(3000);
    }
  }

  tableOutput(PROCESS);

  // Desaloca variaveis e termina
  pthread_mutex_destroy(&mutex);

  return 0;
}