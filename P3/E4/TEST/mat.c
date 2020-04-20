
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MASTER 0

int sizeM = 0,//fila
	sizeN = 0;//columna
double seqCheck(const double *matrix, int longitud)
{
	double res = 0;
	for (int i = 0; i < longitud; i++){
		res+=matrix[i];
	}
	return res;
}

double seed = 12345.0;
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

void matrixSetRow(int posM, int posN, double *matrix, double value)
{
	*(matrix + posM * sizeN + posN) = value;
}

void matrixSetCol(int posM, int posN, double *matrix, double value)
{
	*(matrix + posN * sizeM + posM) = value;
}

double matrixGetRow(int posM, int posN, const double *matrix)
{
	double value = *(matrix + posM * sizeN + posN);
	return value;
}

double matrixGetCol(int posM, int posN, const double *matrix)
{
	double value = *(matrix + posN * sizeM + posM);
	return value;
}

double *crearMatriz(int sizeM, int sizeN)
{
	double *matrix = calloc(sizeM * sizeN, sizeof(double));

	for (int i = 0; i < sizeM*sizeN; i++)
    {
		matrix[i] = myrand();
	}

	return matrix;
}

void printMatrixCol(double *mat){
    for(int i = 0; i< sizeM; i++){
        for(int j = 0; j < sizeN; j++){
            printf("%.2f ", matrixGetCol(i,j,mat));
        }
        printf("\n");
    }
}
void printMatrixRow(double *mat){
    for(int i = 0; i< sizeM; i++){
        for(int j = 0; j < sizeN; j++){
            printf("%.2f ", matrixGetRow(i,j,mat));
        }
        printf("\n");
    }
}

void receive_array(int from, int tag, double **buf, int **buf_len){
    MPI_Status status;
    MPI_Probe(from, tag, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_DOUBLE, &**buf_len);
    *buf = (double*) realloc(*buf, **buf_len * sizeof(double));
    MPI_Recv(&**buf, **buf_len, MPI_DOUBLE, from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int world_size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    double *vec;

    if (world_size < 2 || argc != 3) {
    	printf("Llamada: mpirun -n n_proc prog matrixRows matrixCols \nDonde n>=3, m > n\n");
  	}else{
        sizeM = atoi(argv[1]);
        sizeN = atoi(argv[2]);
        int longitud = sizeM*sizeN;

        if(sizeM >= world_size && sizeN >= world_size){
            printf("Mi rank: %d\n", rank);
            if(rank==MASTER){
                int n_proc = world_size - 1;
                int *intervals = calculate_intervals(sizeM*sizeN, n_proc);
                double *matriz = crearMatriz(sizeM, sizeN);

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
                    MPI_Send(&(matriz[ini]), length, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
                }


                //receive tasks from processor
                double resultado_final = 0.0;
                double res = 0.0;
                int source;//procesador origen
                for(int i = 1; i < world_size; i++){
                    source = i;
                    tag = i;
                    MPI_Recv(&res, 1, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    resultado_final += res;
                }
                printf("\n--------------------------------------\n");
                printf("resultado paralelo: %f\n", resultado_final);
                printf("resultado secuencial: %f\n", seqCheck(matriz, longitud));
                printf("--------------------------------------\n");
                if(longitud < 100){
                    printMatrixRow(matriz);
                }
                free(intervals);
            }else{
                int from = MASTER;
                int tag = rank;
                int *buf_len = (int*)calloc(sizeof(int), 1);
                double *buf = (double*)malloc(sizeof(double));
                receive_array(from, tag, &buf, &buf_len);
                printf("RECV->FROM: %d, TO: %d, TAG: %d, SIZE: %d\n", MASTER, rank, tag, *buf_len);
                double res = seqCheck(buf, *buf_len);
                MPI_Send(&res, 1, MPI_DOUBLE, MASTER, tag, MPI_COMM_WORLD);
                free(buf_len);
                free(buf);
            }
        }else{
            if(rank==MASTER)
                printf("Numero de filas y columnas no puede ser inferior al numero de procesos\n");
        }
    }
    MPI_Finalize();
}