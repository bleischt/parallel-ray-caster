#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>

#define NUM_SPHERES 10000
#define OUTPUT_FILE "image.ppm"

#include "cast.h"

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

int sphere_retriever(FILE* file, sphere spheres[]) 
{
   int result;
   int i = 0;
   double x, y, z, radius, r, g,b ,ambient ,diffuse ,specular ,roughness;

   result = fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &x, &y,
      &z, &radius, &r, &g, &b, &ambient, &diffuse, &specular, &roughness);

   while(result != EOF && i != 10000) 
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




int main(int argc, char *argv[]) 
{
   FILE* file = open_file(argv[1]);
   FILE *out = fopen(OUTPUT_FILE, "w");

   int num_spheres;
   struct sphere spheres[NUM_SPHERES];
   double elapsedTime = getTimeSeconds();

   int width = 1024;
   int height = 768;
   int *printSpheres = (int *)malloc(sizeof(int) * width * height * 3);

/*
   int *testValues = (int *) calloc(NUM_SPHERES, sizeof(int));

   #pragma offload target(mic) inout(testValues:length(NUM_SPHERES))
   #pragma omp parallel for
   for (int i = 0; i < NUM_SPHERES; i++)
   {
      testValues[i] = 1;
   }

   for (int i = 0; i < NUM_SPHERES; i++)
      if (testValues[i] < 1) {
         printf("DIDN'T WORK, yo\n");
         break;
      }
*/

   num_spheres = sphere_retriever(file, spheres);

   //#pragma offload target(mic) in(width, height) out(printSpheres:length(width*height)) 
   cast_all_rays(-10, 10, -7.5, 7.5, width, height, 
      create_point(0, 0, -14), printSpheres, spheres, num_spheres, create_color(1, 1, 1), create_light(create_point(-150, 50, -100), create_color(1.5, 1.5, 1.5)));

   
   fprintf(out, "P3 \n %d %d \n %d\n", width, height, COLOR_RANGE);   
   for(int x = 0; x < width * height; ++x) {
      fprintf(out, "%d %d %d\n", printSpheres[x * 3], printSpheres[x * 3 + 1], printSpheres[x * 3 + 2]);
   }
   fclose(file);
   fclose(out);



   printf("Run time of Ray Casting: %.3f seconds\n", getTimeSeconds() - elapsedTime);
   return 0; 
}
