#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pingpong.h"

unsigned int seed;
unsigned int item;
unsigned int buffer[5];
short iterator_p = 0;
short iterator_c = 0;

task_t p1, p2, p3, c1, c2;
semaphore_t s_vaga, s_buffer, s_item;

void Produtor(void* arg)
{
	#ifdef DEBUG
	printf("Produtor: iniciando ...\n");
	#endif
	while(1) {

		task_sleep(1);
		
		/* Solicitar uma vaga */
		sem_down(&s_vaga);
		/* Utilizar o buffer */
		sem_down(&s_buffer);

		/* Produzir um novo item e insere item no buffer */
		item = rand()%100;
		buffer[iterator_p == 5 ? iterator_p = 0 : ++iterator_p] = item;
		printf("%s produziu %d\n", (char*)arg, item);

		/* Liberar o buffer */
		sem_up(&s_buffer);
		/* Aumentar quantidade de itens */
		sem_up(&s_item);

	}
	task_exit(0);
}

void Consumidor(void* arg)
{
	#ifdef DEBUG
	printf("Consumidor: iniciando ...\n");
	#endif
	while(1) {

		/* Diminuir quantidade de itens */
		sem_down(&s_item);
		/* Utilizar o buffer */
		sem_down(&s_buffer);

		/* Retira item do buffer */
		item = buffer[iterator_c == 5 ? iterator_c = 0 : ++iterator_c];
		printf("								%s consumiu %d\n", (char*)arg, item);

		/* Liberar o buffer */
		sem_up(&s_buffer);
		/* Liberar uma vaga */
		sem_up(&s_vaga);

		// Print item
		task_sleep(1);

	}
	task_exit(0);
}

int main(int argc, char *argv[])
{
	seed = time(NULL);
	printf("Running with rand seed = %d\n", seed);
	srand(seed);

	printf("Main INICIO\n");
	pingpong_init();

	sem_create(&s_vaga, 5);
	sem_create(&s_buffer, 1);
	sem_create(&s_item, 0);

	task_create(&p1, Produtor, "p1");
	task_create(&p2, Produtor, "p2");
	task_create(&p3, Produtor, "p3");
	task_create(&c1, Consumidor, "c1");
	task_create(&c2, Consumidor, "c2");

	printf("Main FIM\n");
	task_exit(0);
	exit(0);
}