# Sistemas Operacionais 22.2 - Gerenciamento de Memória Virtual

Desenvolver um simulador que implementa um gerenciador de memória virtual

## Alunos:

- Carolina Naccarato
- Cristian Diamantaras
- Daniel de Sousa

## Setup

- Compile o arquivo principal

```bash
gcc main.c -o saida -pthread
```

- Rode o executável passando o arquivo como parâmetro

```bash
./saida arquivo.txt
```

## Parâmetros utilizados:

```C
#define FRAME_SIZE 4096
#define TLB_SIZE 4
#define RAM_SIZE 64
#define PAGE_TABLE_SIZE 128
#define PROCESS 20
#define WORK_SET_LIMIT 4
```

## Premissas:

- Os frames na memória física são preenchidos do 0 ao 63. Quando ocorre um page fault, aplicamos o algoritmo de substituição LRU para escolher uma página para remover da memória a fim de abrir espaço para a que está chegando.

- Implementa o algoritmo LRU para determinar a substituição de páginas e substituição da TLB

- A busca pela referência na TLB é realizada simultaneamente por meio de threads, de forma que temos uma thread para cada entrada na tabela com o objetivo de otimizar este processo
