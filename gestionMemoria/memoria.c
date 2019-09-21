#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <ctype.h>
#include <string.h>

#define MAX 10
#define MAX_MEMORIA 100


//Estructura que contiene los datos de los procesos
typedef struct {
	char nombre;
	int size;
	int enMemoria;
} task;

//Estructura que contiene los datos de las particiones
typedef struct {
	int numero;
	int size;
	int base;
	int ocupada; //0 o 1
	task proceso;
} particion;

//Recreacion de administracion de memoria particionada fija con
//particiones de distinto tamaño
void crearMemoria();
void interfaz();
void opciones();
void crearConfigFile();
void leerConfigFile();
void mostrarDatosAdmin();
void cargarProceso();
void mostrarSHM();
void mostrarProcesos();
void terminarProceso();
particion liberarParticion(char);
void liberarMemoria(particion);
void limpiarMemoria();
void mostrarFragInterna();
void primerAjuste(task*);

//Vector de procesos a cargar en memoria
task procesos[MAX];
particion particiones[MAX]; 

int main(void) {
	printf("\n********Gestion de memoria - Primer Ajuste********\n\n");
	printf("Para cargar procesos primero crear memoria y particiones\n");
	opciones();
	int shmid;
	if( (shmid = shmget(0xa,0,0)) != -1) {
		printf("Eliminando memoria compartida %d\n", shmid);
		// int shmctl(int shmid, int cmd, struct shmid_ds *buf);
		if(shmctl(shmid, IPC_RMID, NULL)) perror("shmctl");
	} else perror("shmid");
	printf("Programa terminado\n");
	return 0;
}

void interfaz() {
	printf("\n\n*****************  MENU  *****************\n");
	printf("1 - Crear memoria compartida\n");
	printf("2 - Crear archivo de configuracion\n");
	printf("3 - Leer archivo de configuracion\n");
	printf("4 - Cargar proceso en memoria\n");
	printf("5 - Terminar Proceso\n");
	printf("6 - Mostrar estructura de datos de administarcion\n");
	printf("7 - Mostrar fragmentacion interna de particion\n");
	printf("8 - Mostrar contenido de la memoria compartida\n");
	printf("9 - Mostrar procesos activos\n");
	printf("0 - Salir\n");
	printf("*******************************************\n");
	printf("==> ");
}

void opciones() {
	int sigue = 1;
	int i;
	do {
		interfaz();
		char opcion;
		scanf(" %c", &opcion);
		opcion = toupper(opcion);
		system("clear");
		switch(opcion) {
			case '1':
				crearMemoria();
				break;
			case '2':
				crearConfigFile();
				break;
			case '3':
				leerConfigFile();
				break;
			case '4':
				cargarProceso();
				break;
			case '5':
				terminarProceso();
				break;
			case '6':
				mostrarDatosAdmin();
				break;
			case '7':
				mostrarFragInterna();
				break;
			case '8':
				mostrarSHM();
				break;
			case '9':
				mostrarProcesos();
				break;
			case '0':
				limpiarMemoria();
				sigue = 0;
				break;
			case 'Q':
				printf("Salida sin borrar memoria\n");
				exit(0);
			default:
				printf("Elija una opcion correcta\n\n");
		}
		for(i=0; i < MAX; i++)
			if(procesos[i].size && !procesos[i].enMemoria)
				primerAjuste(&procesos[i]);
	} while(sigue);
}

//1 - CREAR MEMORIA COMPARTIDA POR TECLADO
void crearMemoria() {
	printf("\nCREAR MEMORIA COMPARTIDA\n");
	int shmid;
	if ((shmid = shmget(0xa,0,0)) != -1) printf("Ya hay una memoria inicializada\n"); 
	else {
		int memSize;
		printf("Ingrese cantidad de memoria: ");
		scanf("%d", &memSize);
		shmid = shmget(0xa, memSize, IPC_CREAT|IPC_EXCL|0600);
		if(shmid != -1) {
			char *map = (char*) shmat(shmid,0,0);
			//memoria llenada con * (asteriscos)
			memset(map,'*',memSize);
			shmdt(map);
		} else printf("Error al crear la memoria\n");
	}
}

//2 - CREAR ARCHIVO DE CONFIGURACION DE PARTICIONES
void crearConfigFile() {
	printf("\nCREAR ARCHIVO DE CONFIGURACION\n");
	//FILE * fopen (const char *filename, const char *opentype);
	int cantidad, i;
	int base = 0;
	FILE *config;
	printf("Ingrese cantidad de pariciones: ");
	scanf("%d", &cantidad);
	config = fopen("config", "w");
	for(i=0; i < cantidad; i++) {
		int size;
		printf("Particion %d\n", i);
		printf("Tamaño: ");
		scanf("%d", &size);
		particion datos = {i, size, base, 0};
		fwrite(&datos, sizeof(particion), 1, config);
		// if(fwrite) printf("Agregado corectamente\n");
		// else printf("Error escribiendo en archivo\n");
		base += size;
	}
	fclose(config);
}

//3 - LEER ARCHIVO DE CONFIGURACION Y ESTABLECER PARTICIONES
void leerConfigFile() {
	printf("\nLEER ARCHIVO DE CONFIGURACION\n");
	FILE *config;
	config = fopen("config", "r");
	particion datos;
	int i;
	while(fread(&datos, sizeof(particion), 1, config)) {
		printf("Particion %d\n",datos.numero);
		printf("Size: %d | Base: %d | Ocupada: %d\n\n",datos.size,datos.base,datos.ocupada);
		particiones[i].numero = datos.numero;
		particiones[i].size = datos.size;
		particiones[i].base = datos.base;
		particiones[i].ocupada = datos.ocupada;
		particiones[i].proceso = datos.proceso;

		i++;
	}
	fclose(config);
}

//4 - CARGAR PROCESO EN MEMORIA SI HAY LUGAR
void cargarProceso() {
	printf("\nCARGAR PROCESO\n");
	if(particiones[0].size) {	
		task proceso;
		printf("Ingrese nombre (1 letra mayuscula): ");
		scanf(" %c", &proceso.nombre);
		proceso.nombre = toupper(proceso.nombre);
		printf("Ingrese tamaño (en bytes): ");
		scanf("%d", &proceso.size);
		proceso.enMemoria = 0;
		int i;
		for(i=0; i < MAX; i++) {
			if(!procesos[i].size) {
				procesos[i] = proceso;
				break;
			}
		}
		// primerAjuste(proceso);
	} else printf("No hay particiones cargadas\n");
}

//5 - TERMINAR PROCESO N
void terminarProceso() {
	int i;
	int found=0;
	char nombre;
	printf("\nTERMINAR PROCESO\n\n");
	printf("Ingrese nombre del proceso: ");
	scanf(" %c", &nombre);
	nombre = toupper(nombre);
	for(i=0; i < MAX; i++) {
		char buscado = procesos[i].nombre;
		if(nombre == buscado) {
			procesos[i].nombre = '\0';
			procesos[i].size = 0;
			found=1;
			particion pLiberada = liberarParticion(nombre);
			liberarMemoria(pLiberada);
			printf("Proceso %c terminado\n", nombre);
			break;
		}
	}
	if(!found) printf("No existe el proceso %c\n", nombre);
}

//6 - MOSTRAR ESTRUCTURAS DE DATOS DE ADMINISTRACION
void mostrarDatosAdmin() {
	printf("\nMOSTRAR ESTRUCTURA DE DATOS DE ADMINISTRACION\n");
	int i;
	int count=0; //para mostrar si hay o no particiones iniciadas
	for(i=0; i < MAX; i++) {
		if(particiones[i].size) {
			particion p = particiones[i];
			printf("Particion %d | size: %d | base: %d | ocupada: %d | proceso: %c | sizeProceso %d\n", 
				p.numero,
				p.size,
				p.base,
				p.ocupada,
				p.proceso.nombre,
				p.proceso.size
			);
			count++;
		}
	}
	if(!count) printf("\nNo hay particiones cargadas. Cargue un archivo de configuracion\n");
}

//7 - MOSTRAR FRAGMENTACION INTERNA DE PARTICION N
void mostrarFragInterna() {
	printf("\nMOSTRAR FRAGMENTACION INTERNA\n");
	int i;
	int count=0;
	for(i=0; i<MAX; i++)
		if(particiones[i].size && particiones[i].proceso.nombre) {
			int fragInterna = particiones[i].size - particiones[i].proceso.size;
			printf("Particion %d: %d\n", i, fragInterna);
			count++;
		}
	if(!count) printf("No hay fragmentacion interna en niguna particion\n");
}

//8 - MOSTRAR CONTENIDO DE LA MEMORIA COMPARTIDA
void mostrarSHM() {
	printf("\nMOSTRAR CONTENIDO DE LA MEMORIA COMPARTIDA\n");
	int shmid;
	if( (shmid = shmget(0xa,0,0)) != -1) {
		printf("shmid = %d\n", shmid);
		char *contenido = (char*) shmat(shmid,0,0);
		printf("\nContenido: %s\n", contenido);
		shmdt(contenido);
	} else printf("\nNo se ha inicializado memoria compartida\n");
}

//9 - MOSTRAR PROCESOS ACTIVOS
void mostrarProcesos() {
	int count=0;
	printf("\nMOSTRAR PROCESOS CARGADOS\n");
	for(int i=0;i< MAX;i++) {
		if(procesos[i].nombre) {
			printf("Proceso %c # Size: %d # EnMemoria? %d\n",
				procesos[i].nombre, procesos[i].size,procesos[i].enMemoria);
			count++;
		}
	}
	if(!count) printf("\nNo hay proceso cargados\n");	
}

//0 - LIMPIAR MEMORIA
void limpiarMemoria() {
	int shmid;
	if( (shmid = shmget(0xa,0,0)) != -1) {
		char *borrarTodo = (char*) shmat(shmid,0,0);
		memset(borrarTodo,'*',MAX_MEMORIA);
		shmdt(borrarTodo);
	} else perror("shmid");
}

//ALGORITMO DE POLITICA DE ASIGNACION DE MEMORIA 
void primerAjuste(task *proceso) {
	int count=0;
	for (int i = 0; i < MAX; ++i) {
		int enoughSize = particiones[i].size - proceso->size;
		int puedeCargarse = !particiones[i].ocupada && enoughSize >= 0;
		if(puedeCargarse) {
			particiones[i].ocupada = 1;
			particiones[i].proceso.nombre = proceso->nombre;
			particiones[i].proceso.size = proceso->size;
			particiones[i].proceso.enMemoria = 1;
			proceso->enMemoria = 1;
			printf("Particion %d ocupada por %c\n",particiones[i].numero, proceso->nombre);
			int shmid = shmget(0xa,0,0);
			char *nombreTask = (char*) shmat(shmid,0,0);
			memset(nombreTask + particiones[i].base , proceso->nombre, proceso->size);
			shmdt(nombreTask);
			count=0;
			break;
		} else { count++; }
	}
	if(count) printf("No hay espacio para el proceso %c\n", proceso->nombre);
}


particion liberarParticion(char nombreProc) {
	int j;
	for(j=0; j < MAX; j++) {
		particion *p;
		 p = &particiones[j];
		if(p->proceso.nombre == nombreProc) {
			// printf("particion: %d proceso %c\n",j, p->proceso.nombre);
			p->ocupada = 0;
			p->proceso.nombre = '\0';
			p->proceso.size = 0;
			printf("Particion %d liberada\n",j);
			return *p;
		}
	}
}

void liberarMemoria(particion p) {
	int shmid = shmget(0xa,0,0);
	if(shmid != -1) {
		char * letra = (char*) shmat(shmid,0,0);
		memset(letra + p.base,'*',p.size);
		shmdt(letra);
	} else perror("shmid");
}