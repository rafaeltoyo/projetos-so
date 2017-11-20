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
#include <signal.h>
#include <sys/time.h>
#include "queue.h"

#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define QUANTUM_SIZE 20
#define MAX_PRIORITY 20

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

// Estrutura que define uma tarefa
typedef struct task_t
{
	struct task_t *prev;
	struct task_t *next;
	struct task_t *main;
	struct task_t **queue;
	struct task_t *join_queue;
	ucontext_t context;
	int tid;
	int prio_static;
	int prio_dynamic;
	unsigned int quantum;
	unsigned int start_time;
	unsigned int total_time;
	unsigned int activation;
	unsigned int exit_code;
	unsigned int sleep_time;
	enum state_t {RUNNING, SUSPENDED, FINISHED} state;
	enum type_t {USER_TASK, SYSTEM_TASK} type;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
	unsigned int size;
	struct task_t **queue;
	enum status {SEM_ON, SEM_OFF};
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
