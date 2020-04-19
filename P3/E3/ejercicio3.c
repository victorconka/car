#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MASTER 0

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
        if(n == 0){//iteracion 0 reparte la parte entera de la division (pack)
            for (int i = 0; i < t_number; i++){//nHilos
                intervals[i*2] = i*pack;
                intervals[(i*2)+1] = i*pack + pack;
            }
        }else{
            for(int i = size-1; i > 0; i-=2){//reparte el offset
                if(offset > 0){
                    intervals[i] += offset;//desplazar al final el ultimo intervalo
                    offset-=1;//disminuir offset. 
                    intervals[i-1] += offset;//siguiente offset es mas pequeño
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

double seqCheck(const double* vec1, const double* vec2, int tam){
	double res = 0.0;
	for(int i = 0; i<tam; i++){
        res += (vec1[i]*vec2[i]);
    }
	return res;
}

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);
    int world_size, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    if (world_size < 2 || argc != 2) {
    	printf("Llamada: mpirun -n n prog m\nDonde n>=2, m > n\n");
  	}else{
        int longitud = atoi(argv[1]);
        if (longitud < world_size){
            printf("Llamada: mpirun -n n prog m\nDonde n>=2, m > n\n");
        }else{
            if (rank == MASTER) {
                int n_proc = world_size - 1;
                int *intervals = calculate_intervals(longitud, n_proc);
                double *vec1 = crearVectorAleatorio(longitud);
                double *vec2 = crearVectorAleatorio(longitud);
            
                int length;//longitud de subarray a enviar
                int ini;//posicion inicial del subarray en el array original
                int dest;//procesador destino
                int tag;//tag para tarea a enviar
                //send tasks to processors
                for(int i = 0; i < n_proc; i++){
                    ini = intervals[i*2];
                    length = intervals[(i*2)+1] - ini;
                    dest = i+1;
                    tag = i+1;
                    //printf("SEND->FROM: %d, TO: %d, TAG: %d, SIZE: %d\n", MASTER, dest, tag, longitud);
                    MPI_Send(&(vec1[ini]), length, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
                    MPI_Send(&(vec2[ini]), length, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
                }

                //receive tasks from processor
                double resultado_final = 0.0;
                double res = 0.0;
                int source;//procesador origen
                //MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for(int i = 1; i < world_size; i++){
                    source = i;
                    tag = i;
                    MPI_Recv(&res, 1, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    resultado_final += res;
                }
                printf("\n--------------------------------------\n");
                printf("resultado paralelo: %f\n", resultado_final);
                printf("resultado secuencial: %f\n", seqCheck(vec1, vec2, longitud));
                printf("--------------------------------------\n");
                free(vec1);
                free(vec2);
                free(intervals);
            }else{
                int from = MASTER;
                int tag = rank;
                int *buf_len = (int*)calloc(sizeof(int), 1);
                double *ingresos = (double*)malloc(sizeof(double));
                double *gastos = (double*)malloc(sizeof(double));
                receive_array(from, tag, &buf_len, &ingresos);
                receive_array(from, tag, &buf_len, &gastos);
                printf("RECV->FROM: %d, TO: %d, TAG: %d, SIZE: %d\n", MASTER, rank, tag, *buf_len);
                double res = seqCheck(ingresos, gastos, *buf_len);
                MPI_Send(&res, 1, MPI_DOUBLE, MASTER, tag, MPI_COMM_WORLD);
                free(buf_len);
                free(ingresos);
                free(gastos);
            }
        }
    }

    MPI_Finalize();
}