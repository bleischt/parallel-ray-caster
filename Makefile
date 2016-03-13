CC = icpc 
CFLAGS = -g -O2 -fopenmp -lcudart


all: clean cast cu 

cu: cu.o 
	$(CC) $(CFLAGS) -o castCu rayCu.o castCu.o

cu.o: 
	$(CC) $(CFLAGS) -D CUDA -c -o rayCu.o ray_caster.c
	nvcc --optimize 2 -c castCu.cu

cast: cast.o 
	$(CC) $(CFLAGS) -o cast ray_caster.o cast.o

cast.o: 
	$(CC) $(CFLAGS) -c *.c 

clean:
	rm -rf *.o cast castCu image.ppm