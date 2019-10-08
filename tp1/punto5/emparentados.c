#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>

// Sincronizacion ABAC con procesos emparentados con semaforos con nombre
// Padre A 
// Hijo B
// Hijo C


/*
Necesito sincronizar ABAC:

B-A-C-A

mA=1
mB=1
mC=0
mX=0

semWait - lock
semSignal - unlock

hiloA			hiloB			hiloC
semWait(ma)		semWait(mb)		semWait(mc)
				semWait(mx)		semWait(mx)
sc 				sc 				sc
semSignal(mx)	semSignal(ma)	semSignal(ma)
				semSignal(mc)	semSignal(mb)
*/
int control = 0;

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
		printf("Hijo B pid: %d\n", getpid());
		while(1) {
			sem_wait(semB);
			sem_wait(semX);
				if(control > 19) exit(0);
				printf("Proceso B [SC]\n");
				// sleep(1);
			sem_post(semA);
			sem_post(semC);
		}
	} else {
		//padre A
		pid = fork();
		if(pid == 0){
			//hijo C
			printf("Hijo C pid: %d\n", getpid());
			while(1) {
				sem_wait(semC);
				sem_wait(semX);
					printf("Proceso C [SC] - %d\n", control);
					// sleep(1);
					control++;
				if(control > 19) {
					sem_post(semA);	
					sem_post(semB);	
					exit(0);
				}
				sem_post(semA);
				sem_post(semB);
			}
		} else {
			printf("PadreA pid: %d\n", getpid());
			//padre A
			while(1) {
				sem_wait(semA);
				if(control > 19) exit(0);
				printf("Proceso A [SC]\n");
				// sleep(1);
				sem_post(semX);
			}
		}
	}
	return 0;
}