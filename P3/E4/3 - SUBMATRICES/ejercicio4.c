
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MASTER 0

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

/**
 * @brief Set value a position (posM, posN) in a matrix stored in a vector by rows
 * 
 * @param posM row position
 * @param posN col position
 * @param sizeN col size of matrix
 * @param matrix matrix pointer
 * @param value double value to set
 */
void matrixSetRow(int posM, int posN, int sizeN, double *matrix, double value)
{
    *(matrix + posM * sizeN + posN) = value;
}

void matrixSetCol(int posM, int posN, int sizeM, double *matrix, double value)
{
    *(matrix + posN * sizeM + posM) = value;
}

/**
 * @brief Get value from matrix stored in vector by rows
 * 
 * @param posM row position
 * @param posN col position
 * @param sizeN col size of matrix
 * @param matrix matrix pointer
 * @return double value at position (posM, posN)
 */
double matrixGetRow(int posM, int posN, int sizeN, const double *matrix)
{
    double value = *(matrix + posM * sizeN + posN);
    return value;
}

double matrixGetCol(int posM, int posN, int sizeM, const double *matrix)
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

void printMatrixCol(double *mat, int sizeM, int sizeN)
{
    for (int i = 0; i < sizeM; i++)
    {
        for (int j = 0; j < sizeN; j++)
        {
            printf("%.2f ", matrixGetCol(i, j, sizeM, mat));
        }
        printf("\n");
    }
}

void printMatrixRow(double *mat, int sizeM, int sizeN)
{
    for (int i = 0; i < sizeM; i++)
    {
        for (int j = 0; j < sizeN; j++)
        {
            printf("%.2f ", matrixGetRow(i, j, sizeN, mat));
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

    //-----------------------------------------------------------------
    //dimensiones de matriz y submatriz
    int sizeM = 0, //fila
        sizeN = 0, //columna
                   //submatriz
        subM = 0,
        subN = 0;
    const int header_size = 3; //dimensiones de la cabecera de la submatriz
    //-----------------------------------------------------------------

    if (world_size < 2 || argc != 5)
    {
        printf("Llamada: mpirun -n n_proc prog matrixRows matrixCols subMatRows subMatCols \nDonde n>=3, m > n\n");
    }
    else
    {
        double globalResult = 0.0;         //resultado global
        const int n_proc = world_size - 1; //restamos procesador 0 de procesadores disponibles
        sizeM = atoi(argv[1]);
        sizeN = atoi(argv[2]);
        subM = atoi(argv[3]);
        subN = atoi(argv[4]);
        const int matSize = sizeM * sizeN;
        const int subSize = subM * subN;

        if (sizeM >= world_size && sizeN >= world_size)
        {
            printf("Mi rank: %d\n", rank);
            if (rank == MASTER)
            {
                double *matriz = crearMatriz(sizeM, sizeN); //matriz aleatoria no es mas que un vector
                double tmpResult = 0.0;
                //--------------------------------------------------------------------------------------
                int *subMatHeader = (int *)malloc(sizeof(int) * header_size);   //cabecera de la matriz con informacion de la dimension de la misma
                double *subMatrix = (double *)malloc(sizeof(double) * subSize); //espacio para submatriz
                int nRowChunks = ceilInt(sizeM, subM) + 1;                      //tamaño del vector que contiene posiciones de filas de submatriz dentro de matriz
                int nColChunks = ceilInt(sizeN, subN) + 1;                      //tamaño del vector que contiene posiciones de columnas de submatriz dentro de matriz
                int *matRowChunks = (int *)calloc(sizeof(int), nRowChunks);     //vector que contiene posiciones de filas de submatriz dentro de matriz
                int *matColChunks = (int *)calloc(sizeof(int), nColChunks);     //vector que contiene posiciones de filas de submatriz dentro de matriz
                calculateChunks(subM, sizeM, &matRowChunks);                    //calcular vector filas
                calculateChunks(subN, sizeN, &matColChunks);                    //calcular vector columnas
                //--------------------------------------------------------------------------------------
                int dest; //procesador destino
                int tag;  //tag para tarea a enviar

                int iniM, iniN, finM, finN;
                int itSubmatrix,                     //iterador de posiciones de submatriz
                    countSubMatrix = 0,              //contador de submatrices
                    subMatLength;                    //length of current submatrix
                for (int i = 0; i < nRowChunks; i++) //
                {
                    iniM = matRowChunks[i];
                    if (i + 1 < nRowChunks)
                    {
                        finM = matRowChunks[i + 1];
                        for (int j = 0; j < nColChunks; j++)
                        {
                            iniN = matColChunks[j];
                            if (j + 1 < nColChunks)
                            {
                                finN = matColChunks[j + 1];
                                //------------------------------------------------------
                                //zona para recorrer y crear submatriz para enviar
                                itSubmatrix = 0; //iterador de posiciones submatriz
                                for (int row = iniM; row < finM; row++)
                                {
                                    for (int col = iniN; col < finN; col++)
                                    {
                                        double val = matrixGetRow(row, col, sizeN, matriz);
                                        subMatrix[itSubmatrix] = val;
                                        itSubmatrix++;
                                    }
                                }
                                subMatHeader[0] = finM - iniM;    //numero de filas de la submatriz
                                subMatHeader[1] = finN - iniN;    //numero de columnas de la submatriz
                                subMatHeader[2] = countSubMatrix; //contador de submatriz
                                //matriz creada
                                //------------------------------------------------------
                                //matriz se puede enviar
                                //1. se envian dimensiones de la matriz
                                dest = (countSubMatrix % n_proc) + 1; //procesador 0 no se utiliza
                                tag = dest;
                                MPI_Send(subMatHeader, header_size, MPI_INT, dest, tag, MPI_COMM_WORLD);
                                //------------------------------------------------------
                                //2. se envia la submatriz para procesarla
                                subMatLength = subMatHeader[0] * subMatHeader[1];
                                MPI_Send(subMatrix, subMatLength, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
                                //------------------------------------------------------
                                //3. Recibir respuesta
                                MPI_Recv(&tmpResult, 1, MPI_DOUBLE, dest, dest, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                                globalResult += tmpResult;
                                //------------------------------------------------------
                                //4. aumentar contador de matrices
                                countSubMatrix++;
                                //------------------------------------------------------
                            }
                        }
                    }
                }

                //-------------------------------------------------------------------------------------------
                //desactivar los procesos
                subMatHeader[0] = -1; //numero de filas de la submatriz
                subMatHeader[1] = -1; //numero de columnas de la submatriz
                subMatHeader[2] = -1; //numero de columnas de la submatriz
                for (int i = 1; i < world_size; i++)
                {
                    MPI_Send(subMatHeader, header_size, MPI_INT, i, i, MPI_COMM_WORLD);
                }
                //-------------------------------------------------------------------------------------------
                printf("\n----------------------------------------------------------------------------\n");
                printf("RESULTADO GLOBAL: %2f\n", globalResult);
                printf("RESULTADO SECUENCIAL: %2f\n", seqCheck(matriz, matSize));
                printf("----------------------------------------------------------------------------\n");
                free(matriz);
                free(subMatrix);
                free(matRowChunks);
                free(matColChunks);
            }
            else
            {
                int subMatCount = 0; //mantener procesos en la escucha
                int from = MASTER;
                int tag = rank;
                int dst = -1;
                int *buf_header = (int *)malloc(sizeof(int) * header_size);
                double *buf = (double *)malloc(sizeof(double) * subM * subN); //reservamos maxima dimension para submatriz
                int matLength = 0;
                double sumResult = 0.0;
                while (subMatCount != -1)
                {
                    MPI_Recv(buf_header, header_size, MPI_INT, MASTER, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    subMatCount = buf_header[2];                   
                    if (subMatCount != -1)
                    {
                        dst = (subMatCount % n_proc) + 1;
                        matLength = buf_header[0] * buf_header[1];
                        MPI_Recv(buf, matLength, MPI_DOUBLE, MASTER, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        sumResult = seqCheck(buf, matLength);
                        MPI_Send(&sumResult, 1, MPI_DOUBLE, MASTER, rank, MPI_COMM_WORLD);
                    }
                }
                free(buf_header);
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