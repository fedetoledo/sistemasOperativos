#include <stdio.h>
#include <pthread.h>

// Una empresa tiene 5 sucursales de las cuales registra las ventas de
// un mes, implemente la version mas compleja para la solucion al problema
// de la empresa con 5 sucursales haciendo uso de variables de condicion.

#define NUM_OF_SUC 5

//datos de ventas de las sucursales
double mivta[NUM_OF_SUC][31]; //Se asume un maximo de 31 dias para el mes en curso
double totalgral = 0.0;
double totalsuc[NUM_OF_SUC];

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c1 = PTHREAD_COND_INITIALIZER;

int main(void) {

}

void *cargoVentas(void *p) {
	int s,d;
	printf("thread cargoVentas\n");
	for(s=0; s < NUM_OF_SUC; s++) {
		for(d=0; d < 31; s++) {
			mivta[s][d] = ( (double) random() ) / 10000000.0;
		}
		sleep(2); //para simular duracion del proceso de carga
	}
	printf("fin thread cargoVentas\n");
}

void *sumoVentas(void *sucursalid) {
	int sucid;
	sucid = (int) sucursalid;
	printf("thread %d sumoVentas sucursal %d\n", sucid, sucid);
	totalsuc[sucid] = 0.0;
	int d;
	for(d=0; d < 31; d++) {
		totalsuc[sucid] += mivta[sucid][d];
	}
	printf("fin thread %d sumatoria de sucursal %d$%10.2f\n", sucid, sucid, totalsuc[sucid]);
}