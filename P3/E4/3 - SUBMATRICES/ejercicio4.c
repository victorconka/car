
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MASTER 0

//matriz
int sizeM = 0, //fila
    sizeN = 0, //columna
//submatriz
    subM = 0, 
    subN = 0;

double seqCheck(const double *matrix, int longitud)
{
    double res = 0;
    for (int i = 0; i < longitud; i++)
    {
        res += matrix[i];
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

    for (int i = 0; i < sizeM * sizeN; i++)
    {
        matrix[i] = myrand();
    }

    return matrix;
}

void printMatrixCol(double *mat)
{
    for (int i = 0; i < sizeM; i++)
    {
        for (int j = 0; j < sizeN; j++)
        {
            printf("%.2f ", matrixGetCol(i, j, mat));
        }
        printf("\n");
    }
}
void printMatrixRow(double *mat)
{
    for (int i = 0; i < sizeM; i++)
    {
        for (int j = 0; j < sizeN; j++)
        {
            printf("%.2f ", matrixGetRow(i, j, mat));
        }
        printf("\n");
    }
}

void receive_array(int from, int tag, double **buf, int **buf_len)
{
    MPI_Status status;
    MPI_Probe(from, tag, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_DOUBLE, &**buf_len);
    *buf = (double *)realloc(*buf, **buf_len * sizeof(double));
    MPI_Recv(&**buf, **buf_len, MPI_DOUBLE, from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

/**
 * Calculate ceil function for the division of two integers 
 * ceil(x/y)
 */
int ceilInt(int x, int y)
{
    if (x > 1 && y >= 1)
    {
        int q = 1 + ((x - 1) / y);
        return q;
    }
    return -1;
}

/**
 * Calculate chunks of size "val" within "maxVal" and add values to **vec
 * for instance maxVal=13 and val=4, *vec={0,4,8,12,13}
 */
void calculateChunks(int val, int maxVal, int **vec)
{
    if (val > 0 && maxVal > val)
    {
        int mIt = 0;
        int i = 0;
        while (mIt <= maxVal)
        {
            vec[0][i] = mIt;
            //--------------------------------
            mIt += val;
            i += 1;
            if (mIt > maxVal && mIt != maxVal + val)
                mIt = maxVal;
        }
    }
}

/**
 * Version por filas y por columnas son identicas, la diferencia se encuentra en la manera
 * de interpretar la matriz. Fijarse en 2 grupos de funciones de lectura y escritura de posiciones
 * por filas y por columnas.
 * Como ejemplo, esta la impresion de la matriz por filas y por columnas al final de estos ejercicios
*/
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int world_size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2 || argc != 5)
    {
        printf("Llamada: mpirun -n n_proc prog matrixRows matrixCols subMatRows subMatCols \nDonde n>=3, m > n\n");
    }
    else
    {
        int n_proc = world_size - 1;
        sizeM = atoi(argv[1]);
        sizeN = atoi(argv[2]);
        subM = atoi(argv[1]);
        subN = atoi(argv[2]);
        int matSize = sizeM * sizeN;
        int subSize = subM * subN;

        if (sizeM >= world_size && sizeN >= world_size)
        {
            printf("Mi rank: %d\n", rank);
            if (rank == MASTER)
            {
                double *matriz = crearMatriz(sizeM, sizeN); //matriz aleatoria no es mas que un vector
                //--------------------------------------------------------------------------------------
                double *subMatrix = (double*)malloc(sizeof(double)*subSize);//espacio para submatriz
                int nRowChunks = ceilInt(sizeM, subM)+1; //tamaño del vector que contiene posiciones de filas de submatriz dentro de matriz
                int nColChunks = ceilInt(sizeN, subN)+1; //tamaño del vector que contiene posiciones de columnas de submatriz dentro de matriz
                int *matRowChunks = (int*)calloc(sizeof(int), nRowChunks); //vector que contiene posiciones de filas de submatriz dentro de matriz
                int *matColChunks = (int*)calloc(sizeof(int), nColChunks); //vector que contiene posiciones de filas de submatriz dentro de matriz
                calculateChunks(subM, sizeM, &matRowChunks);//calcular vector filas
                calculateChunks(subN, sizeN, &matColChunks);//calcular vector columnas
                //--------------------------------------------------------------------------------------
                int dest;   //procesador destino
                int tag;    //tag para tarea a enviar
                
                for (int i = 1; i < nRowChunks; i++)//
                {
                    for(int j = 1; j < nColChunks; j++){

                    }
                    
                }
                /*
                //receive tasks from processor
                double resultado_final = 0.0;
                double res = 0.0;
                int source; //procesador origen
                for (int i = 1; i < world_size; i++)
                {
                    source = i;
                    tag = i;
                    MPI_Recv(&res, 1, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    resultado_final += res;
                }
                printf("\n--------------------------------------\n");
                printf("resultado paralelo: %f\n", resultado_final);
                printf("resultado secuencial: %f\n", seqCheck(matriz, matSize));
                printf("--------------------------------------\n");
                if (matSize < 100)
                {
                    printMatrixRow(matriz);
                }
                */
                free(matriz);
            }
            else
            {
                int from = MASTER;
                int tag = rank;
                int *buf_len = (int *)calloc(sizeof(int), 1);
                double *buf = (double *)malloc(sizeof(double));
                receive_array(from, tag, &buf, &buf_len);
                printf("RECV->FROM: %d, TO: %d, TAG: %d, SIZE: %d\n", MASTER, rank, tag, *buf_len);
                double res = seqCheck(buf, *buf_len);
                MPI_Send(&res, 1, MPI_DOUBLE, MASTER, tag, MPI_COMM_WORLD);
                free(buf_len);
                free(buf);
            }
        }
        else
        {
            if (rank == MASTER)
                printf("Numero de filas y columnas no puede ser inferior al numero de procesos\n");
        }
    }
    MPI_Finalize();
}