#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MASTER 0
#define SLAVE 1

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


int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);
    int world_size, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    if (world_size != 2) {
    	printf("Llamada: mpirun -n 2 prog\n");
  	}else{
        int length = 10;
        if (rank == MASTER) {
            double *vec = crearVectorAleatorio(length);
            printf("\n-----------------ORIGINAL---------------------\n");
            for(int i = 0; i < length; i++){
                printf("%.2f\t", vec[i]);
            }

            int dest;//procesador destino
            int tag;//tag para tarea a enviar
            //send tasks to processors
            for(int i = 0; i < length; i++){
                dest = SLAVE;
                tag = i;
                //printf("SEND->FROM: %d, TO: %d, TAG: %d, SIZE: %d\n", MASTER, dest, tag, longitud);
                MPI_Send(&(vec[i]), 1, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
                //printf("SEND->%d: %.2f\n", i, vec[i]);

            }
            MPI_Recv(vec, length, MPI_DOUBLE, SLAVE, SLAVE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            printf("\n-----------------CUADRADO---------------------\n");
            for(int i = 0; i < length; i++){
                printf("%.2f\t", vec[i]);
            }
            printf("\n----------------------------------------------\n");
            free(vec);
        }else{
            int from = MASTER;
            int tag = rank;
            double *vec = (double*)malloc(sizeof(double)*10);
            for(int i = 0; i < length; i++){
                MPI_Recv(&vec[i], 1, MPI_DOUBLE, MASTER, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //printf("RECV->%d: %.2f\n", i, vec[i]);
                //calcular cuadrado
                vec[i] *= vec[i];
            }
            MPI_Send(vec, length, MPI_DOUBLE, MASTER, SLAVE, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}