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
	/* Adicionar a fila de tarefas prontas */
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

	/* Salva o estado atual do contexto em execucao antes da troca */
	/* Troca para o contexto da task passada como parametro */
	#ifdef DEBUG
	printf ("task_switch: trocando contexto %d -> %d\n", oldTask->tid, task->tid);
	#endif
	if ( swapcontext(&(oldTask->context),&(task->context)) < 0 )
		return -1;
	return 0;
}

void task_exit (int exit_code)
{
	(void)exit_code;
	/* Tarefa sendo encerrada ... */
	#ifdef DEBUG
	printf ("task_exit: tarefa %d sendo encerrada\n", currentTask->tid);
	#endif

	/*
	 * Flag para indicar ao dispatcher que a tarefa foi encerrada
	 * para limpesa de variáveis dessa tarefa.
	 */
	isExitCurrentTask = 1;

	/* Retorna para task principal (inicial no caso) caso o dispatcher terminar */
	if ( currentTask == &dispatcherTask )
	{
		#ifdef DEBUG
		printf ("task_exit: voltando para a main\n");
		#endif
		
		task_switch(&mainTask);
	}
	/* Senao temos que retornar para o dispatcher mesmo */
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
	/* Se a tarefa que chamou esse metodo nao eh a main ... */
	if (currentTask != &mainTask) {
		/* ... devemos retornar ela para a fila de tarefas prontas ... */
		queue_append((queue_t**)&readyQueue, (queue_t*)currentTask);
		currentTask->queue = &readyQueue;
	}
	/* ... e entao chamar o dispatcher. */
	task_switch(&dispatcherTask);
}

#ifdef DEBUG
/* 
 * imprime na tela um elemento da fila (chamada pela função queue_print) 
 * Funcao copiada e adaptada do testafila.c (projeto 00)
 * Usada apenas para debugar a fila de tarefas prontas
 */
void print_elem (void *ptr)
{
   task_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->tid) : printf ("*") ;
   printf ("<%d>", elem->tid) ;
   elem->next ? printf ("%d", elem->next->tid) : printf ("*") ;
}
#endif

/* *** *** *** FUNCAO DO DISPATCHER *** *** *** */
void dispatcher_body ()
{
	task_t *nextTask;
	/* Enquanto houver tarefas da fila de prontas */
	while ( queue_size((queue_t*)readyQueue) )
	{
		/* Usar o scheduler para obter uma nova tarefa */
		nextTask = scheduler();
		if (nextTask)
		{
			#ifdef DEBUG
			queue_print("dispatcher: fila",(queue_t*)nextTask,(void*)*print_elem);
			#endif

			/* Remover essa tarefa escolhida da fila de prontos */
			queue_remove((queue_t**)&readyQueue, (queue_t*)nextTask);
			nextTask->queue = 0;

			#ifdef DEBUG
			printf("dispatcher: entrando task %d\n", nextTask->tid);
			#endif

			/* Trocar para essa task */
			if ( task_switch (nextTask) ) {
				exit(-1);
			}

			/* Ao retornar ao dispatcher, verificar se a tarefa nao terminou sua exec. */
			if (isExitCurrentTask == 1) {
				#ifdef DEBUG
				printf ("dispatcher: limpando a task %d\n", nextTask->tid);
				#endif
				/* Limpar as alocacoes dinamicas */
				free(nextTask->context.uc_stack.ss_sp);
				isExitCurrentTask = 0;
			}
		}
	}
	#ifdef DEBUG
	printf("dispatcher: finalizando \n");
	#endif

	/* Encerrar o dispatcher */
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
	/* Verificar os limites de prioridade */
	if (prio > 20 || prio < -20)
	{
		#ifdef DEBUG
		printf("task_setprio: prioridade %d inválida (-20 a +20 apenas)\n", prio);
		#endif
		/* Nao atualizar prioridade se nao for valida */
		return;
	}
	/* Se nao houver task, vamos modificar a task atual */
	if (task == NULL)
	{
		task = currentTask;
	}
	/* Atualizar prioridade estatica e resetar a prioridade dinamica */
	task->prio_static = prio;
	task->prio_dynamic = prio;
}

int task_getprio (task_t *task)
{
	/* Se nao houver task, retorna a prioridade da task atual */
	if (task == NULL)
		return currentTask->prio_static;
	/* Se nao retorna a prioridade da task */
	return task->prio_static;
}
