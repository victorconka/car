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
	double* vector;	
	int ini;
	int fin;
	double resultadoLocal; //out: para almacenar el resultado parcial
} tarea;


double seqCheck(const double* vec1, const double* vec2, int tam){
	double res1 = 0;
    double res2 = 0;
	for(int i = 0; i<tam; i++){
		res1 += vec1[i];
        res2 += vec2[i];
    }
    double res = res1-res2;
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
	if(argc != 3){
		printf("Llamada: ./prog Longitud nHilos\n");
	}else{

		int longitud = atoi(argv[1]);
		int nHilos = atoi(argv[2]);
        double resultadoGlobal = 0;
        double* vector;
        int num_vectores = 2; //vector de gastos y de ingresos
        double* vectores[num_vectores];
        double* ingresos = crearVectorAleatorio(longitud);
        double* gastos = crearVectorAleatorio(longitud);
        vectores[0] = ingresos;
        vectores[1] = gastos;
		
        int pack = longitud / nHilos; //ambos vectores tienen la misma logitud
		int offset = longitud % nHilos;

        int iteraciones = nHilos;

        if (offset == 0) {
            offset = pack;
        }else{
            iteraciones+=1;
        }

        pthread_t* hilos = malloc(sizeof(pthread_t)*iteraciones);
        tarea* deberes = malloc(sizeof(tarea)*iteraciones);
        
		int fin = 0;
        for (int v = 0; v < num_vectores; v++){
            vector = vectores[v];
            for(int i = 0; i < iteraciones; i++){
                deberes[i].vector = vector;
                deberes[i].ini = i*pack;
                fin = i*pack;
                if (i < iteraciones-1) fin+=pack;
                else fin+=offset;             
                deberes[i].fin = fin;
                deberes[i].resultadoLocal = 0;
                pthread_create(&hilos[i], 0, sumaPar, &deberes[i]);		
            }
            for(int i = 0; i<iteraciones; i++){
                pthread_join(hilos[i], 0);
                if(v==0) resultadoGlobal += deberes[i].resultadoLocal;
                else resultadoGlobal -= deberes[i].resultadoLocal;
            }
        }
		printf("Resultado Paralelo: %f\n", resultadoGlobal);		
		printf("Resultado Secuencial: %f\n", seqCheck(ingresos, gastos, longitud));		

		free(hilos);
		free(deberes);
        free(ingresos);
        free(gastos);
		
	}
	return 0;
}