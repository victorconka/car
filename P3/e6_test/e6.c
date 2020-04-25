// definir librerias de biblioteca estandar de C, y de MPI. esto ultimo es de comunicacion de procesos mediante pase de mensajes
#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <ctype.h>

int opcion = 0;
char cadena[20];

// ejecutar metodo principal 'main'
int main(int argc, char **argv)
{
      int rank, size; // definir variables rango y tamano, es decir, que procesador se esta ejecutando, y cuantos procesadores hay en total

      MPI_Status status; // contiene datos como el origen, la etiqueta y el tamano del MPI
      MPI_Init(&argc, &argv); // iniciar procesos MPI, antes de esta linea de codigo no se pueden introducir procesos MPI
      MPI_Comm_size(MPI_COMM_WORLD, &size); // tamano comunicador seleccionado
      MPI_Comm_rank(MPI_COMM_WORLD, &rank); // rango o identificador del proceso que lo llama dentro del comunicador seleccionado

      printf("I am processor number %i.  Numer of total processors = %i\n \n", rank, size);
      fflush(stdout);

      if (rank==0) { // si soy procesador 0, hacer
          printf("Introduzca opción: \n"); // pedir al usuario que introduzca una opcion (numero) de como se ejecutara el programa
          fflush(stdin);
          fflush(stdout);
          scanf("%d", &opcion); // guardar en la variable 'opcion' la opcion introducida por el usuario previamente
          fflush(stdin);
          fflush(stdout);
          printf("Opción escogida: %c \n", opcion); // mostrar en consola la opcion seleccionada por el usuario previamente, como confirmacion
          MPI_Send(&opcion, 1, MPI_INT, 1, 123, MPI_COMM_WORLD); // enviar la opcion especificada por el usuario al 'mpirecv' correspondiente
          
          printf("Introduzca el texto: \n");
          scanf("%s", cadena);
          printf("Cadena leida: %s \n", cadena);
          MPI_Send(&cadena, 20, MPI_CHAR, 1, 512, MPI_COMM_WORLD);
          
          MPI_Recv(&cadena, 20, MPI_CHAR, 1, 433, MPI_COMM_WORLD, &status);
          printf("Cadena a mayusculas: %s \n", cadena);
          
      }else{
          printf("esta linea no imprime\n");
          MPI_Recv(&opcion, 1, MPI_INT, 0, 123, MPI_COMM_WORLD, &status);
          
          if (opcion==0) {
              printf("Termino.");
          }else{
              if (rank==1 && opcion==1) {
                  MPI_Recv(&cadena, 20, MPI_CHAR, 0, 512, MPI_COMM_WORLD, &status);
                  for (int indice = 0; cadena[indice] != '\0'; ++indice) {
                      cadena[indice] = toupper(cadena[indice]);
                  }
                  MPI_Send(&cadena, 20, MPI_CHAR, 0, 433, MPI_COMM_WORLD);
              }
          }
          
      }

      printf("Rank: %d Cerrando programa...\n", rank);

      MPI_Finalize(); // finalizar procesos MPI, despues de esta linea de codigo no se pueden introducir mas procesos MPI
      return 0; // salir del actual metodo 'main'
}
