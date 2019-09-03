#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
// Implementacion de una sincronizacion de los hilos a,b,c y
// de forma tal, que la secuencia de ejecucion y acceso a su
// seccion critica sea la siguiente: ABAC... detener el proceso
// luego de 21 iteraciones completas. Basarse en la practica hecha
// en clase. Resolver la sincronizacion con variables Mutex
// y Saforos Posix sin nombre.

void *hiloA();
void *hiloB();
void *hiloC();

int count = 1;
// Unnamed Saphores
sem_t Sa, Sb, Sc, Sx;

int main(int argc, char **argv) {
	pthread_t hA,hB,hC;
	//Cambio los valores iniciales de los Saforos a y b
	sem_init(&Sa,0,1);
	sem_init(&Sb,0,1);
	sem_init(&Sc,0,0);
	sem_init(&Sx,0,0);
	// system("ipcs");
	// printf("lanzo hilo A\n");
	pthread_create(&hA, NULL, hiloA, NULL);
	// printf("lanzo hilo B\n");
	pthread_create(&hB, NULL, hiloB, NULL);
	// printf("lanzo hilo C\n");
	pthread_create(&hC, NULL, hiloC, NULL);
	// printf("espera por fin de hilo A\n");
	pthread_join(hA,NULL);
	sem_post(&Sb);
	sem_post(&Sc);
	sem_post(&Sx);
	// printf("espera por fin de hilo B\n");
	pthread_join(hB,NULL);
	// printf("espera por fin de hilo C\n");
	pthread_join(hC,NULL);
	sem_destroy(&Sa);
	sem_destroy(&Sb);
	sem_destroy(&Sc);
	sem_destroy(&Sx);
	printf("fin main\n");
	return 0;
}

void *hiloA() {
	// printf("Hilo A en ejecucion\n");
	while(1) {
		sem_wait(&Sa);
		printf("Seccion critica A\n");
		sem_post(&Sx);
	}
	printf("Hilo A, fin\n");
	pthread_exit(0);
}

void *hiloB() {
	// printf("Hilo B en ejecucion\n");
	while(1) {
		sem_wait(&Sb);
		sem_wait(&Sx);
		printf("seccion critica B\n");
		sem_post(&Sa);
		sem_post(&Sc);
	}
	printf("Hilo B, fin\n");
	pthread_exit(0);
}

void *hiloC() {
	// printf("Hilo C en ejecucion\n");
	while(1) {
		sem_wait(&Sc);
		sem_wait(&Sx);
		printf("seccion critica C\n");
		if(count < 20) {
			printf("fin ciclo %d\n", count);
			count++;
		} else {
			printf("fin ciclo %d\n", count);
			exit(0);
		}
		sem_post(&Sa);
		sem_post(&Sb);
	}
	printf("Hilo C, fin\n");
	pthread_exit(0);
}