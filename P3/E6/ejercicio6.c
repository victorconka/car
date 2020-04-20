#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#define MASTER 0
#define UNO 1
#define DOS 2
#define TRES 3
#define CUATRO 4

double seed = 12353.0;
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

double *crearVectorAleatorio(int tam)
{
    double *vec = malloc(sizeof(double) * tam);
    for (int i = 0; i < tam; i++)
    {
        vec[i] = myrand();
    }
    return vec;
}

int read_option()
{
    char term;
    int number;
    int loop = 1;
    while (loop == 1)
    {
        puts("Tell me something nice: [0, 1, 2, 3, 4]");
        puts("0: SALIR");
        puts("1: TEXTO A MAYUSCULAS");
        puts("2: NUMEROS REALES AL CUADRADO");
        puts("3: CUADRADO DE SUMA DE VALORES ASCII");
        puts("4: EJECUTAR 1..3 SIMULTANEAMENTE");
        if (scanf("%d", &number) != 1)
        {
            term = getchar();
            printf("INVALID INPUT: %c. AVAILABLE OPTIONS: [0,1,2,3,4]\n", term);
        }
        else
        {
            if (number >= 0 && number < 5)
            {
                loop = 0;
            }
            else
            {
                printf("INVALID INPUT: %d. AVAILABLE OPTIONS: [0,1,2,3,4]\n", number);
            }
        }
    }
    return number;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int world_size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int char_length = 20; //longitud del texto a leer
    double *vec;

    if (world_size != 4)
    {
        printf("Llamada: mpirun -n n prog\n");
        MPI_Finalize();
        return 0;
    }

    int condition = 1;
    while (condition == 1)
    {
        if (rank == MASTER)
        {
            int option = read_option();
            //send option to every process
            for (int i = 1; i < world_size; i++)
            {
                MPI_Send(&option, 1, MPI_INT, i, i, MPI_COMM_WORLD);
            }
            char input_string[char_length];
            int res = -1;
            switch (option)
            {
            case 1:
                printf("\nEnter string without spaces of length < %d\n", char_length);
                fflush(stdin);
                scanf("%s", input_string);
                MPI_Send(input_string, char_length, MPI_CHAR, UNO, UNO, MPI_COMM_WORLD);
                MPI_Recv(input_string, char_length, MPI_CHAR, UNO, UNO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("toupper(): %s\n", input_string);
                fflush(stdout);
                break;
            case 2:
                vec = crearVectorAleatorio(10);
                for (int i = 0; i < 10; i++)
                {
                    printf("%.2f ", vec[i]);
                }
                printf("\n");
                MPI_Send(vec, 10, MPI_DOUBLE, DOS, DOS, MPI_COMM_WORLD);
                MPI_Recv(vec, 1, MPI_DOUBLE, DOS, DOS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Resultado de sqrt(sum(vec)): %f\n", vec[0]);
                free(vec);
                fflush(stdout);
                break;
            case 3:
                printf("\nEscriba mensaje para el proceso 3 sin espacios y de longitud < %d\n", char_length);
                fflush(stdin);
                scanf("%s", input_string);
                MPI_Send(input_string, char_length, MPI_CHAR, TRES, TRES, MPI_COMM_WORLD);
                
                MPI_Recv(&res, char_length, MPI_INT, TRES, TRES, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Suma de caracteres del mensaje escrito: %d\n", res);
                fflush(stdout);
                break;
            case 4:
                printf("Escriba un mensaje sin espacios y de longitud < %d\n", char_length);
                printf("mensaje que se enviará: ");
                fflush(stdin);
                scanf("%s", input_string);
                vec = crearVectorAleatorio(10);
                printf("vector para calcular: \n");
                for (int i = 0; i < 10; i++)
                {
                    printf("%.2f ", vec[i]);
                }
                printf("\n");
                //********************************************************//
                //proceso 1
                MPI_Send(input_string, char_length, MPI_CHAR, UNO, UNO, MPI_COMM_WORLD);
                MPI_Recv(input_string, char_length, MPI_CHAR, UNO, UNO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Proceso 1: toupper(): %s\n", input_string);
                //proceso 2
                vec = crearVectorAleatorio(10);
                MPI_Send(vec, 10, MPI_DOUBLE, DOS, DOS, MPI_COMM_WORLD);
                MPI_Recv(vec, 1, MPI_DOUBLE, DOS, DOS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Proceso 2: sqrt(sum(vec)): %f\n", vec[0]);
                //proceso 3
                MPI_Send(input_string, char_length, MPI_CHAR, TRES, TRES, MPI_COMM_WORLD);
                MPI_Recv(&res, char_length, MPI_INT, TRES, TRES, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Proceso 3: Suma de caracteres: %d\n", res);
                break;
            default:
                MPI_Finalize();
                return 0;
            }
        }
        else
        {
            int option = -1;
            MPI_Recv(&option, 1, MPI_INT, MASTER, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (option == 0)
            {
                MPI_Finalize();
                return 0;
            }
            if (rank == option || option == 4)
            {
                if (rank == UNO)
                {
                    char buf[char_length];
                    MPI_Recv(buf, char_length, MPI_CHAR, MASTER, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    for (int i = 0; i < char_length; i++)
                    {
                        buf[i] = toupper(buf[i]);
                    }
                    MPI_Send(buf, char_length, MPI_CHAR, MASTER, rank, MPI_COMM_WORLD);
                }
                else if (rank == DOS)
                {
                    double vec[10];
                    MPI_Recv(vec, 10, MPI_DOUBLE, MASTER, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    double res = 0.0;
                    for (int i = 0; i < 10; i++)
                    {
                        res += vec[i];
                    }
                    res *= res;
                    MPI_Send(&res, 1, MPI_DOUBLE, MASTER, rank, MPI_COMM_WORLD);
                }
                else if (rank == TRES)
                {
                    printf("Entrando en funcionalidad 3\n");
                    char buf[char_length];
                    int res = 0;
                    MPI_Recv(buf, char_length, MPI_CHAR, MASTER, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    for (int i = 0; i < char_length; i++)
                    {
                        //printf("%d - %d\n", i, buf[i]);
                        if (buf[i] < 127) //excluir signo delete
                            res += (int)buf[i];
                    }
                    MPI_Send(&res, 1, MPI_INT, MASTER, rank, MPI_COMM_WORLD);
                }
                else
                { //cuatro
                }
            }
        }
    }
}