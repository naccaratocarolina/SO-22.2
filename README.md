# Sistemas Operacionais 22.2 - Escalonador de Processos
Desenvolver um simulador que implementa o algoritmo de escalonamento de processos, usando a estratégia de seleção Round Robin (ou Circular) com Feedback.

## Alunos:
- Carolina Naccarato
- Cristian Diamantaras
- Daniel de Sousa

## Regras:
1. Se Prioridade(A) > Prioridade(B), A é executado (B não).

2. Se Prioridade(A) = Prioridade(B), A e B são executados em RR.

3. Quando um job entra no sistema, ele é colocado na prioridade mais alta (a fila mais alta).

4. Uma vez que um job usa seu quantum de tempo em um determinado nível (independentemente de quantas vezes ele desistiu da CPU, por exemplo, durante uma rajada de E/S), sua prioridade é reduzida (ou seja, ele desce uma fila).

5. Após algum período de tempo S, mova todos os trabalhos do sistema para a fila mais alta.

# Input:

- A primeira linha contém 3 inteiros separados por espaço, X Y S.
  - X denota o algoritmo de agendamento da CPU.
  - Y denota o número de processos onde 3 ≤ Y ≤ 100
  - S denota o período de tempo para o aumento de prioridade.

- Haverá X linhas de inteiros separados por espaço A B C.
  - A é o ID da fila,
  - B é a prioridade, e
  - C é o quantum e tempo para esta fila.

  - Haverá Y linhas de inteiros separados por espaço F G H I J.
  - F é o ID do processo,
  - G é a hora de chegada, e
  - H é o tempo total de execução.
  - I é a duração da rajada de IO.
  - J é a frequência da rajada de IO.