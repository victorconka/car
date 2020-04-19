#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

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
	double resultadoLocal; //out: para almacenar el resultado parcial
} tarea;


/**
 * vec1 - ingresos
 * vec2 - gastos
 */
double seqCheck(const double* vec1, const double* vec2, int tam){
    //ingresos - gastos
	double res1 = 0.0;
    double res2 = 0.0;
	for(int i = 0; i<tam; i++){
        res1 += vec1[i];
        res2 += vec2[i];
    }
    double res = res1 - res2;
	return res;
}

void* sumaPar(void* args){
	tarea* deberes = (tarea*) args;
	for(int i = deberes->ini; i<deberes->fin; i++){
		deberes->resultadoLocal += (deberes->vector)[i];
	}
    return 0;
}

int main(int argc, char* argv[]){
	double resultadoGlobal = 0.0;
    double suma1 = 0.0;
    double suma2 = 0.0;
	if(argc != 2){
		printf("Llamada: ./prog Longitud \n");
	}else{
		int longitud = atoi(argv[1]);
		int nHilos = 2;
		
        double* vector;
        int num_vectores = 2; //vector de gastos y de ingresos
        double* vectores[num_vectores];
        double* ingresos = crearVectorAleatorio(longitud);
        double* gastos = crearVectorAleatorio(longitud);
        vectores[0] = ingresos;
        vectores[1] = gastos;
		
        int pack = longitud / nHilos; //ambos vectores tienen la misma logitud
		int offset = longitud % nHilos;

        
		int fin = 0;
        for (int v = 0; v < num_vectores; v++){
            vector = vectores[v];
            pthread_t* hilos = malloc(sizeof(pthread_t)*nHilos);
            tarea* deberes = malloc(sizeof(tarea)*nHilos);
            for(int i = 0; i < nHilos; i++){
                deberes[i].vector = vector;
                deberes[i].ini = i*pack;
                fin = i*pack + pack;
                if (i == nHilos-1) fin+=offset;
                deberes[i].fin = fin;
                deberes[i].resultadoLocal = 0.0;
                pthread_create(&hilos[i], 0, sumaPar, &deberes[i]);		
            }
            for(int i = 0; i<nHilos; i++){
                pthread_join(hilos[i], 0);
                if(v==0) suma1 += deberes[i].resultadoLocal;
                else suma2 += deberes[i].resultadoLocal;
            }
            free(hilos);
		    free(deberes);
        }
        resultadoGlobal = suma1-suma2;
		printf("Resultado Paralelo: %f\n", resultadoGlobal);		
		printf("Resultado Secuencial: %f\n", seqCheck(ingresos, gastos, longitud));		
        
        free(ingresos);
        free(gastos);
	}
	return 0;
}