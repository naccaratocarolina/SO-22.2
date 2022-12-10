#include "display.h"
#include "io.h"
#include "process.h"
#include "queue.h"
#include "rr.h"
#include <stdio.h>

void getAverageWaitTime (Queue q[], int x, int y, float* awt);

/**
 * @param queue[] fila de io
 * @param x numero de filas
 * @param process[] fila dos demais processos
 * @param y quantidade de processos
 * @param pboost fatia de tempo dos demais processos na CPU
 */
void mlfq (Queue queue[], int x, Process process[], int y, int pboost) {
  int clock = 0, pb = pboost, rpro = y, run = 1, pdone = 0;
  float awt = 0;

  // Inicializa filas
  Queue Ready = initQueue(-1); // fila de prontos
  Queue gnatt[x + 1]; // fila de io
  for (int i = 0; i < x; i++)
    gnatt[i] = initQueue(queue[i].QueuePos);
  gnatt[x] = initQueue(-1);

  // Ordena pelo ArrivalTime
  arrivalSort(process, y);

  // Inicia o clock
  while (run) {
    int flag = 0, sum = 0, i = 0;
    while (i != x) {

      // Verifica se há processos recém-chegados
      int newpro = 0;
      for (int i = 0; i < y; i++) {
        if (process[i].Arrival <= clock && process[i].Ready == 0) {
          rpro = rpro - 1, newpro = 1, process[i].Ready = 1;
          enqueue(&Ready, &process[i]);
        }
      }

      // Adiciona processos recém-chegados à fila de prioridade mais alta
      while (Ready.head != NULL) {
        enqueue(&queue[0], dequeue(&Ready));
      }

      if (queue[i].head != NULL) {
        flag = 1;

        // Executa round robin
        rr(queue[i], &gnatt[i], &clock, &sum, &pb, 0);
        // Verifica se um IO precisa ser executado
        if (ioBurstTime(queue, gnatt, &gnatt[x], i, x, &clock, &sum, &pb, &pdone)) {
          break;
        }

        // Calcula process burst
        if (pb <= 0) {
          // Move o último processo em execução para o final da fila
          if (queue[i].head != NULL) {
            enqueue(&queue[i], dequeue(&queue[i]));
          }

          // Move tudo para a fila de prontos
          for (int j = i; j < x; j++) {
            while (queue[j].head != queue[j].tail) {
              enqueue(&Ready, dequeue(&queue[j]));
            }
            if (queue[j].head != NULL) {
              enqueue(&Ready, dequeue(&queue[j]));
            }
          }

          // Atualiza o valor do tempo restante antes do próximo aumento
          pb += pboost;

          // Há novos processos na fila Pronto
          newpro = 1;
        }

        else {
          // Move o processo em execução para a próxima fila
          int QueuePosx = (i == x - 1) ? i : i + 1;
          if (queue[i].head->ExecTime > 0)
            enqueue(&queue[QueuePosx], dequeue(&queue[i]));
          else
            dequeue(&queue[i]);
        }

        break;
      } else {
        // Incrementa o índice da fila se não houver nenhum processo na fila atual
        i++;
      }

      // Se um novo processo chegou
      if (newpro == 1) {
        break;
      }

      // Se não houver processos restantes, para o relógio
      if (isEmpty(queue[i]) && rpro == 0 && isEmpty(Ready)) {
        run = 0;
      }
    }

    // Se nenhum processo for executado no tempo atual da CPU
    if (flag == 0) {
      pb--;
      sum++;
      clock++;
    }

    // Subtraia o tempo de execução do ciclo atual de chegada
    for (int i = 0; i < y; i++) {
      process[i].ArrivalTime -= sum;
    }
  }

  getAverageWaitTime(gnatt, x, y, &awt);

  printLabel("\n\nDiagrama de Gantt\n");
  printf("------------------------------------------------------------\n");
  for (int i = 0; i < x + 1; i++)
    printGnattChart(gnatt[i]);

  displayProcess(gnatt, gnatt[x], process, x, y, awt);
}

/**
 * Calcula o tempo medio de espera de processos
 * @param q[] lista de processos
 * @param x numero de filas
 * @param y quantidade de processos
 * @param awt tempo medio de espera
 */
void getAverageWaitTime (Queue q[], int x, int y, float* awt) {
  for (int i = 0; i < x; i++) {
    Process* curr = q[i].head;
    while (curr != NULL) {
      if (curr->ExecTime <= 0)
        *awt += curr->waiting;
      curr = curr->next;
    }
  }
  *awt /= y;
}