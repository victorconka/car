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

double submatrixGetRow(int posM, int posN, const double *matrix)
{
    double value = *(matrix + posM * subN + posN);
    return value;
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
int main()
{
    printf("Hello World\n");
    sizeM = 13;
    sizeN = 14;
    subM = 3;
    subN = 4;
    int subSize = subM*subN;
    double *matriz = crearMatriz(sizeM, sizeN); //matriz aleatoria no es mas que un vector
    //--------------------------------------------------------------------------------------
    double *subMatrix = (double *)malloc(sizeof(double) * subSize); //espacio para submatriz
    int nRowChunks = ceilInt(sizeM, subM) + 1;                      //tamaño del vector que contiene posiciones de filas de submatriz dentro de matriz
    int nColChunks = ceilInt(sizeN, subN) + 1;                      //tamaño del vector que contiene posiciones de columnas de submatriz dentro de matriz
    int *matRowChunks = (int*)calloc(sizeof(int), nRowChunks);       //vector que contiene posiciones de filas de submatriz dentro de matriz
    int *matColChunks = (int*)calloc(sizeof(int), nColChunks);       //vector que contiene posiciones de filas de submatriz dentro de matriz
    calculateChunks(subM, sizeM, &matRowChunks);                    //calcular vector filas
    calculateChunks(subN, sizeN, &matColChunks);                    //calcular vector columnas
    //--------------------------------------------------------------------------------------
    int dest; //procesador destino
    int tag;  //tag para tarea a enviar
    int iniM, iniN, finM, finN;
    int itSubmatrix, itProc = 1;
    for (int i = 0; i < nRowChunks; i++) //
    {
        iniM = matRowChunks[i];
        if(i+1 < nRowChunks){
            finM = matRowChunks[i+1];
            for (int j = 0; j < nColChunks; j++)
            {
                iniN = matColChunks[j];
                if(j+1 < nColChunks){
                    finN = matColChunks[j+1];
                    //printf("(%d,%d),(%d,%d) ", iniM, finM, iniN, finN);
                    //zona para recorrer y crear submatriz para enviar
                    //------------------------------------------------------
                    
                    itSubmatrix = 0;
                    for(int row = iniM; row < finM; row++){
                        for(int col = iniN; col < finN; col++){
                            double val = matrixGetRow(row, col, matriz);
                            printf("%.2f ", val);
                            subMatrix[itSubmatrix] = val;
                            itSubmatrix++;
                        }
                        printf("\n");
                    }
                    printf("\n");
                    
                    for(int k = 0; k < (finM-iniM); k++){
                        for(int l = 0; l < (finN-iniN); l++){
                            double val = submatrixGetRow(k, l, subMatrix);
                            printf("%.2f ", val);
                        }
                        printf("\n");
                    }
                    
                    printf("//------------------------------------------------------\n");
                    //------------------------------------------------------
                }
                
            }
        }
        printf("\n");
    }
    for(int k = 0; k < subM; k++){
        for(int l = 0; l < subN; l++){
            printf("%f ", matrixGetRow(k, l, subMatrix));
        }
        printf("\n");
    }
    //importante, pasarle dimensiones al subproceso
    return 0;
}
