CC = gcc
OPTIONS = -march=native -mlzcnt -mbmi2 -Ofast
OMIT = -fomit-frame-pointer

all:
	$(CC) -o Leena $(OPTIONS) $(OMIT) ./src/*.c

run:
	$(CC) -Wall -o Leena $(OPTIONS) $(OMIT) ./src/*.c
	./Leena

profile:
	$(CC) -p -g -o Leena.bin $(OPTIONS) ./src/*.c
	./Leena.bin

debug:
	$(CC) -Wall -O0 -o Leena -march=native -g ./src/*.c
	
asm:
	$(CC) -S -fverbose-asm  -fopt-info-vec-optimized $(OPTIONS) $(OMIT) ./src/*.c

clean:
	rm Leena *.s *.o