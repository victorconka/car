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

typedef struct tarea{
	double *matrix;
	int *columnas;
	int *size;
} tarea;


void *sumCols(void *args)
{
	tarea *deberes = (tarea *)args;
	double *resultadoLocal = malloc(sizeof(double));
	*resultadoLocal = 0;
	int columna = 0;
	int nColumnas = *deberes->size;
	if (DEBUG) printf("\n");
	double val = 0;
	for (int i = 0; i < nColumnas; i++){//columna fija
		columna = deberes->columnas[i];
		if (DEBUG) printf("Columna: %d, valores: ->", columna);
		for(int j = 0; j < sizeM; j++){	
			val = matrixGet(j, columna, deberes->matrix);
			if (DEBUG) printf("%.2f, ", val);
			*resultadoLocal+=val;
		}
		if (DEBUG) printf("\n");
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

		if (DEBUG) printMatrix(matrix);

		//repartimos columnas entre hilos/tareas
		int columnaCount = 0; //itera columnas de la matriz
		int tareaN = 0; //numero de tarea/deber ya que es lo mismo
		while (columnaCount < sizeN){ //recorrer todas la columnas repartir estas entre tareas
			tareaN = columnaCount % nHilos; //calculamos que columna le tocara a que tarea
			//inicializar columnas y size en la primera iteracion de cada hilo
			if (columnaCount < nHilos){
				deberes[tareaN].matrix = matrix; //no importa reasignar la matriz, pero lo podemos evitar
				deberes[tareaN].size = malloc( sizeof(int) );
				deberes[tareaN].columnas = malloc( sizeof(int) * (sizeN/nHilos) );
			}
			//asignamos la c columnaCount a la tarea tareaN
			deberes[tareaN].columnas[*deberes[tareaN].size] = columnaCount;
			//incrementar numero de columnas
			*deberes[tareaN].size+=1;
			columnaCount++;
		}
		//creamos threads por cada tarea
		for (int i = 0; i<nHilos; i++){
			pthread_create(&hilos[i], 0, sumCols, &deberes[i]);
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
