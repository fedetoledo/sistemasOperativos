#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#define NUM_OF_SUC 5

// Problema: Una empresa tiene 5 sucursales de las cuales registra las 
// ventas de un mes, ejercicio propuesto en práctica titulada 
// “TP III – Procesos Livianos – Threads”; implemente la versión más 
// compleja para la solución al problema de la Empresa con 5 sucursales 
// haciendo uso de variables de condición. Solo realice el programa C, no 
// es necesario que adjunte con el código las respuestas a las preguntas 
// que se proponen en los puntos 5 y 6 de la práctica 
// “TP III – Procesos Livianos – Threads”.

//datos de ventas de las sucursales
double mivta[NUM_OF_SUC][31]; //Se asume un maximo de 31 dias para el mes en curso
double totalgral = 0.0;
double totalsuc[NUM_OF_SUC];

pthread_mutex_t cargada_mutex[NUM_OF_SUC];
pthread_cond_t cargada_cv[NUM_OF_SUC];

void *cargoVentas(void*);
void *sumoVentas(void*);

int cargadas[NUM_OF_SUC];

int main(void) {
	long s;
	pthread_t carga,suma;
	//inicializo las matrices y vector en 0
	memset(&cargadas,0,sizeof(int)*NUM_OF_SUC);
	memset(&mivta,0,sizeof(double)*NUM_OF_SUC*31);
	memset(&totalsuc,0,sizeof(double)*NUM_OF_SUC);

	printf("main(): creando thread para carga de ventas\n");
	//creo hilo de carga, solo necesito 1
	pthread_create(&carga, NULL, cargoVentas, NULL);

	//por cada sucursal creo un hilo de sumoVentas
	for(s=0; s< NUM_OF_SUC; s++) {
		printf("main(): creando thread(sucursal) %d\n", s);
		pthread_create(&suma,NULL,sumoVentas,(void*)s);

	}
	printf("main(): espera por fin de cargaVentas\n");
	pthread_join(carga, NULL);
	printf("main(): espera por fin de sumoVentas\n");
	pthread_join(suma,NULL);
	printf("main(): fin main\n");

	int t;
	for(t=0; t < NUM_OF_SUC; t++) totalgral += totalsuc[t];
	printf("main(): Total Gral $ %12.2f\n", totalgral);
}

void *cargoVentas(void *p) {
	int s,d;
	printf("thread cargoVentas\n");
	for(s=0; s < NUM_OF_SUC; s++) {
		//Cargo las ventas de una sucursal
		for(d=0; d < 31; d++) {
			mivta[s][d] = ( (double) random() ) / 10000000.0;
		}
		//Marco el flag de cargada
		cargadas[s]=1;
		//libero el mutex de la sucursal para sumar
		pthread_cond_signal(&cargada_cv[s]);
		printf("Espero por carga de sucursal %d\n", s);
		sleep(1); //para simular duracion del proceso de carga
	}
	printf("fin thread cargoVentas\n");
}

void *sumoVentas(void *sucursalid) {
	int sucid;
	sucid = (long) sucursalid;
	//bloqueo el mutex
	pthread_mutex_lock(&cargada_mutex[sucid]);

	if(!cargadas[sucid]) {
		//Si no esta cargada, bloqueo el mutex con condicion
		printf("thread %d esperando condicion de cargada para sucursal %d\n",sucid,sucid);
		pthread_cond_wait(&cargada_cv[sucid], &cargada_mutex[sucid]);
	}

	//cuando se libera el mutex(temino de cargar ventas) sumo todas
	printf("thread %d sumo ventas sucursal %d\n", sucid, sucid);
	totalsuc[sucid] = 0.0;
	int d;
	for(d=0; d < 31; d++) {
		totalsuc[sucid] += mivta[sucid][d];
	}
	//Libero el mutex de la sucurasal
	pthread_mutex_unlock(&cargada_mutex[sucid]);
	printf("fin thread %d sumatoria de sucursal %d $%10.2f\n", sucid, sucid, totalsuc[sucid]);
}