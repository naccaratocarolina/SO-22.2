#include "display.h"
#include "process.h"
#include "queue.h"
#include "rr.h"
#include <stdio.h>

#ifndef IO
#define IO

/**
 * Calcula a fatia de tempo que o io vai ocupar na CPU (IO burst).
 * Nesta implementação, o IO não se sobrepõe.
 * @param q lista de processos
 * @param ioQueue lista de io
 * @param io atual
 * @param i indice do processo atual
 * @param x quantidade de processos
 * @param clock instante de tempo
 * @param sum variavel acumuladora que armazena o tempo total de execucao
 * @param pb fatia de tempo do processo na CPU
 * @param pdone verifica se o processo atual esta executando
 */
int ioBurstTime(Queue q[], Queue gnatt[], Queue* io, int i, int x, int* clock, int* sum, int* pb, int* pdone) {
  // Se o processo não terminar de executar e tiver io BurstTime
  if (q[i].head->ExecTime > 0
      && q[i].head->ioBurstTime > 0
      && q[i].head->ioFreq <= q[i].quantum) {

    // Adiciona processo à fila de io se o tempo de execução for maior que 0
    Process* temp = pcopy(q[i].head);
    initProcess(temp, clock, q[i].head->ioBurstTime);
    if (temp->CompleteTime - temp->StartTime > 0) enqueue(io, temp);

    // Se pdone for 0, o processo começou a ser executado na fila atual
    if (*pdone == 0) {
      // Processo na fila io
      Process* ioexec = q[i].head;

      // Índice da fila que será executada durante o tempo de ioexec na fila io
      int QueuePosx = (i == x - 1 || q[i].head->next != NULL) ? i : i + 1;
      if (q[QueuePosx].head == ioexec) { enqueue(&q[QueuePosx], dequeue(&q[QueuePosx])); }
      if (q[QueuePosx].head != NULL) {
        // Continua executando processos enquanto o io ainda está acontecendo
        if (*clock < temp->CompleteTime) {
          int nQueuePosx = (i == x - 1 || q[QueuePosx].head->next == NULL) ? i : i + 1;
          rr(q[QueuePosx], &gnatt[QueuePosx], clock, sum, pb, temp->CompleteTime - *clock);
          if (ioexec->ExecTimeQueue <= 0) enqueue(&q[(i == x - 1) ? i : i + 1], dequeue(&q[i]));
          if (ioexec->ExecTimeQueue <= 0 || ioBurstTime(q, gnatt, io, QueuePosx, x, clock, sum, pb, pdone)) {
            *pdone = 1;
            return 1;
          }
          enqueue(&q[nQueuePosx], dequeue(&q[QueuePosx]));
        }
      }

      // Tempo de processo na fila atual ainda não concluído
      *pdone = 1;
      return 1;
    }
  }

  // Tempo de processo na fila atual ainda não concluído
  *pdone = 0;
  return 0;
}

#endif