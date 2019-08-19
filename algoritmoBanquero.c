#include <stdio.h>
#include <stdbool.h>

void ingresarDatos();
void calcularNA();
void mostrarDatos();
void liberarRecursos(int);
void limpiarProceso(int);

int asignacion[4][3] = {
	{1,0,0},
	{6,1,2},
	{2,1,1},
	{0,0,2}
};
int necesidad[4][3] = {
	{3,2,2},
	{6,1,3},
	{3,1,4},
	{4,2,2}
};
int na[4][3];
int asignados[5];
int disponible[3] = {0,0,0};
int recursos[3] = {9,3,6};
int corriendo[5];
int i, j, exec, r, p;
int count = 0;
bool safe = false;

int main(int argc, char const *argv[]) {


	ingresarDatos();
	calcularNA();
	mostrarDatos();

	while(count != 0) {
		safe = false;
		for(i=0; i < p; i++) { //por cada proceso
			if(corriendo[i]) { //si esta corriendo
				exec = 1; //lo ejecuto
				for(j=0; j < r; j++) { //por cada recurso
					//si na > disponible
					if (na[i][j] > disponible[j]) {
						exec = 0; //suspendo la ejecucion de ese proceso
						break; //corto el for
					}
				}

				if(exec) { //si se esta ejecutando
					printf("\nProceso %d en ejecucion\n", i+1);
					corriendo[i] = 0; //termina la ejecucion
					count--; //disminuyo cantidad de procesos corriendo
					safe = true; //es estado seguro
					for (j=0; j < r; j++) { //por cada recurso
						liberarRecursos(i);
						limpiarProceso(i); //limpio tabla
					}
					break;
				}
			}
		}
		
		if(!safe) {
			printf("Procesos en estado inseguro\n");
			break;
	
		}
		mostrarDatos();
	}

	if(safe) printf("ESTADO SEGURO\n");

	return 0;
}


void liberarRecursos(int i) {
	for (j=0; j < r; j++) {
		asignados[j] = recursos[j] - asignacion[i][j] - disponible[j];
		// printf("asignados en %d: %d\n",j+1, asignados[j]);
		disponible[j] = recursos[j] - asignados[j];
		// printf("disponible en %d: %d\n",j+1, disponible[j]);
	}

}

void limpiarProceso(int i) {
	for(j=0; j < r; j++) {
		necesidad[i][j] = 0;
		asignacion[i][j] = 0;
		na[i][j] = 0;
	}
}

void calcularNA() {
	for(i=0; i < p; i++)
		for(j=0; j < r; j++)
			na[i][j] = necesidad[i][j] - asignacion[i][j];
}

void ingresarDatos() {
	printf("\nIngrese numero de recursos: ");
	scanf("%d", &r);

	printf("\nIngrese numero de procesos: ");
	scanf("%d", &p);
	for (i=0; i < p; i++) {
		corriendo[i] = 1;
		count++;
	}

	// printf("\nIngrese vector de recursos maximos: ");
	// for (i=0; i < r; i++)
	// 	scanf("%d", &recursos[i]);

	// printf("\nIngrese matriz de asignacion: ");
	// for (i=0; i < p; i++) {
	// 	for (j=0; j < r; j++)
	// 		scanf("%d", &asignacion[i][j]);
	// }

	// printf("\nIngrese matriz de necesidad: ");
	// for (i=0; i < p; i++ ) {
	// 	for (j=0; j < r; j++)
	// 		scanf("%d", &necesidad[i][j]);
	// 	printf("\n");
	// }

	//calculo disponible
	for(i=0; i < p ; i++)
		for(j=0; j < r; j++) {
			asignados[j] += asignacion[i][j];
			disponible[j] = recursos[j] - asignados[j];
		}
}

void mostrarDatos() {

	printf("\nMatriz de necesidad:\n");
	for (i=0; i < p; i++) {
		for (j=0; j < r; j++) 
			printf("\t%d", necesidad[i][j]);
		printf("\n");
	}

	printf("\nMatriz de asignacion:\n");
	for (i=0; i < p; i++) {
		for (j=0; j < r; j++)
			printf("\t%d", asignacion[i][j]);
		printf("\n");
	}

	printf("\nMatriz de N-A:\n");
		for (int i = 0; i < p; ++i) {
			for(j=0; j < r; j++) 
				printf("\t%d", na[i][j]);
			printf("\n");
		}
	
	printf("\nRecursos maximos: ");
	for (i=0; i < r; i++)
		printf("%d ", recursos[i]);

	// printf("\nRecursos asignados: ");
	// for (i=0; i < r; i++) {
	// 	printf("%d ", asignados[i]);
	// }

	printf("\nRecursos disponibles: ");
	for(i=0; i < r; i++)
		printf("%d ", disponible[i]);
	printf("\n");
}


