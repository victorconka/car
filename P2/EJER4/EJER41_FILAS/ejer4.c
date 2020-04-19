#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>

bool DEBUG = false;

int sizeM = 0,
	sizeN = 0;

/**
 * http://www.cs.utsa.edu/~wagner/CS2073/random/random.html
 */
double seed = 11111.0;
double myrand()
{
	double a = 16807.0,
		   m = 2147483647.0;
	double q;
	seed = a * seed;
	q = floor(seed / m);
	seed = seed - q * m;
	double res = (seed / m);
	return res;
}

void matrixSet(int posM, int posN, double *matrix, double value)
{
	*(matrix + posM * sizeN + posN) = value;
}

double matrixGet(int posM, int posN, const double *matrix)
{
	double value = *(matrix + posM * sizeN + posN);
	return value;
}

double *crearMatrizAleatoria(int sizeM, int sizeN)
{
	double *matrix = calloc(sizeM * sizeN ,sizeof(double));
	for (int i = 0; i < sizeM; i++)
	{
		for (int j = 0; j < sizeN; j++)
		{
			matrixSet(i, j, matrix, myrand());
		}
	}
	return matrix;
}

typedef struct tarea{
	double *matrix;
	int *filas;
	int *size;
} tarea;

double seqCheck(const double *matrix)
{
	double res = 0;
	double val = 0;
	for (int i = 0; i < sizeM*sizeN; i++){
		if(DEBUG) 
			if ( i%sizeN == 0) printf("\n");
		val = *(matrix + i);
		res+=val;
		if(DEBUG) printf("%.2f\t", val);
		
	}
	return res;
}

void *sumRows(void *args)
{
	tarea *deberes = (tarea *)args;
	double *resultadoLocal = malloc(sizeof(double));
	*resultadoLocal = 0;
	int fila = 0;
	int nFilas = *deberes->size;
	if(DEBUG) printf("\n");
	double val = 0;
	for (int i = 0; i < nFilas; i++){
		fila = deberes->filas[i];
		if(DEBUG) printf("Fila: %d, valores: ->",fila);
		for(int j = 0; j < sizeN; j++){	
			val = matrixGet(fila, j, deberes->matrix);
			if(DEBUG) printf("%.2f, ", val);
			*resultadoLocal+=val;
		}
		if(DEBUG) printf("\n");
	}
	return resultadoLocal;
}

void printMatrix(double *matrix){
	for (int i = 0; i < sizeM; i++){
		for(int j = 0; j < sizeN; j++)
			printf("%.2f\t", matrixGet(i,j,matrix));
		printf("\n");
	}
	
}

int main(int argc, char *argv[])
{
	double resultadoGlobal = 0;
	double *TMPres = malloc(sizeof(double));

	if (argc != 4)
	{
		printf("Llamada: ./prog nHilos matrixRows matrixCols\n");
	}
	else
	{
		int nHilos = atoi(argv[1]);
		sizeM = atoi(argv[2]); //filas
		sizeN = atoi(argv[3]); //columnas

		printf("Matrix: sizeM=%d, sizeN=%d\n", sizeM, sizeN);

		double *matrix = crearMatrizAleatoria(sizeM, sizeN);
		tarea *deberes = malloc(sizeof(tarea) * nHilos);
		pthread_t *hilos = malloc(sizeof(pthread_t) * nHilos);

		if(DEBUG) printMatrix(matrix);

		//repartimos filas entre hilos/tareas
		int filaCount = 0; //itera filas de la matriz
		int tareaN = 0; //numero de tarea/deber ya que es lo mismo
		while (filaCount < sizeM){ //recorrer todas la filas repartir estas entre tareas
			tareaN = filaCount % nHilos; //calculamos que fila le tocara a que tarea
			//inicializar filas y size en la primera iteracion de cada hilo
			if (filaCount < nHilos){
				deberes[tareaN].matrix = matrix; //no importa reasignar la matriz, pero lo podemos evitar
				deberes[tareaN].size = (int *)calloc( 1, sizeof(int) );
				deberes[tareaN].filas = malloc( sizeof(int) * (sizeM/nHilos) );
			}
			//asignamos la fila filaCount a la tarea tareaN
			deberes[tareaN].filas[*deberes[tareaN].size] = filaCount;
			//incrementar numero de filas
			*deberes[tareaN].size+=1;
			filaCount++;
		}
		//creamos threads por cada tarea
		for (int i = 0; i<nHilos; i++){
			pthread_create(&hilos[i], 0, sumRows, &deberes[i]);
		}
		//calculamos resultado pasando la el puntero TMPres
		* TMPres = 0;
		for(int i = 0; i<nHilos; i++){
			pthread_join(hilos[i], (void**) &TMPres);
            resultadoGlobal += *TMPres;
		}
		printf("Resultado Paralelo: %f\n", resultadoGlobal);		
		printf("Resultado Secuencial: %f\n", seqCheck(matrix));

		free(TMPres);
		free(hilos);
		free(deberes);
		free(matrix);
	}
	return 0;
}
