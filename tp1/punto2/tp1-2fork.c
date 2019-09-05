#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

// Implementar un proceso al cual se le indique por linea de comando
// la cantidad de procesos a crear, todos los procesos a crear seran
// hermanos; cada uno de ellos retornara un valor entero distinto al
// proceso padre y emitira un mensaje en pantalla antes de finalizar
// El proceso padre reportara por pantalla el retorno recibido de cada
// proceso hijo y no permitira que existan procesos huerfanos o zombies

int main(int argc, char const *argv[])
{
	int num = atoi(argv[1]);
	pid_t pid;
	printf("proceso padre %d\n", getpid());
	for (int i=0; i < num; i++) {
		if(fork() == 0) {
			printf("hijo %d de padre %d\n", getpid(), getppid());
			exit(0);
		} else {
			printf("proceso %d\n", getpid());
			wait(0);
		}

	}
	return 0;
}