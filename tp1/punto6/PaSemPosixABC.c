#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

int main(void) {
	sem_t *semA;
	sem_t *semB;
	sem_t *semC;
	sem_t *semX;

	semA = sem_open("/semA",0);
	semB = sem_open("/semB",0);
	semC = sem_open("/semC",0);
	semX = sem_open("/semX",0);

	int control = 0;
	int temp = 0;

	while(1) {
		sem_wait(semA);
			if(control % 2) {
				if(control < 20*2) { // 20 * cantidad de A's
					temp = control /2;
					printf("Proceso A [SC] - %d\n", temp);
					sleep(1);
				} else {
					exit(0);
				}
			} else {
				printf("Proceso A [SC]\n");
				sleep(1);
			}
			control++;
		sem_post(semX);
	}
}