
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int sizeM = 0,//fila
	sizeN = 0;//columna
double seqCheck(const double *matrix)
{
	double res = 0;
	double val = 0;
	for (int i = 0; i < sizeM*sizeN; i++){
		val = *(matrix + i);
		res+=val;
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

void matrixSet(int posM, int posN, double *matrix, double value)
{
	*(matrix + posM * sizeN + posN) = value;
}

double matrixGet(int posM, int posN, const double *matrix)
{
	double value = *(matrix + posM * sizeN + posN);
	return value;
}

double *crearMatrizAleatoria(int sizeM, int sizeN)
{
	double *matrix = calloc(sizeM * sizeN ,sizeof(double));
	for (int i = 0; i < sizeM; i++)
	{
		for (int j = 0; j < sizeN; j++)
		{
			matrixSet(i, j, matrix, myrand());
		}
	}
	return matrix;
}