#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>

#define MAX 5

// Sincronizacion ABAC con procesos emparentados con semaforos con nombre
// Padre A 
// Hijo B
// Hijo C

int main(int argc, char const *argv[])
{
	sem_t *semA;
	sem_t *semB;
	sem_t *semC;
	sem_t *semX;

	semA = sem_open("/semA",0);
	semB = sem_open("/semB",0);
	semC = sem_open("/semC",0);
	semX = sem_open("/semX",0);

	pid_t pid;

	pid = fork();

	if(pid == 0) {
		//hijo B
		int control = 1;
		printf("Hijo B pid: %d\n", getpid());
		while(1) {
			if(control > MAX) exit(0);
			sem_wait(semB);
			sem_wait(semX);
			printf("Proceso B [SC]\n");
			control++;
			// sleep(1);
			sem_post(semA);
			sem_post(semC);

		}
	} else {
		//padre A
		pid = fork();
		if(pid == 0){
			int control=1;
			//hijo C
			printf("Hijo C pid: %d\n", getpid());
			while(1) {
				if(control > MAX) exit(0);
				sem_wait(semC);
				sem_wait(semX);
				printf("Proceso C [SC] - %d\n", control);
				// sleep(1);
				control++;
				sem_post(semA);
				sem_post(semB);
			}
		} else {
			printf("PadreA pid: %d\n", getpid());
			//padre A
			int control = 1;
			while(1) {
				if(control > MAX*2) exit(0);
				sem_wait(semA);
				printf("Proceso A [SC]\n");
				control ++;
				// sleep(1);
				sem_post(semX);

			}
		}
	}
	return 0;
}