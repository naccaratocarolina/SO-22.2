// include libraries
#include "mlfq.h"
#include "process.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

// Declaracao de funcoes
void getProcess (FILE* fp, Process* process);
void getQueues (FILE* fp, Queue* queue);
int checkProcesses (Process process[], int n);
int checkQueues (Queue queue[], int n);

int main() {
  char filename[FILENAME_MAX];
  FILE* fp;

  printf("Digite o arquivo de entrada: ");
  scanf("%s", filename);

  // Abre e le o arquivo
  fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("%s not found.\n", filename);
    exit(EXIT_FAILURE);
  }

  // Armazena o numero de filas, numero de processos e
  // priority boost
  int xys[3];
  fscanf(fp, "%d %d %d", &xys[0], &xys[1], &xys[2]);
  if (xys[2] == 0)
    xys[2] = 1;

  // Armazena x filas
  Queue queue[xys[0]];
  for (int i = 0; i < xys[0]; i++) {
    getQueues(fp, &queue[i]);
  }

  // Armazena y processos
  Process process[xys[1]];
  for (int i = 0; i < xys[1]; i++) {
    getProcess(fp, &process[i]);
  }

  // Fecha o arquivo
  fclose(fp);

  if (xys[0] < 2 || xys[0] > 5)
    printf("Há muitas/poucas filas (2, 5).\n");
  else if (xys[1] < 3 || xys[1] > 100)
    printf("O número de processos deve estar no intervalo (3, 100).\n");
  else if (xys[2] < 0)
    printf("O período de aumento de prioridade (PriorityBoost) não deve ser negativo\n");
  else if (checkProcesses(process, xys[1])) {
  } else if (checkQueues(queue, xys[0])) {
  } else {
    prioritySort(queue, xys[0]);
    mlfq(queue, xys[0], process, xys[1], xys[2]);
  }

  return 0;
}

void getProcess (FILE* fp, Process* process) {
  if (fscanf(fp, "%d", &process->Pid) == EOF || fscanf(fp, "%d", &process->Arrival) == EOF || fscanf(fp, "%d", &process->BurstTime) == EOF || fscanf(fp, "%d", &process->ioBurstTime) == EOF || fscanf(fp, "%d", &process->ioFreq) == EOF) {
    printf("O número de valores listados é menor que o valor especificado.\n");
    exit(EXIT_FAILURE);
  } else {
    process->ExecTime = process->BurstTime;
    process->ArrivalTime = process->Arrival;
    process->next = NULL;
    process->Ready = 0;
    process->ExecTimeQueue = 0;
  }
}

/**
 * @param fp ponteiro para o arquivo
 * @param queue fila de processos
 */
void getQueues (FILE* fp, Queue* queue) {
  if (fscanf(fp, "%d", &queue->QueuePos) == EOF ||
      fscanf(fp, "%d", &queue->priority) == EOF ||
      fscanf(fp, "%d", &queue->quantum) == EOF) {
    printf("O número de valores listados é menor que o valor especificado.\n");
    exit(EXIT_FAILURE);
  } else {
    queue->head = NULL;
    queue->tail = NULL;
  }
}

/**
 * Verifica se o input dos dados dos processos sao validos
 * @param process[] lista de processos
 * @param n numero de processos
 */
int checkProcesses (Process process[], int n) {
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++)
      if (process[i].Pid == process[j].Pid) {
        printf("Os PIDs não devem ser os mesmos.\n");
        return 1;
      }
    if (process[i].Pid < 0 ||
        process[i].Arrival < 0 ||
        process[i].ioBurstTime < 0 ||
        process[i].ioFreq < 0 ||
        process[i].BurstTime < 0) {
      printf("Os valores do processo não devem ser negativos.\n");
      return 1;
    }
  }
  return 0;
}

int checkQueues (Queue queue[], int n) {
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++)
      if (queue[i].QueuePos == queue[j].QueuePos ||
          queue[i].priority == queue[j].priority) {
        printf("QueuePos e prioridades devem possuir valores diferentes.\n");
        return 1;
      }
    if (queue[i].QueuePos < 0 ||
        queue[i].priority < 0 ||
        queue[i].quantum < 0) {
      printf("Os valores da fila não devem ser negativos.\n");
      return 1;
    }
  }
  return 0;
}