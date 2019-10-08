#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

// Implemente un proceso que quede bloqueado a la espera de recibir 
// la señal SIGUSR1 (antes de ello, que indique en pantalla cuál 
// es su process id (PID)) cuando reciba esta señal el proceso deberá 
// imprimir el abecedario en minúsculas y luego volverá a quedar 
// bloqueado; la próxima vez que reciba SIGUSR1 imprimirá el abecedario 
// en mayúsculas y así sucesivamente hasta que reciba SIGUSR2 en ese
// caso, el programa terminará su ejecución. Recomendación: abrir 2 
// consolas, en una consola ejecutar el proceso y en otra utilizarla 
// para enviar las señales SIGUSR1 y SIGUSR2 al proceso. Aplique todo 
// lo aprendido en la práctica “TP IV – Señales - Signals”.

void handlerABC(int);
void finish(int);
int control=1;
int turno = 0;;
int main(int argc, char const *argv[])
{
	printf("pid: %d\n", getpid());
	signal(SIGUSR2, finish);
	while(control) {
		signal(SIGUSR1, handlerABC);
		pause();
	}

	return 0;
}

void handlerABC(int signo) {
	char letra;
	if( (turno % 2) == 0) {
		letra = 'a';
		while(letra <= 'z') {
			printf("letra %c\n", letra);
			letra++;
		}
	} else {
		letra = 'A';
		while(letra <= 'Z') {
			printf("letra %c\n", letra);
			letra++;
		}
	}
	turno++;
}

void finish(int signo) {
	printf("recibi senal %d\n", signo);
	control = 0;
}