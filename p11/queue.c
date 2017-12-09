#include <stdio.h>
#include "queue.h"

void queue_append (queue_t **queue, queue_t *elem) 
{

	/* Verificar se a fila existe */
	if (queue == NULL)
	{
		/* Fila nao existe */
		#ifdef DEBUG
		printf ("Fila não existe.\n");
		#endif
		return;
	}

	/* Verificar se o elemento existe */
	if (elem == NULL)
	{
		#ifdef DEBUG
		printf ("Elemento não existe.\n");
		#endif
		return;
	}

	/* Verificar se o elemento nao esta em outra fila */
	if (elem->prev != NULL || elem->next != NULL)
	{
		#ifdef DEBUG
		printf ("Elemento já está em uma fila.\n");
		#endif
		return;
	}

	/* Inserir o elemento */

	/* Ha um primeiro elemento? */
	if (*queue == NULL)
	{
		/* Nao ha nada */
		#ifdef DEBUG
		printf ("Fila vazia e sendo preenchida com um primeiro valor.\n");
		#endif
		*queue = elem;
		elem->prev = elem;
		elem->next = elem;
		return;
	}
	else
	{
		/* Ja ha um elemento, logo devemos inserir no final da fila (antes do primeiro) */
		/* O elemento novo tem como proximo o inicio da fila */
		elem->next = (*queue);
		/* E antes, o antigo ultimo elemento da lista */
		elem->prev = (*queue)->prev;
		/* Agora, o antigo ultimo elemento vai ter esse novo como proximo */
		(*queue)->prev->next = elem;
		/* E o primeiro elemento tem esse novo elemento como anterior */
		(*queue)->prev = elem;
		#ifdef DEBUG
		printf ("Novo elemento inserido.\n");
		#endif
		return; /* Sucesso! */
	}
}

queue_t *queue_remove (queue_t **queue, queue_t *elem)
{
	queue_t *aux;

	/* Verificar se a fila existe */
	if (queue == NULL)
	{
		/* Fila nao existe */
		#ifdef DEBUG
		printf ("Fila nao existe.\n");
		#endif
		return NULL;
	}

	/* Verificar se o elemento existe */
	if (elem == NULL)
	{
		#ifdef DEBUG
		printf ("Elemento nao existe.\n");
		#endif
		return NULL;
	}

	/* Verificar se a fila nao esta vazia */
	if (*queue == NULL)
	{
		#ifdef DEBUG
		printf ("A fila ja esta vazia.\n");
		#endif
		return NULL;
	}

	/* Comecar no primeiro elemento da fila */
	aux = (*queue);

	/* Enquanto nao achar o elemento a ser removido, vamos procurar na fila */
	while (aux != elem)
	{
		/* Checar se chequei no final da fila */
		if (aux->next == *queue)
		{
			/* Achei o primeiro elemento novamente, logo acabou a fila */
			/* Por estar no laco, nao achamos o elemento, */
			/* pois achar ele significa quebrar esse laco. */
			#ifdef DEBUG
			printf ("Elemento nao esta nessa fila.\n");
			#endif
			return NULL;
		}
		else
		{
			/* Ainda ha elementos na fila, entao continua */
			aux = aux->next;
		}
	}

	/* Excluir o elemento encontrado */

	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;

	#ifdef DEBUG
	printf ("Um elemento foi removido da fila.\n");
	#endif

	/* Era o primeiro? */
	if (elem == *queue)
	{
		/* Era o ultimo? */
		if (elem == elem->next)
		{
			/* Fila esta vazia agora. */
			*queue = NULL;
		}
		else
		{
			/* Ainda tem coisa da fila. */
			*queue = elem->next;
		}
	}

	elem->next = NULL;
	elem->prev = NULL;

	return elem;
}

int queue_size (queue_t *queue)
{
	queue_t *aux = NULL;
	int size = 1;

	/* Verificar se a fila nao esta vazia */
	if (queue == NULL)
	{
		#ifdef DEBUG
		printf ("Fila vazia.\n");
		#endif
		return 0;
	}

	/* Inicia no primeiro elemento da fila (size = 1) */
	aux = queue;

	/* Enquanto o proximo element nao for o primeiro, */
	/* nao atingimos o ultimo elemento da fila */
	while (aux->next != queue)
	{
		/* Aumentar o tamanho em 1 e ir para o proximo elemento */
		size++;
		aux = aux->next;
	}

	/* Ao final, nao chegamos no ultimo elemento, porem comecamos a soma */
	/* com tamanho 1, logo nao precisamos fazer o acerto. */

	#ifdef DEBUG
	printf ("Fila de tamanho %d.\n", size);
	#endif

	return size;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) )
{
	queue_t *aux;

	printf ("%s: [",name);

	/* Fila esta vazia? */
	if (queue == NULL)
	{
		printf ("]\n");
		return;
	}

	/* Comeca no inicio da fila */
	aux = queue;

	do
	{
		/* A partir do segundo, coloca espaco */
		if (aux != queue)
			printf (" ");
		/* Chamar a funcao de print para o elemento atual */
		(void)print_elem(aux);
		/* Seguir para o proximo */
		aux = aux->next;

		/* Atingimos o final? */
	} while (aux != queue);

	printf ("]\n");

	return;
}