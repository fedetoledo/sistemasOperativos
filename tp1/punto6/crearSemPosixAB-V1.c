#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
// Semaforos nombrados
// Se crean en /dev/shm
// por ejemplo con el nombre semA
// entonces esta en /dev/shm/sem.semA
// Si el nombre se le antepone / como en semA = sem_open("/semA",O_CREAT | O_EXCL, 0644, 1)
// entonces el semaforo es compartido por los procesos independientes
// Si al nombre no se le antepone / como por ejemplo semA = sem_open("semA", O_CREAT|O_EXCL, 0644, 1)
// entonces el semaforo es compartido solo por los procesos emparentados.

int main(void) {
	sem_t *semA;
	sem_t *semB;
	sem_t *semC;
	sem_t *semX;

	int ValsemA;
	int ValsemB;
	int ValsemC;
	int ValsemX;

	sem_unlink("/semA");
	sem_unlink("/semB");
	sem_unlink("/semC");
	sem_unlink("/semX");

	semA = sem_open("/semA", O_CREAT | O_EXCL, 0644,0); 
	semB = sem_open("/semB", O_CREAT | O_EXCL, 0644,1); 
	semC = sem_open("/semC", O_CREAT | O_EXCL, 0644,0); 
	semX = sem_open("/semX", O_CREAT | O_EXCL, 0644,1); 

	sem_getvalue(semA, &ValsemA);
	sem_getvalue(semB, &ValsemB);
	sem_getvalue(semC, &ValsemC);
	sem_getvalue(semX, &ValsemX);

	printf("/semA = %d\n", ValsemA);
	printf("/semB = %d\n", ValsemB);
	printf("/semC = %d\n", ValsemC);
	printf("/semX = %d\n", ValsemX);
}