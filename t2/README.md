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
#define PAGE_SIZE 64
#define TLB_SIZE 32
#define FRAME_SIZE 64
#define WORKING_SET_LIMIT 4
```

## Premissas:
- Os frames na memória física são preenchidos do 0 ao 63. Quando a mesma estiver cheia, aplicamos o algoritmo de sibstituição LRU para descobrir qual frame será atualizado

- Implementa o algoritmo LRU para determinar a substituição de páginas e substituição da TLB

- A busca pela referência na TLB é realizada por meio de threads, de forma que temos uma thread para cada entrada na tabela com o objetivo de otimizar este processo

## Input
- O programa recebe como entrada um arquivo preenchido com endereços lógicos, e retorna a tradução de tais endereços
