#include "process.h"
#include "queue.h"
#include <math.h>
#include <stdio.h>

#define DISPLAY

void displayProcess (Queue q[], Queue io, Process p[], int x, int n, float awt);
void printLabel (char* label);
int getNumDigits (int num);
void printGnattChart (Queue gnatt);

/**
 * Imprime os elementos de acordo com o tipo de fila que
 * o mesmo se encontra e calcula o tempo medio de espera
 * @param q[] lista de instancias de uma queue
 * @param io lista de IO
 * @param x posicao do processo que esta sendo executado
 * @param n quantidade de processos
 * @param awt Average Wait Time
 */
void displayProcess (
  Queue q[], Queue io, Process p[], int x, int n, float awt
) {
  pidSort(p, n);
  printLabel("\n\nLista de processos\n");

  for (int i = 0; i < n; i++) {
    Queue temp = initqueue(-1);
    int count = 0;

    printf("------------------------------------------------------------\n");
    printf("Process %d    Queue     StartTime    End    WaitTime    TurnArountTime\n", p[i].Pid);

    for (int j = 0; j < x + 1; j++) {
      Process* curr = q[j].head;

      while (curr != NULL) {
        if (curr->Pid == p[i].Pid) {
          if (j == x) curr->QueuePos = -1;
          enqueue(&temp, pcopy(curr));
          count++;
        }
        curr = curr->next;
      }

      Process temparr[count];
      for (int k = 0; k < count; k++) {
        if (temp.head != NULL) temparr[k] = *dequeue(&temp);
      }

      startTimeSort(temparr, count);
      for (int k = 0; k < count; k++) {
        enqueue(&temp, &temparr[k]);
      }
      
      // Calcula tempo medio de espera
      Process* curr = temp.head;
      float WaitTime = 0;
      float TurnArountTime = 0;
      while (curr != NULL) {
        if (curr->Pid == p[i].Pid) {
          if (curr->QueuePos != -1) {
            printf("%*c%d%*c", 13, ' ', curr->QueuePos, 8 - getNumDigits(curr->QueuePos), ' ');
          } else {
            printf("%*cIO%*c", 13, ' ', 7, ' ');
          }
          printf("%d%*c", curr->StartTime, 8 - getNumDigits(curr->StartTime), ' ');
          printf("%d%*c", curr->CompleteTime, 6 - getNumDigits(curr->CompleteTime), ' ');
          printf("%d%*c", curr->WaitTime, 10 - getNumDigits(curr->WaitTime), ' ');
          printf("%d\n", curr->TurnArountTime);
        }
        WaitTime = curr->WaitTime;
        TurnArountTime = curr->TurnArountTime;
        curr = curr->next;
      }
      printf("\nWaiting Time: %.1f", WaitTime);
      printf("\nTurnaround Time: %.1f\n", TurnArountTime);
    }

    printf("------------------------------------------------------------");
    printf("\nAverage Wait Time: %.1f\n\n", awt);
  }
}

/**
 * @param label mensagem a ser impressa
 */
void printLabel (char* label) {
  printf("%s", label);
}

/**
 * Encontra a quantidade de digitos que um numero possui
 * @param num numero que esta sendo analisado
 */
int getNumDigits (int num) {
  if (num < 0) num *= -10;
  return (num == 0) ? 0 : floor(log10(num));
}

/**
 * Cria o Diagrama de Gantt
 * @param gnatt fila de processos
 */
void printGnattChart (Queue gnatt) {
  if (gnatt.QueuePos != -1) {
    printf("\nQueue %d\n", gnatt.QueuePos);
  } else {
    printf("\nIO\n", gnatt.QueuePos);
  }
  Process* curr = gnatt.head;
  Process* last = gnatt.head;
  while (curr != NULL) {
    if (last == curr) {
      if (curr->StartTime != 0) printf("[///] ");
    } else if (last->CompleteTime < curr->StartTime) {
      printf("[///] ");
    }
    printf(" %d", curr->StartTime);
    printf("%*c", (curr->CompleteTime - curr->StartTime) / 2, ' ');
    printf(" P%d ", curr->Pid);
    printf("%*c", (curr->CompleteTime - curr->StartTime) / 2, ' ');
    printf("%d ", curr->CompleteTime);
    printf(" ");
    last = curr;
    curr = curr->next;
  }
  printf("\n");
}