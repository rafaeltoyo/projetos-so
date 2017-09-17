#include "pingpong.h"

/* MainTask */
task_t mainTask;
task_t *currentTask;
unsigned long count_id = 0;

void pingpong_init () {
	setvbuf(stdout, 0, _IONBF, 0);
	/* Inicializa o encadeamento das task */
	mainTask.prev = 0;
	mainTask.next = 0;
	mainTask.tid = count_id;
	/* Apontar para main */
	mainTask.main = &mainTask;
	/* Realiza uma cópia do contexto atual pra ser o contexto da main */
	getcontext (&(mainTask.context));
	/* Salvar um ponteiro para a task atual em execucao */
	currentTask = &mainTask;
}

int task_create (task_t *task, void (*start_routine)(void*), void *arg) {
	char *stack;
	/* Inicializa o encadeamento das task */
	task->prev = 0;
	task->next = 0;
	task->tid = ++count_id;
	/* Apontar para main */
	task->main = &mainTask;
	/* Realiza uma cópia do contexto atual */
	getcontext (&(task->context));
	/* Cria uma stack para essa task */
	stack = malloc (STACKSIZE) ;
	if (stack)
	{
		/* Inicializa valores para o contexto */
		task->context.uc_stack.ss_sp = stack ;
		task->context.uc_stack.ss_size = STACKSIZE;
		task->context.uc_stack.ss_flags = 0;
		task->context.uc_link = 0;
	}
	else
	{
		perror ("Erro na criação da pilha: ");
		return(-1);
	}
	/* Cria o contexto com a rotina passada */
	makecontext (&(task->context), (void*)(*start_routine), 1, (void*)arg);
	#ifdef DEBUG
	printf ("task_create: criou tarefa %d\n", task->tid);
	#endif
	return task->tid;
}

int task_switch (task_t *task) {
	/* Salvar a task atual como antiga (sera trocada) */
	task_t *oldTask = currentTask;
	/* Muda o ponteiro para a nova "atual" task */
	/* Atualizar esse valor antes da realizacao do swapcontext */
	currentTask = task;
	/* Salva o estado atual do contexto em execucao antes da troca */
	/* Troca para o contexto da task passada como parametro */
	if ( swapcontext(&(oldTask->context),&(task->context)) < 0 ) {
		return -1;
	}
	#ifdef DEBUG
	printf ("task_switch: trocando contexto %d -> %d\n", oldTask->tid, task->tid);
	#endif
	return 0;
}

void task_exit (int exit_code) {
	/* Retorna para task principal (inicial no caso) */
	#ifdef DEBUG
	printf ("task_exit: tarefa %d sendo encerrada\n", currentTask->tid);
	#endif
	task_switch(&mainTask);
}

int task_id () {
	/* Retorna o id da task apontada pelo ponteiro currentTask */
	return currentTask->tid;
}
