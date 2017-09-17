#include "pingpong.h"

void dispatcher_body();
task_t* scheduler();

/* MainTask */
task_t mainTask;
task_t dispatcherTask;
task_t *currentTask;
task_t *readyQueue;
unsigned long count_id = 0;
short isExitCurrentTask;

void pingpong_init ()
{
	setvbuf(stdout, 0, _IONBF, 0);

	#ifdef DEBUG
	printf ("Criando a main\n");
	#endif
	/* Inicializa o encadeamento das task */
	mainTask.prev = 0;
	mainTask.next = 0;
	/* Seta o estado */
	mainTask.state = READY;
	/* Apontar para main */
	mainTask.main = &mainTask;
	/* Atribui o id = 0 para a task principal */
	mainTask.tid = count_id;
	/* Ajusta as prioridades */
	mainTask.prio_static = 0;
	mainTask.prio_dynamic = 0;
	/* Realiza uma cópia do contexto atual pra ser o contexto da main */
	getcontext (&(mainTask.context));
	/* Salvar um ponteiro para a task atual em execucao */
	currentTask = &mainTask;

	#ifdef DEBUG
	printf ("Criando o dispatcher\n");
	#endif
	/* Inicializar o dispatcher */
	task_create (&dispatcherTask, (void*)(*dispatcher_body), (void*)0);
	/* Retirar o dispatcher da fila de prontos */
	queue_remove ((queue_t**)dispatcherTask.queue, (queue_t*)&dispatcherTask);
	dispatcherTask.queue = 0;
	isExitCurrentTask = 0;
}

int task_create (task_t *task, void (*start_routine)(void*), void *arg)
{
	char *stack;
	/* Inicializa o encadeamento das task */
	task->prev = 0;
	task->next = 0;
	/* Seta o estado */
	task->state = READY;
	/* Apontar para main */
	task->main = &mainTask;
	/* Atribui um id para a task */
	task->tid = ++count_id;
	/* Ajusta as prioridades */
	task->prio_static = 0;
	task->prio_dynamic = 0;
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
	/* FCFS */
	queue_append((queue_t**)&readyQueue, (queue_t*)task);
	task->queue = &readyQueue;

	#ifdef DEBUG
	printf ("task_create: criou tarefa %d\n", task->tid);
	#endif

	return task->tid;
}

int task_switch (task_t *task)
{
	/* Salvar a task atual como antiga (sera trocada) */
	task_t *oldTask = currentTask;

	/* Muda o ponteiro para a nova "atual" task */
	/* Atualizar esse valor antes da realizacao do swapcontext */
	currentTask = task;

	#ifdef DEBUG
	printf ("task_switch: trocando contexto %d -> %d\n", oldTask->tid, task->tid);
	#endif
	/* Salva o estado atual do contexto em execucao antes da troca */
	/* Troca para o contexto da task passada como parametro */
	if ( swapcontext(&(oldTask->context),&(task->context)) < 0 )
		return -1;
	return 0;
}

void task_exit (int exit_code)
{
	/* Retorna para task principal (inicial no caso) */
	#ifdef DEBUG
	printf ("task_exit: tarefa %d sendo encerrada\n", currentTask->tid);
	#endif
	
	if ( currentTask == &dispatcherTask )
	{
		#ifdef DEBUG
		printf ("task_exit: voltando para a main\n");
		#endif
		isExitCurrentTask = 1;
		task_switch(&mainTask);
	}
	else
	{
		#ifdef DEBUG
		printf ("task_exit: voltando para o dispatcher\n");
		#endif
		isExitCurrentTask = 1;
		task_switch(&dispatcherTask);
	}
}

int task_id ()
{
	/* Retorna o id da task apontada pelo ponteiro currentTask */
	return currentTask->tid;
}

void task_suspend (task_t *task, task_t **queue)
{
	/* Ha uma task passada como parametro? */
	if (!task)
		task = currentTask;
	task->state = SUSPENDED;
	/* Ha uma queue passada como parametro? */
	if (!task->queue)
		return;
	/* Troca a task de fila (atual -> queue) */
	queue_remove((queue_t**)task->queue, (queue_t*)task);
	queue_append((queue_t**)queue, (queue_t*)task);
	task->queue = queue;
	#ifdef DEBUG
	printf ("task_suspend: tarefa %d sendo suspensa\n", task->tid);
	#endif
}

void task_resume (task_t *task)
{
	/* Retirar a task da fila atual dela */
	if (task->queue)
		queue_remove((queue_t**)task->queue, (queue_t*)task);

	/* Adicionar a fila de task prontas */
	task->state = READY;
	queue_append((queue_t**)&readyQueue, (queue_t*)task);
	task->queue = &readyQueue;
	#ifdef DEBUG
	printf ("task_resume: tarefa %d pronta!\n", task->tid);
	#endif
}

void task_yield ()
{
	if (currentTask != &mainTask)
	{
		queue_append((queue_t**)&readyQueue, (queue_t*)currentTask);
		currentTask->queue = &readyQueue;
	}
	task_switch(&dispatcherTask);
}

// imprime na tela um elemento da fila (chamada pela função queue_print)
void print_elem (void *ptr)
{
   task_t *elem = ptr ;

   if (!elem)
	  return ;

   elem->prev ? printf ("%d", elem->prev->tid) : printf ("*") ;
   printf ("<%d/%d>", elem->tid, elem->prio_dynamic) ;
   elem->next ? printf ("%d", elem->next->tid) : printf ("*") ;
}

void dispatcher_body ()
{
	task_t *nextTask;
	while ( queue_size((queue_t*)readyQueue) )
	{
		#ifdef DEBUG
		queue_print("dispatcher: fila",(queue_t*)nextTask,(void*)*print_elem);
		#endif
		nextTask = scheduler();
		if (nextTask)
		{
			#ifdef DEBUG
			queue_print("dispatcher: fila",(queue_t*)nextTask,(void*)*print_elem);
			#endif
			queue_remove((queue_t**)&readyQueue, (queue_t*)nextTask);
			nextTask->queue = 0;
			#ifdef DEBUG
			printf("dispatcher: entrando task %d\n", nextTask->tid);
			#endif
			if ( task_switch (nextTask) )
				exit(-1);
			if (isExitCurrentTask == 1)
			{
				#ifdef DEBUG
				printf ("task_switch: limpando a task %d\n", nextTask->tid);
				#endif
				free(nextTask->context.uc_stack.ss_sp);
				isExitCurrentTask = 0;
			}
		}
	}
	#ifdef DEBUG
	printf("dispatcher: finalizando\n");
	#endif
	task_exit(0);
}

task_t* scheduler ()
{
	task_t *aux, *newTask;

	#ifdef DEBUG
	printf("scheduler: iniciando\n");
	#endif

	/* Fila vazia? */
	if ( readyQueue == NULL )
	{
		#ifdef DEBUG
		printf("scheduler: não há task pronta\n");
		#endif
		return NULL;
	}

	/* Fila com um único elemento */
	if ( readyQueue == readyQueue->next )
	{
		#ifdef DEBUG
		printf("scheduler: escolhido: task %d com prioridade %d\n",
			readyQueue->tid,readyQueue->prio_dynamic);
		#endif
		readyQueue->prio_dynamic = readyQueue->prio_static;
		return readyQueue;
	}

	/* Fila com mais de um elemento */
	/* variável usada para percorrer a fila de tarefas prontas */
	aux = readyQueue->next;
	/* ponteiro para a tarefas escolhida */
	newTask = readyQueue;
	/* Envelhecer todos elementos (o escolhido terá seu envelhecimento resetado) */
	if (newTask->prio_dynamic > -20)
		newTask->prio_dynamic--;
	/* Percorrer toda a fila de tarefas prontas */
	do
	{
		if (aux->prio_dynamic > -20)
			aux->prio_dynamic--;
		/* Achado uma tarefa com prioridade dinâmica menor OU igual mas com estática menor ... */
		if (newTask->prio_dynamic > aux->prio_dynamic || (
				newTask->prio_dynamic == aux->prio_dynamic && 
				newTask->prio_static > aux->prio_static )
			)
		{
			/* ... vamos escolher ela */
			newTask = aux;
		}
		aux = aux->next;
	}
	while (aux != readyQueue);

	#ifdef DEBUG
	printf("scheduler: escolhido: task %d com prioridade %d\n",
		newTask->tid,newTask->prio_dynamic);
	#endif
	/* Reseta a prioridade dinamica */
	newTask->prio_dynamic = newTask->prio_static;
	return newTask;
}

void task_setprio (task_t *task, int prio)
{
	if (prio > 20 || prio < -20)
	{
		#ifdef DEBUG
		printf("task_setprio: prioridade %d inválida (-20 a +20 apenas)\n", prio);
		#endif
		return;
	}
	if (task == NULL)
	{
		task = currentTask;
	}
	task->prio_static = prio;
	task->prio_dynamic = prio;
}

int task_getprio (task_t *task)
{
	if (task == NULL)
		return currentTask->prio_static;
	return task->prio_static;
}
