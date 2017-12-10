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
#include <strings.h>
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

	short barrier_error;

	enum state_t {RUNNING, SUSPENDED, FINISHED} state;
	enum type_t {USER_TASK, SYSTEM_TASK} type;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
	int sid;

	int size;
	struct task_t *queue;

	enum sstatus_s {SEM_OFF, SEM_ON} status;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
	int bid;

	int size;
	int max_size;
	struct task_t *queue;

	enum bstatus_s {BARRIER_OFF, BARRIER_ON} status;
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
	int mid;

	unsigned int msg_size;
	unsigned int msg_max;
	void* msg_buffer;
	
	unsigned long s_iter;
	unsigned long r_iter;

	semaphore_t sem_buffer;
	semaphore_t sem_prod;
	semaphore_t sem_cons;

	enum mstatus_s {MQUEUE_OFF, MQUEUE_ON} status;
} mqueue_t ;

#endif
