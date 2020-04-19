#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

/**
 * http://www.cs.utsa.edu/~wagner/CS2073/random/random.html
 */
double seed = 11111.0;
double myrand() {
   double a = 16807.0,
          m = 2147483647.0;
   double q;
   seed = a*seed;
   q = floor(seed/m);
   seed = seed - q*m;
   double res = (seed/m);
   return res;
}

double* crearVectorAleatorio(int tam){
	double* vec = (double*)malloc(sizeof(double)*tam);
	for(int i = 0; i<tam; i++){
        vec[i] = myrand();
    }
	return vec;
}

typedef struct tarea{
	double* vector;	
	int* ini;
	int* fin;
	double* resultadoLocal; //out: para almacenar el resultado parcial
} tarea;


double seqCheck(const double* vec, int tam){
	double res = DBL_MAX;
	for(int i = 0; i < tam; i++)
		if(res > vec[i]) res = vec[i];
	return res;
}

/**
 * Busca valor minimo en un *vector
 **/
void* minValue(void* args){
	tarea* deberes = (tarea*) args;
	for(int i = *deberes->ini; i<*deberes->fin; i++){
        if ((deberes->vector)[i] < *deberes->resultadoLocal){
            *deberes->resultadoLocal = (deberes->vector)[i];
        }
	}
    return 0;
}

int main(int argc, char* argv[]){
	double resultadoGlobal = DBL_MAX;
	if(argc != 3){
		printf("Llamada: ./prog nHilos Longitud\n");
	}else{
		int longitud = atoi(argv[2]);
		int nHilos = atoi(argv[1]);
		double* vector = crearVectorAleatorio(longitud);

		/*
		for (int i = 0 ; i < longitud; i++){
			printf("%.2f ", vector[i]);
		}
		*/

		tarea* deberes = malloc(sizeof(tarea)*nHilos);
		pthread_t* hilos = malloc(sizeof(pthread_t)*nHilos);
		
		int pack = longitud / nHilos;
		int offset = longitud % nHilos;
        int fin = 0;
		//repartir cachos del vector entre tareas/hilos
		for(int i = 0; i<nHilos; i++){
			if(i < nHilos){
				deberes[i].ini = (int*)malloc(sizeof(int));
				deberes[i].fin = (int*)malloc(sizeof(int));
				deberes[i].resultadoLocal = (double*)malloc(sizeof(double));
			}
			deberes[i].vector = vector;
			*deberes[i].ini = i*pack;
            fin = i*pack + pack;
            if (i == nHilos-1) fin+=offset;
			*deberes[i].fin = fin;
			*deberes[i].resultadoLocal = DBL_MAX;
			pthread_create(&hilos[i], 0, minValue, &deberes[i]);		
		}
		for(int i = 0; i<nHilos; i++){
			pthread_join(hilos[i], 0);
            if (resultadoGlobal > *deberes[i].resultadoLocal){
			    resultadoGlobal = *deberes[i].resultadoLocal;
            }
		}
		printf("Resultado Paralelo: %f\n", resultadoGlobal);		
		printf("Resultado Secuencial: %f\n", seqCheck(vector, longitud));		

		free(hilos);
		free(deberes);
		free(vector);
		
	}
	return 0;
}
