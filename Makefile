CC = gcc
OPTIONS = -march=native -mlzcnt -mbmi2 -Ofast
OMIT = -fomit-frame-pointer

all:
	$(CC) -o Leena $(OPTIONS) $(OMIT) ./src/*.c

run:
	$(CC) -o Leena $(OPTIONS) $(OMIT) ./src/*.c
	./Leena

profile:
	$(CC) -p -g -o Leena $(OPTIONS) ./src/*.c
	./Leena

debug:
	$(CC) -Wall -o Leena -march=native -g ./src/*.c
	
asm:
	$(CC) -S -fverbose-asm  -fopt-info-vec-optimized $(OPTIONS) $(OMIT) ./src/*.c

clean:
	rm Leena *.s *.o