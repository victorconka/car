#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <ctype.h>

int main(int argc, char *argv[])
{

    char text[100];               //array para la cadena de texto introducida por pantalla
    int buffer[1], buffer_add[1]; //espacio reservado par el buffer
    int parada = 1;               //entero para indicar una parada
    int rank, size;               //distintas variables para rank, tamaño, opcion entre otras.

    MPI_Status status;                    //Se utiliza para guardar información sobre operaciones de recepción de mensajes
    MPI_Init(&argc, &argv);               //Inicializamos el MPI
    MPI_Comm_size(MPI_COMM_WORLD, &size); //Este tipo de dato guarda toda la información relevante sobre un comunicador específico. Se utiliza para especificar el comunicador por el que se desea realizar las operaciónes de transmisión o recepción, en este caso es el tamaño
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //Guarda la informacion del rank segun la opcion seleccionada.
    //MPI_COMM_WORLD constante que define al comunicador global y que no es necesario declarar.

    //si el numero de procesadores es menor al ejecutar el programa muestra este mensaje.
    if (size != 4)
    {
        printf("Debe de ejecutar este programa con 4 procesadores-use el comando mpirun -np 4./progrma\n");
        fflush(stdout); //liberamos buffer de escritura.
        MPI_Finalize(); //finalizamos el programa
        return 0;
    }

    //Mientras la parada sea distinta a 0 se ejecutara nuestro menu y sus funciones
    while (parada != 0)
    {

        //el proceso 0 muestra nuestro menu asi como solicita la opcion a realizar y el texto
        if (rank == 0)
        {
            printf("\n");
            printf("************************************************\n");
            printf("Usuario-Procesador %d\n", rank);
            printf("Rutinas y programacion en MPI\n");
            printf("************************************************\n");
            printf("\n");
            printf("0.Finalizar programa\n");
            printf("1.Conversor a mayusculas\n");
            printf("2.Invertir texto\n");
            printf("3.Sumar caracteres\n");
            printf("4.Llevar a cabo todas las opciones\n");
            printf("\n");
            printf("****** Autor: Manuel Gonzalez Rodriguez ********\n");
            printf("\n");
            printf("Introduzca un numero de opcion: ");
            fflush(stdout);
            int option;
            scanf("%d", &option);

            //Enviamos la opcion a los demas procesos por medio del buffer declarado
            buffer[0] = option;
            for (int i = 1; i < size; i++)
            {
                MPI_Send(buffer, 1, MPI_INT, i, 123, MPI_COMM_WORLD);
            }

            // Si la opcion enviada corresponde a la 0 finaliza nuestro programa
            if (option == 0)
            {
                MPI_Finalize();
                return 0;
            }

            //Por el contrario si es distinta se solicitara al usuario el mensaje a enviar
            printf("Introduzca un mensaje: ");
            fflush(stdout);
            scanf("%s", text);

            //Mandamos el mensaje a un procesador determinado dependiendo de lña opcion selecionada y esperamos la respuesta del mismo
            if (option == 1)
            {

                MPI_Send(text, 100, MPI_CHAR, 1, 123, MPI_COMM_WORLD);
                MPI_Recv(text, 100, MPI_CHAR, 1, 123, MPI_COMM_WORLD, &status);
                printf("Mensaje enviado por el procesador 1: %s \n", text);
                //printf("\n");
            }
            else if (option == 2)
            {
                MPI_Send(text, 100, MPI_CHAR, 2, 123, MPI_COMM_WORLD);
                MPI_Recv(text, 100, MPI_CHAR, 2, 123, MPI_COMM_WORLD, &status);
                printf("Mensaje enviado por el procesador 2: %s \n", text);
            }
            else if (option == 3)
            {
                MPI_Send(text, 100, MPI_CHAR, 3, 123, MPI_COMM_WORLD);
                MPI_Recv(buffer_add, 1, MPI_INT, 3, 123, MPI_COMM_WORLD, &status);
                printf("Mensaje enviado por el procesador 3: %d \n", buffer_add[0]);

                // En el caso de haber seleccionado la opcion 4 se realizara un broadcast del mensaje solicitando la respuesta de cada proceso
            }
            else if (option == 4)
            {

                for (int i = 1; i < size; i++)
                {
                    MPI_Send(text, 100, MPI_CHAR, i, 123, MPI_COMM_WORLD);
                }
                //Recibimos los mensajes de los procesos invocados

                MPI_Recv(text, 100, MPI_CHAR, 1, 123, MPI_COMM_WORLD, &status);
                printf("Mensaje del procesador %d recibido por el procesador %d %s \n", rank, 1, text);
                MPI_Recv(text, 100, MPI_CHAR, 2, 123, MPI_COMM_WORLD, &status);
                printf("Mensaje del procesador %d recibido por el procesador %d %s \n", rank, 2, text);
                MPI_Recv(buffer_add, 1, MPI_INT, 3, 123, MPI_COMM_WORLD, &status);
                printf("Mensaje del procesador %d recibido por el procesador %d %d \n", rank, 3, buffer_add[0]);
            }
        }
        //Si el rango del mensaje es distinto a 0 entramos aqui
        else
        {
            //Recibimos la peticion y dependiendo de la opcion la tarea sera realizada por un procesador en cuestion.
            buffer[0] = -1;
            MPI_Recv(buffer, 1, MPI_INT, 0, 123, MPI_COMM_WORLD, &status);
            int option = buffer[0];

            // Si la opcion es 0 finalizamos el programa
            if (option == 0)
            {
                MPI_Finalize();
                return 0;
            }
            //Comprobamos el destino del mensaje (rango)
            if (rank == option || option == 4)
            {
                //recibimos el mensaje
                MPI_Recv(text, 100, MPI_CHAR, 0, 123, MPI_COMM_WORLD, &status);
                //Si el rango del mensaje es 1 se modificara el texto a mayusculaas
                if (rank == 1)
                {
                    printf("Cambiando el texto a mayusculas..\n");
                    for (int i = 0; i < strlen(text); i++)
                    {
                        text[i] = toupper(text[i]);
                    }
                    //Enviamos al proceso 0 el texto modificado.
                    MPI_Send(text, 100, MPI_CHAR, 0, 123, MPI_COMM_WORLD);
                }
                else if (rank == 2)
                {
                    //Si el rango es 2 el procesador 2 invertira el orden del texto y lo devolvera al proceso 0

                    printf("Invirtiendo orden del texto..\n");
                    int long_text = strlen(text); //longitud de la cadena recibida
                    int j, i;
                    char temporal;
                    for (j = long_text - 1, i = 0; i < long_text / 2; i++, j--)
                    {
                        temporal = text[i];
                        text[i] = text[j];
                        text[j] = temporal;
                    }

                    MPI_Send(text, 100, MPI_CHAR, 0, 123, MPI_COMM_WORLD); //lo mandamos al proceso 0
                }
                else if (rank == 3)
                {
                    // Calculamos el valor de la suma de los caracteres y lo enviamos al proceso 0
                    printf("Calculando el valor de los caracteres..\n");
                    int suma = 0;
                    for (int i = 0; i < strlen(text); i++)
                    {
                        int valor_letra = text[i];
                        suma = suma + valor_letra;
                    }
                    buffer_add[0] = suma;
                    MPI_Send(buffer_add, 1, MPI_INT, 0, 123, MPI_COMM_WORLD);
                }
            }
        }
    }
}
