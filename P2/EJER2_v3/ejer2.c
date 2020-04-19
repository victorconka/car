#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

/**
 * Solucion del ejercicio basado en ejemplo de suma de elementos de vector v3 
 * */

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
	double* vec = malloc(sizeof(double)*tam);
	for(int i = 0; i<tam; i++){
        vec[i] = myrand();
    }
	return vec;
}

typedef struct tarea{
	double* vector;	
	int ini;
	int fin;
} tarea;


double seqCheck(const double* vec, int tam){
	double res = DBL_MAX;
	for(int i = 0; i<tam; i++)
		if(res > vec[i]) res = vec[i];
	return res;
}

void* minValue(void* args){
	tarea* deberes = (tarea*) args;
	double* resultadoLocal = malloc(sizeof(double));
	*resultadoLocal = FLT_MAX;
	for(int i = deberes->ini; i<deberes->fin; i++){
        if ((deberes->vector)[i] < *resultadoLocal){
            *resultadoLocal = (deberes->vector)[i];
        }
	}
    return resultadoLocal;
}

int main(int argc, char* argv[]){
	double resultadoGlobal = DBL_MAX;
	double* TMPres = malloc(sizeof(double)); 

	if(argc != 3){
		printf("Llamada: ./prog Longitud nHilos\n");
	}else{
		int longitud = atoi(argv[1]);
		int nHilos = atoi(argv[2]);
		double* vector = crearVectorAleatorio(longitud);

		tarea* deberes = malloc(sizeof(tarea)*nHilos);
		pthread_t* hilos = malloc(sizeof(pthread_t)*nHilos);
		
		int pack = longitud / nHilos;
		int offset = longitud % nHilos;
        int fin = 0;

		for(int i = 0; i<nHilos; i++){
			deberes[i].vector = vector;
			deberes[i].ini = i*pack;
            fin = i*pack + pack;
            if (i == nHilos-1) fin+=offset;
			deberes[i].fin = fin;
			pthread_create(&hilos[i], 0, minValue, &deberes[i]);		
		}

		* TMPres = FLT_MAX;
		for(int i = 0; i<nHilos; i++){
			pthread_join(hilos[i], (void**) &TMPres);
            if (resultadoGlobal > *TMPres) resultadoGlobal = *TMPres;
		}
		printf("Resultado Paralelo: %f\n", resultadoGlobal);		
		printf("Resultado Secuencial: %f\n", seqCheck(vector, longitud));		

		free(hilos);
		free(deberes);
		free(vector);
		
	}
	return 0;
}
