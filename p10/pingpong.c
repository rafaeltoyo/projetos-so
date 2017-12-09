#include "pingpong.h"

void dispatcher_body();
task_t* scheduler();
int signal_init();

/* MainTask */
task_t mainTask;
task_t dispatcherTask;
task_t *currentTask;
task_t *readyQueue;
task_t *sleepQueue;
unsigned long count_id = 0;
unsigned long numTaskOn = 0;
short isExitCurrentTask;
short isLockedYield;
unsigned long systimeCount = 0;


/* 
 * imprime na tela um elemento da fila (chamada pela função queue_print) 
 * Funcao copiada e adaptada do testafila.c (projeto 00)
 * Usada apenas para debugar a fila de tarefas prontas
 */
void print_elem(void *ptr)
{
   task_t *elem = ptr ;

   if (!elem) {
      return ;
   }

   elem->prev ? printf ("%d", elem->prev->tid) : printf ("*") ;
   printf ("<%d>", elem->tid) ;
   elem->next ? printf ("%d", elem->next->tid) : printf ("*") ;
}


void pingpong_init()
{
	setvbuf(stdout, 0, _IONBF, 0);

	#ifdef DEBUG
	printf("Criando a main\n");
	#endif
	/* Definir como task de usuário */
	mainTask.type = USER_TASK;
	mainTask.quantum = 0;
	mainTask.start_time = 0;
	mainTask.total_time = 0;
	mainTask.activation = 0;
	mainTask.sleep_time = 0;
	/* Inicializa o encadeamento das task */
	mainTask.prev = 0;
	mainTask.next = 0;
	/* Seta o estado */
	mainTask.state = RUNNING;
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
	/* Colocar a MAIN da queue */
	queue_append((queue_t**)&readyQueue, (queue_t*)&mainTask);
	mainTask.queue = &readyQueue;
	mainTask.join_queue = 0;
	/* PS: O CONTADOR DE TASK DEVERIA SER INCREMENTADO, MAS A CRIAÇÃO DO DISPATCHER JÁ INCREMENTARÁ PELA MAIN */
	#ifdef DEBUG
	printf("Criando o dispatcher\n");
	#endif
	/* Inicializar o dispatcher */
	task_create(&dispatcherTask, (void*)(*dispatcher_body), (void*)0);
	dispatcherTask.type = SYSTEM_TASK;
	/* Retirar o dispatcher da fila de prontos */
	queue_remove((queue_t**)dispatcherTask.queue, (queue_t*)&dispatcherTask);
	dispatcherTask.queue = 0;
	isExitCurrentTask = 0;
	isLockedYield = 0;

	task_switch(&dispatcherTask);
}

int task_create(task_t *task, void (*start_routine)(void*), void *arg)
{
	char *stack;
	/* Definir como task de usuário */
	task->type = USER_TASK;
	task->quantum = 0;
	task->start_time = systime();
	task->total_time = 0;
	task->activation = 0;
	task->sleep_time = 0;
	/* Inicializa o encadeamento das task */
	task->prev = 0;
	task->next = 0;
	/* Seta o estado */
	mainTask.state = RUNNING;
	/* Apontar para main */
	task->main = &mainTask;
	/* Atribui um id para a task */
	task->tid = ++count_id;
	numTaskOn++;
	/* Ajusta as prioridades */
	task->prio_static = 0;
	task->prio_dynamic = 0;
	/* Realiza uma cópia do contexto atual */
	getcontext(&(task->context));
	/* Cria uma stack para essa task */
	stack = malloc(STACKSIZE) ;
	if (stack) {
		/* Inicializa valores para o contexto */
		task->context.uc_stack.ss_sp = stack ;
		task->context.uc_stack.ss_size = STACKSIZE;
		task->context.uc_stack.ss_flags = 0;
		task->context.uc_link = 0;
	} else {
		perror("Erro na criação da pilha: ");
		return(-1);
	}
	/* Cria o contexto com a rotina passada */
	makecontext(&(task->context), (void*)(*start_routine), 1, (void*)arg);
	/* Adicionar a fila de tarefas prontas */
	queue_append((queue_t**)&readyQueue, (queue_t*)task);
	task->queue = &readyQueue;
	task->join_queue = 0;

	#ifdef DEBUG
	printf("task_create: criou tarefa %d\n", task->tid);
	#endif

	return task->tid;
}

int task_switch(task_t *task)
{
	/* Salvar a task atual como antiga (sera trocada) */
	task_t *oldTask = currentTask;

	/* Muda o ponteiro para a nova "atual" task */
	/* Atualizar esse valor antes da realizacao do swapcontext */
	currentTask = task;

	/* Salva o estado atual do contexto em execucao antes da troca */
	/* Troca para o contexto da task passada como parametro */
	#ifdef DEBUG
	printf("task_switch: trocando contexto %d -> %d\n", oldTask->tid, task->tid);
	#endif

	if (swapcontext(&(oldTask->context),&(task->context)) < 0) {
		return -1;
	}
	return 0;
}

void task_exit(int exit_code)
{
	currentTask->state = FINISHED;
	/* Tarefa sendo encerrada ... */
	/* Mostrar dados finais da tarefa */
	printf(
		"Task %d exit: execution time %d ms, processor time %d ms, %d activations\n",
		currentTask->tid,
		systime() - currentTask->start_time,
		currentTask->total_time,
		currentTask->activation
	);

	#ifdef DEBUG
	printf("task_exit: tarefa %d sendo encerrada\n", currentTask->tid);
	#endif

	/* Desbloquear as tarefas que deram join nessa */
	if (currentTask->join_queue) {
		while (currentTask->join_queue) {
			currentTask->join_queue->exit_code = exit_code;
			task_resume(currentTask->join_queue);
		}
	}

	/* Finalizar o código se for o dispatcher (última task e terminar) */
	if (currentTask == &dispatcherTask) {
		#ifdef DEBUG
		printf("task_exit: fim do dispatcher\n");
		#endif
		exit(0);
	} else {
		/* Senao temos que retornar para o dispatcher mesmo */
		#ifdef DEBUG
		printf("task_exit: voltando para o dispatcher\n");
		#endif
		/*
		 * Flag para indicar ao dispatcher que a tarefa foi encerrada
		 * para limpeza de variáveis dessa tarefa.
		 */
		isExitCurrentTask = 1;
		numTaskOn--;
		task_switch(&dispatcherTask);
	}
}

int task_id()
{
	/* Retorna o id da task apontada pelo ponteiro currentTask */
	return currentTask->tid;
}

void task_suspend(task_t *task, task_t **queue)
{
	/* Ha uma task passada como parametro? */
	if (!task) {
		task = currentTask;
	}
	/* Ha uma queue passada como parametro? */
	if (!queue) {
		#ifdef DEBUG
		printf ("task_suspend: parametro fila esta nulo\n");
		#endif
		return;
	}

	#ifdef DEBUG
	printf ("task_suspend: tarefa %d sendo suspensa\n", task->tid);
	#endif
	/* Troca a task de fila (atual -> queue) */
	queue_remove((queue_t**)task->queue, (queue_t*)task);
	queue_append((queue_t**)queue, (queue_t*)task);
	task->queue = queue;
	task->state = SUSPENDED;
	isLockedYield = 0;
	task_yield();
}

void task_resume(task_t *task)
{
	if (!task || task->state == FINISHED) {
		#ifdef DEBUG
		printf("task_resume: tarefa %d ja encerrada!\n", task->tid);
		#endif
		return;
	}
	#ifdef DEBUG
	printf("task_resume: tarefa %d pronta!\n", task->tid);
	#endif
	/* Retirar a task da fila atual dela e adicionar na fila de prontos */
	queue_remove((queue_t**)task->queue, (queue_t*)task);
	queue_append((queue_t**)&readyQueue, (queue_t*)task);
	task->queue = &readyQueue;
	task->state = RUNNING;
}

void task_yield()
{
	/* Tarefa perdeu o processador */
	/* Devolve tarefa para fila de prontas */
	#ifdef DEBUG
	printf ("task_yield: tarefa %d perdendo processador\n", currentTask->tid);
	#endif
    if (currentTask->state == RUNNING) {
		queue_remove((queue_t**)currentTask->queue, (queue_t*)currentTask);
        queue_append((queue_t**)&readyQueue, (queue_t*)currentTask);
	    currentTask->queue = &readyQueue;
    }
	task_switch(&dispatcherTask);
}

void task_setprio(task_t *task, int prio)
{
	/* Verificar os limites de prioridade */
	if (prio > MAX_PRIORITY || prio < -MAX_PRIORITY) {
		#ifdef DEBUG
		printf("task_setprio: prioridade %d inválida (-MAX_PRIORITY a +MAX_PRIORITY apenas)\n", prio);
		#endif
		/* Nao atualizar prioridade se nao for valida */
		return;
	}
	/* Se nao houver task, vamos modificar a task atual */
	if (!task) {
		task = currentTask;
	}
	/* Atualizar prioridade estatica e resetar a prioridade dinamica */
	task->prio_static = prio;
	task->prio_dynamic = prio;
}

int task_getprio(task_t *task)
{
	/* Se nao houver task, retorna a prioridade da task atual */
	if (!task) {
		return currentTask->prio_static;
	}
	/* Se nao retorna a prioridade da task */
	return task->prio_static;
}

int task_join (task_t *task)
{
	#ifdef DEBUG
	printf ("task_join: tarefa %d dando join na tarefa %d\n", currentTask->tid, task->tid);
	#endif
	if (!task || !currentTask || task->state == FINISHED) {
		#ifdef DEBUG
		printf ("task_join: erro\n");
		#endif
		return -1;
	}
	/*
	 * Tarefa perdeu o processador
	 * Coloca na fila de dependência da task passada
	 * Volta para o dispatcher e aguarda essa tarefa receber o processador novamente
	 */
	task_suspend(currentTask, &(task->join_queue));
	/* Voltando, retorna o exit_code registrado */
	#ifdef DEBUG
	printf ("task_join: ... voltando (exit_code = %d)\n", currentTask->exit_code);
	#endif

	return currentTask->exit_code;
}

void task_sleep(int t) {
	#ifdef DEBUG
	printf ("task_sleep: tarefa %d adormecendo\n", currentTask->tid);
	#endif
	if (!currentTask || currentTask->state == FINISHED) {
		#ifdef DEBUG
		printf ("task_sleep: erro (task invalida)\n");
		#endif
		return;
	}
	if (t < 1) {
		#ifdef DEBUG
		printf ("task_sleep: erro (tempo deve ser maior que 0)\n");
		#endif
		return;
	}
	currentTask->state = SUSPENDED;
	/*
	 * Tarefa solicitando para adormecer
	 * Colocar na fila de tarefas adormecidas
	 * Voltar para o dispatcher e esperar o tempo para ser acordada
	 * t -> segundos acormecido (t x 1000 = tempo em milisegundos)
	 */
	currentTask->sleep_time = systime() + t * 1000;
	#ifdef DEBUG
	printf ("task_sleep: tempo adormecida (previsto): %ds\n", t);
	#endif
	task_suspend(currentTask, &sleepQueue);
}

/* *** *** *** FUNCAO DO DISPATCHER *** *** *** */
void dispatcher_body()
{
	task_t *nextTask;
	task_t *nextSleepyTask;
	unsigned int sleepTaskIterator;

	/* Inicializar o timer para registrar os 'tiks' */
	if (signal_init()) {
		exit(1);
	}

	/* Enquanto houver tarefas da fila de prontas */
	while (numTaskOn) {
		/* Desbloquear as tarefas adormecidas */
		sleepTaskIterator = queue_size((queue_t*)sleepQueue);
		nextSleepyTask = sleepQueue;
		while (sleepQueue && sleepTaskIterator) {
			if (nextSleepyTask->sleep_time <= systime()) {
				task_resume(nextSleepyTask);
			} else {
				nextSleepyTask = nextSleepyTask->next;
			}
			sleepTaskIterator--;
		}

		if (!readyQueue) {
			printf("dispatcher: fila de task prontas vazia ...\n");
		}

		/* Usar o scheduler para obter uma nova tarefa */
		nextTask = scheduler();
		if (nextTask) {
			dispatcherTask.activation++;

			#ifdef DEBUG
			queue_print("dispatcher: fila",(queue_t*)nextTask,(void*)*print_elem);
			#endif

			/* Remover essa tarefa escolhida da fila de prontos */
			queue_remove((queue_t**)&readyQueue, (queue_t*)nextTask);
			nextTask->queue = 0;

			#ifdef DEBUG
			printf("dispatcher: entrando task %d\n", nextTask->tid);
			#endif

			nextTask->quantum = QUANTUM_SIZE;
			nextTask->activation++;

			/* Trocar para essa task */
			if (task_switch(nextTask)) {
				exit(-1);
			}

			/* Ao retornar ao dispatcher, verificar se a tarefa nao terminou sua exec. */
			if (isExitCurrentTask) {
				#ifdef DEBUG
				printf("dispatcher: limpando a task %d\n", nextTask->tid);
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

task_t* scheduler()
{
	task_t *aux, *newTask;

	#ifdef DEBUG
	printf("scheduler: iniciando\n");
	#endif

	/* Fila vazia? */
	if (!readyQueue) {
		#ifdef DEBUG
		printf("scheduler: não há task pronta\n");
		#endif
		return NULL;
	}

	/* Fila com um único elemento */
	if (readyQueue == readyQueue->next) {
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
	/* Percorrer toda a fila de tarefas prontas */
	do {
		/* Achado uma tarefa com prioridade dinâmica menor OU igual mas com estática menor ... */
		if (newTask->prio_dynamic > aux->prio_dynamic ||
			(newTask->prio_dynamic == aux->prio_dynamic && 
			newTask->prio_static > aux->prio_static)
		) {
			/* ... vamos escolher ela */
			newTask = aux;
		}
		aux = aux->next;
	} while (aux != readyQueue);

	/* variável usada para percorrer a fila de tarefas prontas */
	aux = readyQueue;
	/* Envelhecer todos elementos (o escolhido terá seu envelhecimento resetado) */
	/* Percorrer toda a fila de tarefas prontas */
	do {
		if (aux != newTask && aux->prio_dynamic > -MAX_PRIORITY)
			aux->prio_dynamic--;
		aux = aux->next;
	} while (aux != readyQueue);

	#ifdef DEBUG
	printf("scheduler: escolhido: task %d com prioridade %d\n",
		newTask->tid,newTask->prio_dynamic);
	#endif
	
	newTask->prio_dynamic = newTask->prio_static;
	return newTask;
}

/* tratador do sinal */
void signal_behaviour(int signum)
{
	// Contar um tiks do timer (1ms)
	systimeCount++;
	currentTask->total_time++;
	/* Se a task atual for uma task de usuario, devemos decrementar a 'vida' dela */
	if (currentTask->type == USER_TASK) {
		/* Se chegar em 0, a task deve perder o processador */
		#ifdef DEBUG
		printf("signal_behaviour: Tiks na task %d\n", currentTask->tid);
		#endif
		if (--currentTask->quantum <= 0) {
			currentTask->quantum = 0;
			if (isLockedYield) {
				#ifdef DEBUG
				printf("signal_behaviour: preempcao bloqueada, aguardando ...\n");
				#endif
				return;
			}
			#ifdef DEBUG
			printf("signal_behaviour: fim da execucao\n");
			#endif
			task_yield();
		}
	}
}

int signal_init()
{
	/* registra a acao para o sinal de timer SIGALRM */
	action.sa_handler = signal_behaviour;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	if (sigaction(SIGALRM, &action, 0) < 0) {
		perror("Erro em sigaction: ");
		exit(1);
	}
	#ifdef DEBUG
	printf("signal_init: inicializado o signal action para responder ao timer\n");
	#endif

	/* ajusta valores do temporizador */
		/* primeiro disparo, em micro-segundos */
	timer.it_value.tv_usec = 1000;
		/* primeiro disparo, em segundos */
	timer.it_value.tv_sec  = 0;
		/* disparos subsequentes, em micro-segundos */
	timer.it_interval.tv_usec = 1000;
		/* disparos subsequentes, em segundos */
	timer.it_interval.tv_sec  = 0;

	/* arma o temporizador ITIMER_REAL */
	if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
		perror("Erro em setitimer: ");
		exit(1);
	}
	#ifdef DEBUG
	printf("signal_init: inicializado o timer com 1ms entre os tiks\n");
	#endif

	return 0;
}

unsigned int systime ()
{
	return systimeCount;
}

int sem_create (semaphore_t *s, int value)
{
	#ifdef DEBUG
	printf("sem_create: criando semaforo\n");
	#endif
	if (s->status == SEM_ON) {
		#ifdef DEBUG
		printf("sem_create: ERRO, semaforo ja inicializado\n");
		#endif
		return -1;
	}
	s->size = value;
	s->status = SEM_ON;
	return 0;
}

int sem_down (semaphore_t *s)
{
	#ifdef DEBUG
	printf("sem_down: task %d solicitando semaforo\n", currentTask->tid);
	#endif
    /* Checar se o semaforo existe e esta ativo */
    if (!s || s->status == SEM_OFF) {
        
        #ifdef DEBUG
	    printf("sem_down: ERRO, semoforo nao existe\n");
	    #endif
	    /* Erro: Semaforo ja encerrado */
	    return -1;
    }
    /* Bloquear preempcao */
    isLockedYield = 1;
    /* Decrementa o contador */
    if (--s->size < 0) {
        #ifdef DEBUG
		printf("sem_down: semoforo cheio, aguarde ...\n");
		#endif
        /* Aguardar espaco ... (isLockedYield = 0 antes do switch) */
		task_suspend(currentTask, &(s->queue));        
        /* ... o semaforo ainda existe apos a espera? */
        if (s->status == SEM_OFF) {
			#ifdef DEBUG
			printf("sem_down: ERRO, semoforo ja encerrado\n");
			#endif
			return -1;
		}
		isLockedYield = 0;
		return 0;
    }
    isLockedYield = 0;
	return 0;
}

int sem_up (semaphore_t *s)
{
	#ifdef DEBUG
	printf("sem_up: task %d liberando o semaforo\n", currentTask->tid);
	#endif
    /* Checar se o semaforo existe e esta ativo */
    if (!s || s->status == SEM_OFF) {
    	#ifdef DEBUG
	    printf("sem_up: ERRO, semoforo nao existe\n");
	    #endif
	    /* Erro: Semaforo ja encerrado */
	    return -1;
    }
    /* Bloquear preempcao */
    isLockedYield = 1;
	/* Aumentar contador e verifica se podemos acordar alguma task (abriu espaço) */
	if (++s->size <= 0 && s->queue) {
		#ifdef DEBUG
	    printf("sem_up: acordando task %d\n", s->queue->tid);
	    #endif
		task_resume(s->queue);
	}
	/* Liberar preempcao */
	isLockedYield = 0;
	return 0;
}

int sem_destroy (semaphore_t *s)
{
    #ifdef DEBUG
	printf("sem_destroy: Solicitacao de destruicao do semaforo\n");
	#endif
    /* Checar se o semaforo existe e esta ativo */
	if (!s || s->status == SEM_OFF) {
        #ifdef DEBUG
	    printf("sem_destroy: ERRO, semaforo nao existe\n");
	    #endif
		/* Erro: Semaforo ja encerrado */
		return -1;
	}
	/* Encerrar semaforo */
	s->status = SEM_OFF;
	/* Liberar elementos suspencos */
	while (s->queue) {
		task_resume(s->queue);
		printf("sdadasdasd\n");
	}
	return 0;
}
