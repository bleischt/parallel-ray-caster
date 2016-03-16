#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//timer
#include <sys/stat.h>
#include <sys/time.h>

//mmap and file io
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "cast.h"

#define NUM_SPHERES 10000
#define OUTPUT_FILE "image.ppm"

/* Return time since epoch in ms */
double getTimeSeconds() 
{
   struct timeval  tv;
   gettimeofday(&tv, NULL);

   double time_in_mill = tv.tv_sec * 1000 + tv.tv_usec / 1000; 

   return time_in_mill / 1000;
}

FILE* open_file(char const name[]) 
{
   FILE* file;
   file = fopen(name, "r");

   if(file == NULL)
   {
      perror(name);
      exit(1);
   }

   return file;
}

int open_file_mmap(char const name[]) 
{
   int fd;
   fd = open(name, O_RDONLY);

   if(fd == -1) {
      perror("open");
      return fd;
   }

   return fd;
}

int sphere_retriever(FILE* file, sphere spheres[]) 
{
   int result;
   int i = 0;
   double x, y, z, radius, r, g, b ,ambient ,diffuse ,specular ,roughness;

   result = fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &x, &y,
      &z, &radius, &r, &g, &b, &ambient, &diffuse, &specular, &roughness);

   while(result != EOF && i != NUM_SPHERES) 
   {  
      spheres[i] = create_sphere(create_point(x, y, z), radius, 
         create_color(r, g, b), create_finish(ambient, diffuse, specular, roughness));
      result = fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &x, 
         &y, &z, &radius, &r, &g, &b, &ambient, &diffuse, &specular,
         &roughness); 
      i++;
   } 

   return i;
}



//Read a spheres into memory using mmap()
int fast_sphere_retriever(int fd, int num_lines, sphere spheres[]) 
{   
   double *read = (double*) mmap(0, sizeof(double) * 11 * num_lines, PROT_READ, MAP_PRIVATE, fd, 0);

    //free uneeded virtual memory
   // munmap(read, BYTES_TO_READ * rowSize * colSize);

  if (read == MAP_FAILED) {
    fprintf(stderr, "Failed read from file. Exiting...\n");
    exit(7);
  }
  
  for (int i = 0; i < num_lines * 11; i += 11) {
    spheres[i/11] = create_sphere(create_point(read[i], read[i+1], read[i+2]), read[i+3], 
         create_color(read[i+4], read[i+5], read[i+6]), create_finish(read[i+7], read[i+8], read[i+9], 
								      read[i+10]));
  }

  return num_lines;        
}

void fast_print_spheres(char *fileName, int num_lines)
{
   FILE *file = open_file(fileName);
   double print[NUM_SPHERES * 11]; 

   int result;
   int i = 0;
   //double x, y, z, radius, r, g, b ,ambient ,diffuse ,specular ,roughness;

   result = fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &print[0], &print[1],
      &print[2], &print[3], &print[4], &print[5], &print[6], &print[7], &print[8], &print[9], &print[10]);

   while(result != EOF && i/11 != NUM_SPHERES) 
   {  
      //spheres[i/11] = create_sphere(create_point(x, y, z), radius, 
      //   create_color(r, g, b), create_finish(ambient, diffuse, specular, roughness));
      result = fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &print[i+0], &print[i+1],
      &print[i+2], &print[i+3], &print[i+4], &print[i+5], &print[i+6], &print[i+7], &print[i+8], &print[i+9], &print[i+10]); 
      printf("radius: %f\n", print[i]);
      i+=11;
   } 

   //FILE *out = open_file("bunny_small_binary.in");
   FILE *out = fopen("dragon_binary.in", "w");
   //if(fd == -1) 
    //  perror("open");
   
   fwrite(print, sizeof(double), num_lines * 11, out);
   fclose(out);
}


int main(int argc, char *argv[]) 
{
   
  
   
   int numLines;
    
    if (strcmp(argv[1], "bunny_small_binary.in") == 0)
      numLines = 453;
    else if (strcmp(argv[1], "bunny_large_binary.in") == 0)
      numLines = 8171;
    else 
      numLines = 5205;

   //fast_print_spheres("dragon.in", numLines);
   printf("numLines: %d\n", numLines);


   FILE* file = open_file(argv[1]);
   //int in = open_file_mmap(argv[1]);
   FILE *out = fopen(OUTPUT_FILE, "w");
   
   int num_spheres;
   sphere spheres[NUM_SPHERES];
   double elapsedTime = getTimeSeconds();

   int width = 1024;
   int height = 768;
   int *printSpheres = (int *)malloc(sizeof(int) * width * height * 3);


   num_spheres = sphere_retriever(file, spheres);
   //num_spheres = fast_sphere_retriever(in, numLines, spheres);
   

   cast_all_rays(-10, 10, -7.5, 7.5, width, height, 
      create_point(0, 0, -14), printSpheres, spheres, num_spheres, create_color(1, 1, 1), create_light(create_point(-150, 50, -100), create_color(1.5, 1.5, 1.5)));

   
   fprintf(out, "P3 \n %d %d \n %d\n", width, height, COLOR_RANGE);   
   for(int x = 0; x < width * height; ++x) {
      fprintf(out, "%d %d %d\n", printSpheres[x * 3], printSpheres[x * 3 + 1], printSpheres[x * 3 + 2]);
   }
   fclose(file);
   //close(in);
   fclose(out);



   printf("Run time of Ray Casting: %.3f seconds\n", getTimeSeconds() - elapsedTime);
   return 0; 
}
