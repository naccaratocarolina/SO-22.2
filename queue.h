#include "process.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef QUEUE
#define QUEUE

// Inicializacao da estrutura de uma queue
typedef struct queue {
  int QueuePos;
  int priority;
  int quantum;
  Process* head;
  Process* tail;
} Queue;

Queue initQueue (int QueuePos);
int isEmpty (Queue q);
void enqueue (Queue* q, Process* p);
Process* dequeue (Queue* q);
void sortqbypriority (Queue q[], int x);
void printqueue (Queue q);

/**
 * Inicializa atributos da queue
 * @param QueuePos posicao na Queue
 */ 
Queue initQueue (int QueuePos) {
  Queue q;
  q.QueuePos = QueuePos;
  q.priority = 0;
  q.quantum = 0;
  q.head = NULL;
  q.tail = NULL;
  return q;
}

/**
 * Verifica se a queue esta vazia olhando para
 * a sua cabeca. Caso esteja vazia, retorna 1;
 * C.c., 0
 * @param q instancia de uma queue
 */
int isEmpty (Queue q) {
  if (q.head == NULL) return 1;
  return 0;
}

/**
 * Emfila um novo elemento no final da queue 
 * de acordo com a sua cabeca e cauda
 * @param q instancia de uma queue
 * @param p instancia de um processo
 */
void enqueue (Queue* q, Process* p) {
  p->next = NULL;
  if (q->tail != NULL) q->tail->next = p;
  if (q->head == NULL) q->head = p;
  q->tail = p;
}

/**
 * Desenfila o primeiro elemento da queue
 * de acordo com sua cabeca e cauda,
 * retornando o elemento removido
 * @param q instancia de uma queue
 */
Process* dequeue (Queue* q) {
  Process* temp = q->head;
  q->head = q->head->next;
  if (q->head == NULL) q->tail = NULL;
  temp->next = NULL;
  return temp;
}

/**
 * Ordena a queue de acordo com a prioridade
 * @param q[] lista de instancias de uma queue
 * @param priority prioridade que usaremos como base no sort
 */
void prioritySort (Queue q[], int x) {
  Queue temp;
  for (int i = 0; i < x; i++)
    for (int j = i + 1; j < x; j++)
      if (q[i].priority > q[j].priority) {
        temp = q[i];
        q[i] = q[j];
        q[j] = temp;
      }
}

/**
 * Imprime os elementos da queue
 * @param q instancia de uma queue
 */
void printqueue (Queue q) {
  Process* curr = q.head;
  printf("Q[%d]\t", q.QueuePos);
  while (curr != NULL) {
    printf("P%d ->", curr->Pid);
    curr = curr->next;
  }
}

#endif