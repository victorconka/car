#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

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
	double* vector1;
	double* vector2;	
	int ini;
	int fin;
} tarea;


double seqCheck(const double* vec1, const double* vec2, int tam){
	double res = (double)0;
	for(int i = 0; i<tam; i++)
		res += vec1[i] * vec2[i];
	return res;
}

double resultadoGlobal = 0;
pthread_mutex_t mutex;
void* scalarProduct(void* args){
	tarea* deberes = (tarea*) args;
	for(int i = deberes->ini; i<deberes->fin; i++){
        resultadoGlobal += (deberes->vector1)[i] * (deberes->vector2)[i];
	}
    return 0;
}

int main(int argc, char* argv[]){
	if(argc != 3){
		printf("Llamada: ./prog Longitud nHilos\n");
	}else{
		int longitud = atoi(argv[1]);
		int nHilos = atoi(argv[2]);
		double* vector1 = crearVectorAleatorio(longitud);
		double* vector2 = crearVectorAleatorio(longitud);

		tarea* deberes = malloc(sizeof(tarea)*nHilos);
		pthread_t* hilos = malloc(sizeof(pthread_t)*nHilos);
		pthread_mutex_init(&mutex, 0);

		int pack = longitud / nHilos;
		int offset = longitud % nHilos;
        int fin = 0;

		for(int i = 0; i<nHilos; i++){
			deberes[i].vector1 = vector1;
			deberes[i].vector2 = vector2;
			deberes[i].ini = i*pack;
            fin = i*pack + pack;
            if (i == nHilos-1) fin+=offset;
			deberes[i].fin = fin;
			pthread_create(&hilos[i], 0, scalarProduct, &deberes[i]);		
		}

		for(int i = 0; i<nHilos; i++){
			pthread_join(hilos[i], 0);
		}
		printf("Resultado Paralelo: %f\n", resultadoGlobal);		
		printf("Resultado Secuencial: %f\n", seqCheck(vector1, vector2, longitud));		

		free(hilos);
		free(deberes);
		free(vector1);
		free(vector2);
		pthread_mutex_destroy(&mutex);
		
	}
	return 0;
}
