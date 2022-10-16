#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<limits.h>

struct ProcessData {
  int Ppid;
	int Pid;
  int ArrivalTime;
  int BurstTime;
  int WaitTime;
  int TurnArountTime;
  int RemainTime;
  int CompleteTime;
} Q1[10], Q2[10], PQ[10]; // Fila de IO (PQ) e fila para os demais processos

void arrivalSort (int pnum) {
  struct ProcessData temp;
  for (int i=0; i<pnum; i++) {
    for (int j=i+1; j<pnum; j++) {
      if (Q1[i].ArrivalTime > Q1[j].ArrivalTime) {
        temp = Q1[i];
        Q1[i] = Q1[j];
        Q1[j] = temp;
      }
    }
  }
}

void readInput (int pnum) {
  for(int i=0; i<pnum; i++) {
    Q1[i].Ppid = i + 1;
    printf("Digite: <Pid> <ArrivalTime> <BurstTime> para o processo #%d: ", i + 1);
    scanf("%d%d%d", &Q1[i].Pid, &Q1[i].ArrivalTime, &Q1[i].BurstTime);
    Q1[i].RemainTime = Q1[i].BurstTime;
  }
  printf("\n\n");
}

void printProcessData (int pnum) {
  puts("Processos criados com sucesso:");
  for(int i=0; i<pnum; i++){
    printf("Processo #%d : Pid: %d | ArrivalTime: %d | BurstTime: %d\n",
      Q1[i].Ppid, Q1[i].Pid, Q1[i].ArrivalTime, Q1[i].BurstTime
    );
  }
  printf("\n\n");
}


int main (void) {
  int numProcesses;
  int i, j;
  int Clock = 0, Q_process = 0, PQ_process = 0;
  int quantum1 = 5, quantum2 = 8;
  int flag = 0; // controla quando um processo vai para Q2

  // Leitura e avaliacao dos parametros de entrada
  puts("Digite: <numero de processos>");
  scanf("%d", &numProcesses);
  printf("Numero de processos inserido: %d\n\n", numProcesses);

  // Preenche dados dos processos
  readInput(numProcesses);
  printProcessData(numProcesses);

  // Ordena baseado no arrival time
  arrivalSort(numProcesses);
  Clock = Q1[0].ArrivalTime;

  printf("Processos na fila Q1 com quantum = 5\n");
  for (i=0; i<numProcesses; i++) {
    if (Q1[i].RemainTime <= quantum1) {
      Clock += Q1[i].RemainTime;
      Q1[i].RemainTime = 0;
      Q1[i].WaitTime = Clock - Q1[i].ArrivalTime - Q1[i].BurstTime;
      Q1[i].TurnArountTime = Clock - Q1[i].ArrivalTime;
      printf("Processo: %d - Pid: %d - RemainTime: %d - WaitTime: %d - TurnArountTime: %d\n",
        Q1[i].Ppid, Q1[i].Pid, Q1[i].BurstTime, Q1[i].WaitTime, Q1[i].TurnArountTime);
    } else {
      Q2[Q_process].WaitTime = Clock;
      Clock += quantum1;
      Q1[i].RemainTime -= quantum1;
      Q2[Q_process].BurstTime = Q1[i].RemainTime;
      Q2[Q_process].RemainTime = Q2[Q_process].BurstTime;
      Q2[Q_process].Ppid = Q1[i].Ppid;
      Q_process = Q_process + 1;
      flag = 1;
    }
  }

  if (flag == 1) {
    printf("\nProcessos na fila Q1 com quantum = 8\n");
  }

  for (i=0; i<Q_process; i++) {
    if (Q2[i].RemainTime <= quantum2) {
      Clock += Q2[i].RemainTime;
      Q2[i].RemainTime = 0;
      Q2[i].WaitTime = Clock - quantum1 - Q2[i].BurstTime;
      Q2[i].TurnArountTime = Clock - Q2[i].ArrivalTime;
      printf("Processo: %d - Pid: %d - RemainTime: %d - WaitTime: %d - TurnArountTime: %d\n",
        Q2[i].Ppid, Q2[i].Pid, Q2[i].BurstTime, Q2[i].WaitTime, Q2[i].TurnArountTime);
    }
    else {
      PQ[PQ_process].ArrivalTime = Clock;
      Clock += quantum2;
      Q2[i].RemainTime -= quantum2;
      PQ[PQ_process].BurstTime = Q2[i].RemainTime;
      PQ[PQ_process].RemainTime = PQ[PQ_process].BurstTime;
      PQ[PQ_process].Ppid = Q2[i].Ppid;
      PQ_process = PQ_process + 1;
      flag = 2;
    }
  }

  if (flag == 2) {
    printf("\nProcessos na fila PQ para IO\n");
  }

  for (i=0; i<PQ_process; i++) {
    if (i == 0) {
      PQ[i].CompleteTime = PQ[i].BurstTime + Clock - quantum1 - quantum2;
    }
    else {
      PQ[i].CompleteTime = PQ[i - 1].CompleteTime + PQ[i].BurstTime;
    }
  }

  for (i=0; i<PQ_process; i++) {
    PQ[i].TurnArountTime = PQ[i].CompleteTime;
    PQ[i].WaitTime = PQ[i].TurnArountTime - PQ[i].BurstTime;
    printf("Processo: %d - Pid: %d - RemainTime: %d - WaitTime: %d - TurnArountTime: %d\n",
        PQ[i].Ppid, PQ[i].Pid, PQ[i].BurstTime, PQ[i].WaitTime, PQ[i].TurnArountTime);
  }

  return 0;
}