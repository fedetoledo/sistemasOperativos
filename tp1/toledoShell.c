#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

/*
SOPORTA -----
* Ejecutar comandos con argumentos
* Ejecutar comandos en foreground
* Ejecutar comandos en background
* Cambiar de directorio 
* Redireccionar salida a archivo
*/

#define MAX_LARGO_SWITCH	50 	//Un switch de comando no puede tener mas de MAX_LARGO_SWITCH caracteres
#define MAX_LARGO_CMD		512 //Un comando no puede tener mas de MAX_LARGO_CMD caracteres
#define MAX_SWITCHES_CMD 	100 //Un comando puede tener hasta MAX_SWITCHES_CMD switches
#define CHEQUEO_ZOMBIES 	25 	//Se chequea y mata zombies cada CHEQUEO_ZOMBIES segundos

//Signal handlers
void mata_zombies(int);
void fallo_cmd_background(int);

void prompt();
void printHelp();
void ingreso_comando(char *);
void borrar_espacios_final(char *);
void borrar_espacios_inicio(char *);
void borrar_caracteres_control(char *);
void borrar_ampersand(char *);
void parse_comando(char *, int, char *[]);
char *printDir();
int cd(char *);
int comando_background(char *);
int comando_redireccion(char *);

//Ultimo pid creado
pid_t ultimo_pid = 0;

int main(int argc, char **argv) {
	char comando[MAX_LARGO_CMD];
	char *comando_argv[MAX_SWITCHES_CMD];
	int retorno;

	signal(SIGALRM, mata_zombies);
	signal(SIGUSR1, fallo_cmd_background);
	alarm(CHEQUEO_ZOMBIES);

	do {


		ingreso_comando(comando);
		parse_comando(comando, MAX_SWITCHES_CMD, comando_argv); //Parseo el comando para dividir los switches

		//Comandos del shell

		//Exit
		if (strncmp(comando, "exit", 4) == 0) break;

		//Ayuda
		if (strncmp(comando, "help", 4) == 0) {
			printHelp();
			continue;
		}

		//Cambiar directorio
		if (strncmp(comando, "cd", 2) == 0) {
			if (comando_argv[1] > 0 && cd(comando_argv[1]) < 0) {
				perror(comando_argv[1]);
			}
			continue; //Salta el fork
		}

		pid_t pid = fork(); //Mientras no sea el comando exit, creo un proceso

		if (pid == 0) { //Proceso hijo
			signal(SIGALRM, SIG_DFL); //Le devuelvo sus acciones por defecto a las senales
			signal(SIGUSR1, SIG_DFL);


			int back = comando_background(comando);

			if ( comando_background(comando) ) borrar_ampersand(comando); //Si es background, borro el ampersand

			parse_comando(comando, MAX_SWITCHES_CMD, comando_argv); //Parseo el comando para dividir los switches
			int i = 0;

			//REDIRECCION DE SALIDA
			//printf("es redirect?: %d\n", comando_redireccion(comando));
    		if (comando_redireccion(comando)) {

				char *archivo;
				int lastIndex = 0;
				int i;
				
				//Busco la ultima posicion del vector
				for (i = 0; comando_argv[i] != NULL; i++);
				lastIndex = i-1;
				archivo = comando_argv[lastIndex];
				
    			comando_argv[lastIndex-1] = '\0'; //Elimino > para que funcione el comando
    			comando_argv[lastIndex] = '\0'; //Elimino el nombre del archivo 

	    		int outfile = open(archivo, O_CREAT | O_WRONLY, S_IRWXU);
				if (outfile == -1){
	      			fprintf(stderr, "Error: fallo en la creacion del archivo %s\n", archivo);
	    		} else {
	    			//redirecciona stdout desde el proceso hijo al archivo.
	      			if(dup2(outfile, STDOUT_FILENO) != STDOUT_FILENO){
	        			fprintf(stderr, "Error: fallo la redireccion de stdout\n");
	      			}
					execvp(comando_argv[0], comando_argv);
					printf("Comando [%s] no encontrado\n", comando);
				}
			} else {
				execvp(comando_argv[0], comando_argv);
				printf("Comando [%s] no encontrado\n", comando);
			}

			if (back) kill(getppid(), SIGUSR1); //Envio senal al padre para notificar fallo en hijo
			exit(127); //Llamada al sistema para terminar proceso con determinado valor de retorno
		} else { // proceso padre
			ultimo_pid = pid;
			if ( !comando_background(comando) ) {
				pid = waitpid(pid, &retorno, 0); //No usar wait()
				if (WEXITSTATUS(retorno) != 127) {
					//printf("Proceso %d finalizo con status = %d\n", pid, WEXITSTATUS(retorno));
				}
			}
		}

	} while(strncmp(comando, "exit", 4));
	//Antes de finalizar, elimino todo posible proceso zombie
	//Pueden quedar procesos huerfanos en ejecucion background
	mata_zombies(0);
	return 0;
}

//FUNCIONES -----------------------

void ingreso_comando(char *cmd) {
	prompt();
	memset(cmd, 0, MAX_LARGO_CMD);
	//en cmd me quedan los caracteres tipeados + \n + \0
	fgets(cmd, MAX_LARGO_CMD, stdin);

	//Con esta funcion reemplazo el \n al final del comando por \0
	borrar_caracteres_control(cmd);
	borrar_espacios_final(cmd);
	borrar_espacios_inicio(cmd);
}

int comando_background(char *cmd) {
	int n = strlen(cmd)-1;
	return( n >= 0 && cmd[n] == '&');
}

void borrar_espacios_final(char *cmd) {
	int n = strlen(cmd) - 1;
	while( n >= 0 && cmd[n] == ' ') {
		cmd[n] = '\0';
		n--;
	}
}

void borrar_espacios_inicio(char *cmd) {
	char *p = cmd;
	while( *p && *p == ' ') p++;
	if (p == cmd) return; //no tenia espacios al inicio
	char tmp[MAX_LARGO_CMD];
	strcpy(tmp,p);
	strcpy(cmd,tmp);
}

void borrar_caracteres_control(char *cmd) {
	char *p = cmd;
	while(*p) {
		switch(*p) {
			case '\n':
			case '\r':
			case '\t':
			case '\b':
				*p = ' ';
				break;
		}
		p++;
	}
}

void borrar_ampersand(char *cmd) {
	cmd[strlen(cmd)-1]= '\0'; // borro &
	borrar_espacios_final(cmd);
}

void parse_comando(char *cmd, int argc, char *argv[]) {
	int i = 0;
	int pos1 = 0;
	int pos2 = 0;
	int nc = 0;
	int tope = strlen(cmd);
	do {
		pos1= i;
		while(i < tope && cmd[i] != ' ') i++;
		pos2= i;
		if ( pos1 == pos2 ) break;
		if ( (pos2 - pos1) > MAX_LARGO_SWITCH) {
			printf("parseo_cmd(): argumento %d de comando [%s] supera los %d caracteres\n",nc,cmd,MAX_LARGO_SWITCH);
			break;	
		}
		argv[nc] = strndup(cmd+pos1,pos2-pos1);
		//avanzo hasta el proximo switch
		while( i < tope && cmd[i] == ' ') i++;
		nc++;
	} while( nc < argc && i < tope);
	argv[nc] = NULL;
}

//SIGNAL HANDLERS --------------

void mata_zombies(int signo) {
	int retorno = 0;
	pid_t pid = 0;
	while((pid = waitpid(0, &retorno, WNOHANG)) > 0) {
		//Mata un zombie
	}
	alarm(CHEQUEO_ZOMBIES);
}

void fallo_cmd_background(int signo) {
	int retorno;
	waitpid(ultimo_pid, &retorno, 0);
}

//EXTRA ----------------
void prompt() {
	char *user = getenv("USER");
	char *desktop = getenv("DESKTOP_SESSION");
	printf("\033[0;32m");
	printf("%s@%s:", user, desktop);
	printf("\033[0;33m");
	printf("%s", printDir());
	printf("\033[0m");
	printf("$ ");
}

char * printDir() {
	char cwd[1024];
	return getcwd(cwd, sizeof(cwd));
}

int cd (char * path) {
	return chdir(path);
}

int comando_redireccion(char *cmd) {

	for (int i=0; i< strlen(cmd); i++) {
		if (cmd[i] == '>') {
			return 1;
		}

	}
	return 0;
}

void printHelp() {
	printf("\n");
	printf("******* toledoShell *******\n");
	printf("Comandos disponibles\n");
	printf("cd <path> - Cambia el directorio de trabajo\n");
	printf("help - Muestra la ayuda\n");
	printf("exit - Sale del shell\n");
	printf("comando > archivo - redirecciona la salida a archivo\n");
}