#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <float.h>

#define MASTER 0
#define INFO 2

void receive_array(int from, int tag, int **buf_len, double **buf){
    MPI_Status status;
    MPI_Probe(from, tag, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_DOUBLE, &**buf_len);
    *buf = (double*) realloc(*buf, **buf_len * sizeof(double));
    MPI_Recv(&**buf, **buf_len, MPI_DOUBLE, from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}


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

/**
 * Hacer el reparto de forma equitativa.
 * Primero se reparte la parte entera y posteriormete
 * se reparte el resto empezando por el final
 */
int* calculate_intervals(int longitud, int t_number){
    int size = t_number*2;
    int *intervals = (int*) calloc(sizeof(int), size);
    int pack = longitud / t_number;
	int offset = longitud % t_number;

    for (int n = 0; n < 2; n++){
        if(n == 0){
            for (int i = 0; i < t_number; i++){//nHilos
                intervals[i*2] = i*pack;
                intervals[(i*2)+1] = i*pack + pack;
            }
        }else{
            for(int i = size-1; i > 0; i-=2){//empezar por final
                if(offset > 0){
                    intervals[i] += offset;
                    offset-=1;
                    intervals[i-1] += offset;
                }
            }  
        }
    }

    printf("\nintervals: -----------------\n");
    for(int i = 0; i < size; i++){
        printf("%d\t", intervals[i] );
    }
    printf("\n--------------------------\n");
    return intervals;
}

double seqCheck(const double* vec, int tam){
	double res = DBL_MAX;
	for(int i = 0; i < tam; i++)
		if(res > vec[i]) res = vec[i];
	return res;
}

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);
    int world_size, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    if (world_size < 3 || argc != 2) {
    	printf("Llamada: mpirun -n n prog m\nDonde n>=2, m > n\n");
  	}else{
        int longitud = atoi(argv[1]);
        if (longitud < world_size){
            printf("Llamada: mpirun -n n prog m\nDonde n>=2, m > n\n");
        }else{

            double *vec = crearVectorAleatorio(longitud);
            int it = 0;
            if (rank == MASTER) {
                int *intervals = calculate_intervals(longitud, world_size);
                int length;//longitud de subarray a enviar
                int ini;//posicion inicial del subarray en el array original
                int dest;//procesador destino
                int tag;//tag para tarea a enviar
                //send tasks to processors
                for(int i = 1; i < world_size; i++){//repartir trabajo menos a uno mismo
                    ini = intervals[i*2];
                    length = intervals[(i*2)+1] - ini;
                    dest = i;
                    tag = 111;
                    MPI_Send(&(vec[ini]), length, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
                }
                printf("rank: %d finished\n", rank);
                double res = seqCheck(vec, intervals[1]-intervals[0]);
                MPI_Send(&res, 1, MPI_DOUBLE, INFO, rank, MPI_COMM_WORLD);
                printf("rank: %d sent result\n", rank);
                free(intervals);
            }else if (rank == INFO)
            {
                //receive tasks from processor
                double resultado_final = DBL_MAX;
                double res = DBL_MAX;
                int source;//procesador origen
                int tag;//tag para tarea a enviar
                //--------------------------------------------------------------
                //recibir parte de trabajo
                int *buf_len = (int*)calloc(sizeof(int), 1);
                double *vec1 = (double*)malloc(sizeof(double));
                receive_array(MASTER, 111, &buf_len, &vec1);
                printf("recibido array");
                resultado_final = seqCheck(vec1, *buf_len);
                printf("%d -> %f", rank, resultado_final);
                //--------------------------------------------------------------
                for(int i = 0; i < world_size; i++){
                    if(i != INFO){
                        source = i;
                        tag = i;
                        MPI_Recv(&res, 1, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        if(res < resultado_final)
                            resultado_final = res;
                    }
                }
                printf("\n--------------------------------------\n");
                printf("resultado paralelo: %f\n", resultado_final);
                printf("resultado secuencial: %f\n", seqCheck(vec, longitud));
                printf("--------------------------------------\n");
                free(vec);
            }else{
                int from = MASTER;
                int rcv_tag = 111;
                int *buf_len = (int*)calloc(sizeof(int), 1);
                double *vec = (double*)malloc(sizeof(double));
                receive_array(from, rcv_tag, &buf_len, &vec);
                printf("RECV->FROM: %d, TO: %d, TAG: %d, SIZE: %d\n", MASTER, rank, rcv_tag, *buf_len);
                double res = seqCheck(vec, *buf_len);
                int send_tag = rank;
                MPI_Send(&res, 1, MPI_DOUBLE, INFO, send_tag, MPI_COMM_WORLD);
                free(buf_len);
                free(vec);
            }
        }
    }

    MPI_Finalize();
}