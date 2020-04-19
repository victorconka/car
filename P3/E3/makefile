#gcc -g -Wall -march=x86-64 -o hello hello.c  -I. -L. -L/usr/lib64 -pthread -lm
DEBUG   =  -g
FLAGS  = -Wall -march=x86-64
CC     = mpicc
INC    = -I.
LIB    = -L. -L/usr/lib64 -pthread -lm
SRC      = $(wildcard *.c)
OBJ      = $(SRC:.c=.c)
EXEC     = $(SRC:.c=)

all: $(EXEC)

${EXEC}: $(OBJ)
	$(CC) $^ -o $@ $(INC) $(LIB)

clean:
	@rm -rf $(EXEC)