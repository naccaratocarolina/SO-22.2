#include "process.h"
#include "queue.h"
#include <math.h>
#include <stdio.h>

#ifndef DISPLAY
#define DISPLAY

void displayProcess (Queue q[], Queue io, Process p[], int x, int y, float awt);
void printLabel (char* str);
void printGnattChart (Queue gnatt);
int getNumDigits (int num);

void displayProcess (Queue q[], Queue io, Process p[], int x, int y, float awt) {
  pidSort(p, y);
  printLabel("\n\nLista de Processos\n");
  for (int i = 0; i < y; i++) {

    Queue temp = initQueue(-1);
    int count = 0;
    printf("------------------------------------------------------------\n");
    printf("Processo %d    Fila     Inicio    Fim    Espera    Turnaround\n", p[i].Pid);

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
    }

    Process temparr[count];
    for (int k = 0; k < count; k++) {
      if (temp.head != NULL) temparr[k] = *dequeue(&temp);
    }

    sortbyStartTime(temparr, count);
    for (int k = 0; k < count; k++) {
      enqueue(&temp, &temparr[k]);
    }

    Process* curr = temp.head;
    float waiting = 0;
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
        printf("%d%*c", curr->waiting, 10 - getNumDigits(curr->waiting), ' ');
        printf("%d\n", curr->TurnArountTime);
      }
      waiting = curr->waiting;
      TurnArountTime = curr->TurnArountTime;
      curr = curr->next;
    }
    printf("\nTempo de espera: %.1f", waiting);
    printf("\nTurnaround: %.1f\n", TurnArountTime);
  }

  printf("------------------------------------------------------------");
  printf("\nTempo medio de espera: %.1f\n\n", awt);
}

/**
 * @param label mensagem a ser impressa
 */
void printLabel (char* str) {
  printf("%s", str);
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
 * @param gnatt fila de processos de io
 */
void printGnattChart (Queue gnatt) {
  if (gnatt.QueuePos != -1) {
    printf("\nQueue %d\n", gnatt.QueuePos);
  } else {
    printf("\nIO\n");
  }
  Process* curr = gnatt.head;
  Process* last = gnatt.head;
  while (curr != NULL) {
    if (last == curr) {
      if (curr->StartTime != 0) printf("[//] ");
    } 
    printf(" %d", curr->StartTime);
    printf("%*c", (curr->CompleteTime - curr->StartTime) / 2, ' ');
    printf(" P%d ", curr->Pid);
    printf("%*c", (curr->CompleteTime - curr->StartTime) / 2, ' ');
    printf("%d ", curr->CompleteTime);
    printf(" ");
    printf("[//] ");
    last = curr;
    curr = curr->next;
  }
  printf("\n");
}

#endif