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
void ocuparParticion(int,task*);
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
particion * liberarParticion(char);
void liberarMemoria(particion);
void limpiarMemoria();
void mostrarFragInterna();
void primerAjuste(task*);
void peorAjuste(task*);
void mejorAjuste(task*);
void politicaElegida(int,task*);
int elegirPolitica();

//Vector de procesos a cargar en memoria
task procesos[MAX];
particion particiones[MAX]; 
void * politicas[3];
char* nombrePol = "(Primer Ajuste por defecto)";

int main(void) {
	printf("\n************** GESTION DE MEMORIA - Federico Toledo *************\n");
	printf("\nIMPORTANTE: Para cargar procesos primero crear memoria y particiones\n");
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
	printf("\n\n*****************  MENU  *****************\n\n");
	printf("     P - Elegir politica de asignacion\n");
	printf("     1 - Crear memoria compartida\n");
	printf("     2 - Crear archivo de configuracion\n");
	printf("     3 - Leer archivo de configuracion\n");
	printf("     4 - Cargar proceso en memoria\n");
	printf("     5 - Terminar Proceso\n");
	printf("     6 - Mostrar particiones\n");
	printf("     7 - Mostrar fragmentacion interna\n");
	printf("     8 - Mostrar contenido SHM\n");
	printf("     9 - Mostrar procesos activos\n");
	printf("     0 - Salir\n");
	printf("\n*******************************************\n");
	printf("==> ");
}

void opciones() {
	int sigue = 1;
	int politica = 0;
	int i;
	do {
		printf("\n************** GESTION DE MEMORIA - Federico Toledo *************\n");
		printf("\nPolitica de asignacion: %s\n", nombrePol);
		interfaz();
		char opcion;
		scanf(" %c", &opcion);
		opcion = toupper(opcion);
		system("clear");
		switch(opcion) {
			case '1':
				printf("\nCREAR MEMORIA COMPARTIDA\n");
				crearMemoria();
				break;
			case '2':
				printf("\nCREAR ARCHIVO DE CONFIGURACION\n");
				crearConfigFile();
				break;
			case '3':
				printf("\nLEER ARCHIVO DE CONFIGURACION\n");
				leerConfigFile();
				break;
			case '4':
				printf("\nCARGAR PROCESO\n");
				cargarProceso();
				break;
			case '5':
				printf("\nTERMINAR PROCESO\n\n");
				terminarProceso();
				break;
			case '6':
				printf("\nMOSTRAR ESTRUCTURA DE DATOS DE ADMINISTRACION\n");
				mostrarDatosAdmin();
				break;
			case '7':
				printf("\nMOSTRAR FRAGMENTACION INTERNA\n");
				mostrarFragInterna();
				break;
			case '8':
				printf("\nMOSTRAR CONTENIDO DE LA MEMORIA COMPARTIDA\n");
				mostrarSHM();
				break;
			case '9':
				printf("\nMOSTRAR PROCESOS CARGADOS\n");
				mostrarProcesos();
				break;
			case 'P':
				politica = elegirPolitica();
				break;
			case '0':
				limpiarMemoria();
				sigue = 0;
				break;
			default:
				printf("Elija una opcion correcta\n\n");
		}
		//busca procesos sin memoria y les asigna si es posible
		for(i=0; i < MAX; i++)
			if(procesos[i].size && !procesos[i].enMemoria)
				politicaElegida(politica, &procesos[i]);
	} while(sigue);
}

int elegirPolitica() {
	int eleccion;
	printf("ELECCION POLITICA DE ASIGNACION\n");
	printf("0 - Primer ajuste\n");
	printf("1 - Mejor ajuste\n");
	printf("2 - Peor ajuste\n");
	printf("==> ");
	scanf(" %d", &eleccion);
	switch(eleccion) {
		case 0:
			nombrePol = "Primer Ajuste";
			return 0;
		case 1:
			nombrePol = "Mejor Ajuste";
			return 1;
		case 2:
			nombrePol = "Peor Ajuste";
			return 2;
		default:
			return -1;
	}
}

void politicaElegida(int num, task *proceso) {
	switch(num) {
		case 0:
			primerAjuste(proceso);
			break;
		case 1:
			mejorAjuste(proceso);
			break;
		case 2:
			peorAjuste(proceso);
			break;
	}
}

//1 - CREAR MEMORIA COMPARTIDA POR TECLADO
void crearMemoria() {
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
	FILE *config;
	config = fopen("config", "r");
	particion datos;
	int i;
	while(fread(&datos, sizeof(particion), 1, config)) {
		printf("Particion %d\n",datos.numero);
		printf("Size: %d | Base: %d | Ocupada: %d\n\n",datos.size,datos.base,datos.ocupada);
		particiones[i] = datos;
		i++;
	}
	fclose(config);
}

//4 - CARGAR PROCESO EN COLA PARA POSTERIOR ASIGNACION A MEMORIA
void cargarProceso() {
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
	} else printf("No hay particiones cargadas\n");
}

//5 - TERMINAR PROCESO N
void terminarProceso() {
	int i;
	int found=0;
	char nombre;
	printf("Ingrese nombre del proceso: ");
	scanf(" %c", &nombre);
	nombre = toupper(nombre);
	for(i=0; i < MAX; i++) {
		char buscado = procesos[i].nombre;
		if(nombre == buscado) {
			procesos[i].nombre = '\0';
			procesos[i].size = 0;
			found=1;
			particion * pLiberada = liberarParticion(nombre);
			liberarMemoria(*pLiberada);
			printf("Proceso %c terminado\n", nombre);
			break;
		}
	}
	if(!found) printf("No existe el proceso %c\n", nombre);
}

//6 - MOSTRAR ESTRUCTURAS DE DATOS DE ADMINISTRACION
void mostrarDatosAdmin() {
	int i;
	int count=0; //para mostrar si hay o no particiones iniciadas
	for(i=0; i < MAX; i++) {
		if(particiones[i].size) {
			particion p = particiones[i];
			printf("Particion %d | size: %d | base: %d | ocupada: %d | proceso: %c | sizeProceso %d\n", 
				p.numero,p.size,p.base,p.ocupada,p.proceso.nombre,p.proceso.size
			);
			count++;
		}
	}
	if(!count) printf("\nNo hay particiones cargadas. Cargue un archivo de configuracion\n");
}

//7 - MOSTRAR FRAGMENTACION INTERNA DE PARTICION N
void mostrarFragInterna() {
	int i;
	int count=0;
	for(i=0; i<MAX; i++)
		if(particiones[i].size && particiones[i].proceso.nombre) {
			int fragInterna = particiones[i].size - particiones[i].proceso.size;
			printf("Particion %d: %d bytes\n", i, fragInterna);
			count++;
		}
	if(!count) printf("No hay fragmentacion interna en niguna particion\n");
}

//8 - MOSTRAR CONTENIDO DE LA MEMORIA COMPARTIDA
void mostrarSHM() {
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
	for(int i=0;i< MAX;i++) {
		if(procesos[i].nombre) {
			printf("Proceso %c # Size: %d # Asignado: %s\n",
				procesos[i].nombre, procesos[i].size,procesos[i].enMemoria ? "Si":"No");
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
	int i;
	for (i = 0; i < MAX; ++i) {
		int enoughSize = particiones[i].size - proceso->size;
		int puedeCargarse = !particiones[i].ocupada && enoughSize >= 0;
		if(puedeCargarse) {
			ocuparParticion(i, proceso);
			break;
		}
	}
}


//ALGORITMO DE POLITICA DE ASIGNACION DE MEMORIA
void mejorAjuste(task *proceso) {
	int i,dif,minIndex;
	int count=0;
	int min=10000;
	for(i=0; i < MAX; i++)
		if(particiones[i].size && !particiones[i].ocupada) {
			dif = particiones[i].size - proceso->size;
			if(dif < min && dif >= 0) {
				min = dif;
				minIndex = i;
				count++;
			}
		}
	if(count) {
		ocuparParticion(minIndex, proceso);
	}
}

void peorAjuste(task *proceso) {
	int i,dif,maxIndex;
	int count=0;
	int max=-10000;
	for(i=0; i < MAX; i++)
		if(particiones[i].size && !particiones[i].ocupada) {
			dif = particiones[i].size - proceso->size;
			if(dif > max && dif >= 0) {
				max = dif;
				maxIndex = i;
				count++;
			}
		}
	if(count) {
		ocuparParticion(maxIndex, proceso);
	}
}

void ocuparParticion(int indice, task *proceso) {
	particiones[indice].ocupada = 1;
	particiones[indice].proceso = *proceso;
	proceso->enMemoria = 1;
	printf("Particion %d ocupada por %c\n", particiones[indice].numero, proceso->nombre);
	int shmid;	
		if( (shmid = shmget(0xa,0,0)) != -1) {
			char *nombreTask = (char*) shmat(shmid,0,0);
			memset(nombreTask + particiones[indice].base , proceso->nombre, proceso->size);
			shmdt(nombreTask);
		}
}

particion * liberarParticion(char nombreProc) {
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
			return p;
		}
	}
	return NULL;
}

void liberarMemoria(particion p) {
	int shmid = shmget(0xa,0,0);
	if(shmid != -1) {
		char * letra = (char*) shmat(shmid,0,0);
		memset(letra + p.base,'*',p.size);
		shmdt(letra);
	} else perror("shmid");
}