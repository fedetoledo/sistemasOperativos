#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
// Implementacion de una sincronizacion de los hilos a,b,c y
// de forma tal, que la secuencia de ejecucion y acceso a su
// seccion critica sea la siguiente: ABAC... detener el proceso
// luego de 21 iteraciones completas. Basarse en la practica hecha
// en clase. Resolver la sincronizacion con variables Mutex
// y semaforos Posix sin nombre.

void *hiloA();
void *hiloB();
void *hiloC();

int count = 1;
pthread_mutex_t mA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mC = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mX = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char const *argv[])
{
	pthread_t hA, hB, hC;
	pthread_mutex_lock(&mC); //C=0
	pthread_mutex_lock(&mX); //X=0
	printf("main lanza hiloA\n");
	pthread_create(&hA, NULL, hiloA, NULL);
	printf("main lanza hiloB\n");
	pthread_create(&hB, NULL, hiloB, NULL);
	printf("main lanza hiloC\n");
	pthread_create(&hC, NULL, hiloC, NULL);
	printf("main espera por fin de hiloA\n");
	pthread_join(hA, NULL);
	pthread_mutex_unlock(&mB);
	pthread_mutex_unlock(&mC);
	pthread_mutex_unlock(&mX);
	printf("main espera por fin de hiloB\n");
	pthread_join(hB, NULL);
	printf("main espera por fin de hiloC\n");
	pthread_join(hC, NULL);
	printf("finaliza main\n");
	return 0;
}

void *hiloA() {
	printf("Hilo A en ejecucion\n");
	while(1) {
		pthread_mutex_lock(&mA);
		printf("Seccion critica de A\n");
		pthread_mutex_unlock(&mX);
	}
	printf("Hilo A, fin\n");
	pthread_exit(0);
}

void *hiloB() {
	printf("Hilo B en ejecucion\n");
	while(1) {
		pthread_mutex_lock(&mB);
		pthread_mutex_lock(&mX);
		printf("Seccion critica de B\n");
		pthread_mutex_unlock(&mA);
		pthread_mutex_unlock(&mC);
	}
	printf("Hilo A, fin\n");
	pthread_exit(0);
}

void *hiloC() {
	printf("Hilo C en ejecucion\n");
	while(1) {
		pthread_mutex_lock(&mC);
		pthread_mutex_lock(&mX);
		printf("Seccion critica de C\n");
		if(count < 20){
			printf("Termina ciclo %d\n", count);
			count++;
		} else {
			printf("Termina ciclo %d\n", count);
			exit(0);

		}
		pthread_mutex_unlock(&mA);
		pthread_mutex_unlock(&mB);
	}
	printf("Hilo C, fin\n");
	pthread_exit(0);
}