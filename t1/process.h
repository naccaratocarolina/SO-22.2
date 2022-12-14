#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef PROCESS
#define PROCESS

// Inicializacao da estrutura de um processo
typedef struct process {
  int Pid;
  int QueuePos; // Posicao na Queue
  int Arrival, ArrivalTime;
  int BurstTime, ExecTime, ExecTimeQueue;
  int ioBurstTime, ioFreq;
  int StartTime;
  int CompleteTime;
  int waiting;
  int TurnArountTime;
  int Ready;
  struct process* next;
} Process;

// Declaracao de funcoes
void arrivalSort (Process process[], int n);
void pidSort (Process process[], int n);
void initProcess (Process* process, int* clock, int ExecTime);
Process* pcopy (Process* process);

/**
 * Ordena a lista de processos de acordo com o ArrivalTime
 * @param process[] lista de processos
 * @param n quantidade de processos
 */
void arrivalSort (Process process[], int n) {
  Process temp;
  for (int i = 0; i < n; i++)
    for (int j = i + 1; j < n; j++)
      if (process[i].ArrivalTime > process[j].ArrivalTime) {
        temp = process[i];
        process[i] = process[j];
        process[j] = temp;
      }
}

/**
 * Ordena a lista de processos de acordo com o Pid
 * @param process[] lista de processos
 * @param n quantidade de processos
 */
void pidSort (Process process[], int n) {
  Process temp;
  for (int i = 0; i < n; i++)
    for (int j = i + 1; j < n; j++)
      if (process[i].Pid > process[j].Pid) {
        temp = process[i];
        process[i] = process[j];
        process[j] = temp;
      }
}

/**
 * Ordena a lista de processos de acordo com o StartTime
 * @param process[] lista de processos
 * @param n quantidade de processos
 */
void sortbyStartTime (Process process[], int n) {
  Process temp;
  for (int i = 0; i < n; i++)
    for (int j = i + 1; j < n; j++)
      if (process[i].StartTime > process[j].StartTime) {
        temp = process[i];
        process[i] = process[j];
        process[j] = temp;
      }
}

/**
 * Inicializa atributos do processo
 * @param process instancia de Processo
 * @param clock instante de tempo em que a funcao eh chamada
 * @param ExecTime instante em que as instru????es s??o executadas
 */
void initProcess (Process* process, int* clock, int ExecTime) {
  process->StartTime = *clock;
  process->CompleteTime = process->StartTime + ExecTime;
  process->TurnArountTime = process->CompleteTime - process->Arrival;
  process->waiting = process->TurnArountTime - (process->BurstTime - process->ExecTime);
}

/**
 * Copia os valores do processo em outro array
 * @param p instancia de um processo
 */
Process* pcopy (Process* p) {
  Process* temp = (Process*)malloc(sizeof(Process));
  temp->Pid = p->Pid;
  temp->QueuePos = p->QueuePos;
  temp->Arrival = p->Arrival;
  temp->BurstTime = p->BurstTime;
  temp->ArrivalTime = p->ArrivalTime;
  temp->ExecTime = p->ExecTime;
  temp->StartTime = p->StartTime;
  temp->CompleteTime = p->CompleteTime;
  temp->TurnArountTime = p->TurnArountTime;
  temp->waiting = p->waiting;
  temp->next = NULL;
  return temp;
}

#endif