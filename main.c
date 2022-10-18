#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<limits.h>

struct ProcessData {
  int Ppid;
  int Pid;
  int ArrivalTime;
  int BurstTime;
  int Priority;
  int WaitTime;
  int ResTime;
  int Finishtime;
  int RemainTime;
  int StartTime;
};

typedef struct ProcessData PD;
PD CurrentProcess;

void readInput (int count, PD* process) {
  for(int i=0; i<count; i++) {
    printf("Digite: <Pid> <ArrivalTime> <BurstTime> <Prioridade> para o processo #%d: ", i + 1);
    scanf("%d %d %d %d",
      &process[i].Pid, &process[i].ArrivalTime, &process[i].BurstTime, &process[i].Priority
    );
    // Armazena o input
    process[i].Ppid = i + 1;
    process[i].RemainTime = process[i].BurstTime;
  }
  puts("");
}

void printProcessData (int count, PD* process) {
  puts("Processos criados com sucesso:");
  for(int i=0; i<count; i++){
    printf("Processo #%d : Pid: %d | ArrivalTime: %d | BurstTime: %d | Priority: %d\n",
      process[i].Ppid, process[i].Pid, process[i].ArrivalTime, process[i].BurstTime, process[i].Priority
    );
  }
}

int arrivalSort (const PD* a, const PD* b) {
  return (*a).ArrivalTime - (*b).ArrivalTime;
}

bool pidSort (const PD* a, const PD* b) {
  return (*a).Pid > (*b).Pid;
}

bool ppidSort (const PD* a, const PD* b) {
  return (*a).Ppid > (*b).Ppid;
}


/* ==========================================================
   Queue
   ========================================================== */

struct queue {
  int head, tail, size;
  unsigned capacity;
  PD Data[10];
};

typedef struct queue Queue;

struct Queue* initQueue (unsigned capacity) {
  struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
  queue->capacity = capacity;
  queue->head = queue->size = 0;
  queue->tail = capacity - 1;
  return queue;
}

int isFull (struct Queue* queue) {
  return (queue->size == queue->capacity);
}

int isEmpty (struct Queue* queue) {
  return (queue->size == 0);
}

void enqueue (struct Queue* queue, PD process) {
  if (isFull(queue)) return;
  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->Data[queue->tail] = process;
  queue->size = queue->size + 1;
}

void dequeue (struct Queue* queue) {
  if (isEmpty(queue)) return;
  PD dequeuedProcessD = queue->Data[queue->head];
  queue->head = (queue->head + 1) % queue->capacity;
  queue->size = queue->size - 1;
}

PD getHead (struct Queue* queue) {
  if (isEmpty(queue)) return;
  return queue->Data[queue->head];
}

PD getTail (struct Queue* queue) {
  if (isEmpty(queue)) return;
  return queue->Data[queue->tail];
}

/* ==========================================================
   Priority Queue
   ========================================================== */

typedef struct node {
  PD data;
  int priority; // valores menores possuem maior prioridade
  struct node* next;
} Node;

Node* createNode (PD d, int p) {
  Node* temp = (Node*)malloc(sizeof(Node));
  temp->data = d;
  temp->priority = p;
  temp->next = NULL;
  return temp;
}

PD peek (Node** head) {
  return (*head)->data;
}

void pop(Node** head) {
  Node* temp = *head;
  (*head) = (*head)->next;
  free(temp);
}

void push (Node** head, PD d, int p) {
  Node* start = (*head);

  // Cria um novo nó
  Node* temp = createNode(d, p);

  // Caso Especial: A cabeça da lista tem menos prioridade que o novo nó. 
  // Portanto, inserie um novo nó antes do nó principal e altera a cabeça
  if ((*head)->priority > p) {
    // Insere novo nó antes da cabeça
    temp->next = *head;
    (*head) = temp;
  } else {
    // Percorre a lista e encontra a posição 
    // para inserir o novo n[ó]
    while (start->next != NULL && start->next->priority < p) {
      start = start->next;
    }
    temp->next = start->next;
    start->next = temp;
  }
}

int isEmptyPQ(Node** head) {
  return (*head) == NULL;
}

int main () {
  int numProcesses = 0;

  // Leitura e avaliacao dos parametros de entrada
  puts("Digite: <numero de processos>");
  scanf("%d", &numProcesses);
  printf("Numero de processos inserido: %d\n\n", numProcesses);

  // Aloca espaco para a quantidade de processos inserida
  PD *process, *processTemp;
  process = (PD*) calloc(numProcesses, sizeof(PD));
  if (process == NULL) {
    puts("--ERRO: malloc");
    return 1;
  }

  // Preenche dados dos processos
  readInput(numProcesses, process);
  printProcessData(numProcesses, process);
  processTemp = process;

  // Ordena baseado no arrival time
  qsort(process, numProcesses, sizeof(process[0]), arrivalSort);

  // Calcula o tempo total de execucao dos processos criados
  int totalExecutionTime = 0;
  totalExecutionTime += process[0].ArrivalTime;

  for(int i=0; i<numProcesses; i++) {
    if(totalExecutionTime >= process[i].ArrivalTime) {
      totalExecutionTime = totalExecutionTime + process[i].BurstTime;
    }
    else {
      int difference = (process[i].ArrivalTime - totalExecutionTime);
      totalExecutionTime = totalExecutionTime + difference + process[i].BurstTime;
    }
  }

  int ProcessesTAT[totalExecutionTime]; // turn around time
  for (int i =0; i<totalExecutionTime; i++) {
    ProcessesTAT[i] = -1;
  }
  printf("\nTempo total de execução: %d", totalExecutionTime);

  Queue* Q1 = initQueue(numProcesses);
  Queue* Q2 = initQueue(numProcesses);
  Node* PQ = NULL; //Priority Queue para I/O
  int cpuState = 0; // 0 para oscioso, 1 para ocupado
  int quantum1 = 5;
  int quantum2 = 8;

  CurrentProcess.Pid = -2;
  CurrentProcess.Priority = 999999;

  int Clock, Q1_process = 0, Q2_process = 0, PQ_process = 0;
  for(Clock=0; Clock < totalExecutionTime; Clock++) {
    
  }

  return 0;
}
