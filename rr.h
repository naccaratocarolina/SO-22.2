#include "display.h"
#include "process.h"
#include "queue.h"
#include <stdio.h>

#ifndef RR
#define RR

/**
 * @param q lista de processos
 * @param ioQueue lista de io
 * @param sum variavel acumuladora que armazena o tempo total de execucao
 * @param pb fatia de tempo do processo na CPU
 * @param execfor tempo de execucao do processo
 */
void rr (Queue q, Queue* gnatt, int* clock, int* sum, int* pb, int execfor) {
  int exec = q.head->ExecTime;
  if (q.quantum < exec) exec = q.quantum;

  // Calcula tempo minimo de execucao
  if (execfor == 0) {
    if (q.head->ExecTimeQueue <= 0) q.head->ExecTimeQueue = exec;
  } else {
    if (execfor < exec) exec = execfor;
  }
  if (exec > q.head->ioFreq && q.head->ioFreq > 0) exec = q.head->ioFreq;

  q.head->ExecTime -= exec;
  q.head->ExecTimeQueue -= exec;
  q.head->QueuePos = q.QueuePos;

  // Adiciona copia do processo na fila atual
  Process* temp = pcopy(q.head);
  initProcess(temp, clock, exec);
  if (exec > 0) enqueue(gnatt, temp);

  // Atualiza variaveis
  *sum += exec;
  *clock += exec;
  *pb -= exec;
}

#endif