CC = gcc
OPTIONS = -march=native -masm=intel -msse4.2 -mlzcnt -mbmi2 -Ofast
OMIT = -fomit-frame-pointer

all:
	$(CC) -o Leena $(OPTIONS) $(OMIT) *.c

debug:
	$(CC) -o Leena -march=native -g *.c
	
asm:
	$(CC) -S -fverbose-asm  -fopt-info-vec-optimized $(OPTIONS) $(OMIT) *.c

clean:
	rm Leena *.s *.o