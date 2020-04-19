#gcc -g -Wall -march=x86-64 -o hello hello.c  -I. -L. -L/usr/lib64 -pthread -lm
DEBUG   =  -g
FLAGS  = -Wall -march=x86-64
CC     = gcc
INC    = -I.
LIB    = -L. -L/usr/lib64 -pthread -lm
SRC      = $(wildcard *.c)
OBJ      = $(SRC:.c=.o)
#EXEC     = $(shell basename $$(pwd))
EXEC     = $(SRC:.c=)

all: $(EXEC)

${EXEC}: $(OBJ)
	$(CC) $(DEBUG) $(FLAGS) -o $@ $^ $(INC) $(LIB)

%.o: %.c
	$(CC) $(DEBUG) $(FLAGS) -o $@ -c $< $(INC) $(LIB)

.PHONY: clean mrproper

clean:
	@rm -rf *.o

mrproper: clean
	@rm -rf $(EXEC)