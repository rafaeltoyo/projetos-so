// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#define STACKSIZE 32768		/* tamanho de pilha das threads */

// Estrutura que define uma tarefa
typedef struct task_t
{
	struct task_t *prev;
	struct task_t *next;
	struct task_t *main;
	ucontext_t context;
	int tid;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
