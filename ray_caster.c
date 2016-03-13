#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifdef CUDA
#include "castCu.h"
#else
#include "cast.h"
#endif

/* Return time since epoch in ms */
double getTimeSeconds() {
   struct timeval  tv;
   gettimeofday(&tv, NULL);

   double time_in_mill = tv.tv_sec * 1000 + tv.tv_usec / 1000; 

   return time_in_mill / 1000;
}

FILE* open_file(char const name[]) {
   FILE* file;
   file = fopen(name, "r");
   if(file == NULL)
   {
      perror(name);
      exit(1);
   }
   return file;
}

int sphere_retriever(FILE* file, sphere spheres[]) {
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


int main(int argc, char *argv[]) {
   FILE* file;
   int num_spheres;
   double elapsedTime = getTimeSeconds();
   struct sphere spheres[10000];
   file = open_file(argv[1]);
   num_spheres = sphere_retriever(file, spheres);
   cast_all_rays(-10, 10, -7.5, 7.5, 1024, 768, 
      create_point(0, 0, -14), spheres, num_spheres, create_color(1, 1, 1), create_light(create_point(-150, 50, -100), create_color(1.5, 1.5, 1.5)));
   fclose(file);

   printf("Run time of Ray Casting: %.3f seconds\n", getTimeSeconds() - elapsedTime);
   return 0; 
}
