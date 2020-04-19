#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>

bool DEBUG = false;

int sizeM = 0,
	sizeN = 0;

int maskM = 3,
	maskN = 3;

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
	double *matrix = (double *)calloc(sizeM * sizeN ,sizeof(double));
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
	int *filas; //fila%2==0 - ini;fila%2==1 - fin
	int *columnas; //columnas%2==0 - ini;columnas%2==1 - fin
	int* size;
} tarea;

void *sumCols(void *args)
{
	
	tarea *deberes = (tarea *)args;
	double *resultadoLocal = (double *)calloc(1,sizeof(double));
	double val = 0;
	if(DEBUG) printf("suma llamada, size = %d\n", *deberes->size);
	for(int i = 0; i < *deberes->size; i+=1){
		for(int iFila = *(deberes->filas + i); iFila < *(deberes->filas + i+1); iFila++){
			if(DEBUG) printf("iFila %d", iFila);
			for(int iCol = deberes->columnas[i]; iCol < deberes->columnas[i+1]; iCol++){
				if(DEBUG) printf("iFila %d, iCol %d\n", iFila, iCol);
				val = matrixGet(iFila, iCol, deberes->matrix);
				*resultadoLocal+=val;
				if(DEBUG) printf("%.2f ", val);
			}
			if(DEBUG) printf("\n");
		}
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
	if (argc != 6 && argc != 4)
	{
		printf("Llamada 1: ./prog nHilos matrixRows matrixCols\n");
		printf("Llamada 2: ./prog nHilos maskRows maskCols matrixRows matrixCols\n");
	}
	else
	{
		int nHilos = atoi(argv[1]);
		if(argc == 4){
			sizeM = atoi(argv[2]); //filas
			sizeN = atoi(argv[3]); //columnas
		}else{
			maskM = atoi(argv[2]); //filas mascara
			maskN = atoi(argv[3]); //columnas mascara
			sizeM = atoi(argv[4]); //filas
			sizeN = atoi(argv[5]); //columnas
		}
		//comprobar que el tamaño de mascara es adecuado
		if(maskM > sizeM ||maskN > sizeN){
			printf("tamaño de submatriz no puede ser inferior a la matriz");
			return 0;
		}

		printf("Matrix: sizeM=%d, sizeN=%d\n", sizeM, sizeN);
		printf("Mask: maskM=%d, maskN=%d\n", maskM, maskN);

		double *matrix = crearMatrizAleatoria(sizeM, sizeN);
		tarea *deberes = malloc(sizeof(tarea) * nHilos);
		pthread_t *hilos = malloc(sizeof(pthread_t) * nHilos);

		if (DEBUG) printMatrix(matrix);

		int filaFin = 0, //posicion final de la fila
			colFin = 0; //posicion final de la columna
		int it = 0,//iterador de submatrices
			tareaN = 0;//numero de tarea/deber ya que es lo mismo

		//calcular numero de entradas de filas y columnas que habrá
		int nFilas = sizeM/maskM;
		if (sizeM%maskM > 0) nFilas+=1;
		int nCols = sizeN/maskN;
		if (sizeN%maskN > 0) nCols = nCols+1;
		int nTareas = (nCols*nFilas)/nHilos;
		if((nCols*nFilas)%nHilos > 0) nTareas+=1;
		if (nTareas == 0) nTareas+=1;
		if (DEBUG) printf("Struct: filas=%d, cols=%d, tareas=%d\n", nFilas, nCols, nTareas);

		int size = 0;
		for (int i = 0; filaFin < sizeM; i+=maskM){//recorrer filas con saltos de mascara
			colFin = 0;
			filaFin = i + maskM;
			if (filaFin > sizeM) filaFin = sizeM;

			for (int j = 0; colFin < sizeN; j+=maskN){//recorrer columnas con saltos de mascara
				colFin += maskN;
				if (colFin > sizeN)	colFin = sizeN;
				tareaN = it % nHilos;
				//inicializar si fuera necesario elementos del struct
				if (it < nHilos){//no queremos inicializar para fila > 0
					if (DEBUG) printf("--init tarea: %d\n", tareaN);
					deberes[tareaN].matrix = matrix; //no importa reasignar la matriz, pero lo podemos evitar
					deberes[tareaN].filas = (int*)calloc(nTareas*2, sizeof(int));
					deberes[tareaN].columnas = (int*)calloc(nTareas*2, sizeof(int));
					deberes[tareaN].size = calloc(1, sizeof(int));
				}
				size = *deberes[tareaN].size;//size es mas versatil que *deberes[tareaN].size
				if (DEBUG) printf("i = %d, j = %d, it = %d, tarea - %d\n", i, j, it, tareaN);
				if (DEBUG) printf("tarea: %d, fila: %d - %d, col: %d - %d, size: %d\n", tareaN, i, filaFin, j, colFin, size);
				//asignar fila inicio y fin
				deberes[tareaN].filas[size] = i;//inicio fila
				deberes[tareaN].filas[size+1] = filaFin;//fin fila
				//asignar columna inicio y fin
				deberes[tareaN].columnas[size] = j;//inicio columna
				deberes[tareaN].columnas[size+1] = colFin;//fin columna
				//aumentar y asignar size
				size+=2;
				*deberes[tareaN].size = size;
				it++;
			}
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
