CC = icpc 
CFLAGS = -g -O2 -fopenmp


all: clean cast  

cast: cast.o 
	$(CC) $(CFLAGS) -o cast ray_caster.o cast.o

cast.o: 
	$(CC) $(CFLAGS) -c *.c 

clean:
	rm -rf *.o cast image.ppm
