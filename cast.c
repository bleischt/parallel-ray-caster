#pragma offload_attribute(push,target(mic))
#include "cast.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ALLOC alloc_if(1) free_if(0)
#define REUSE alloc_if(0) free_if(0)
#define FREE alloc_if(0) free_if(1)

point create_point(double x, double y, double z) {
   point p;
   p.x = x;
   p.y = y;
   p.z = z;
   return p;
}

vector create_vector(double x, double y, double z) {
   vector v;
   v.x = x;
   v.y = y;
   v.z = z;
   return v;
}

ray create_ray(point p, vector dir) {
   ray r;
   r.p = p;
   r.dir = dir;
   return r;
}

sphere create_sphere(point center, double radius, color c, finish f) {
   sphere s;
   s.center = center;
   s.radius = radius;
   s.color = c;
   s.finish = f;
   return s;
}  

color create_color(double r, double g, double b) {
   color c;
   c.r = r;
   c.g = g;
   c.b = b;
   return c;
}

finish create_finish(double ambient, double diffuse, double specular, double roughness) {
   finish f;
   f.ambient = ambient;
   f.diffuse = diffuse;
   f.specular = specular;
   f.roughness = roughness;
   return f;
}

light create_light(point p, color c) {
   light l;
   l.p = p;
   l.c = c;
   return l;
}

view create_view(double min_x, double max_x, double min_y, double max_y, int width, int height) {
   view v;
   v.min_x = min_x;
   v.max_x = max_x;
   v.min_y = min_y;
   v.max_y = max_y;
   v.width = width;
   v.height = height;
   return v;
}

flag create_flag(point eye, view view, light light, color ambient) {
   flag f;
   f.eye = eye;
   f.view = view;
   f.light = light;
   f.ambient = ambient;
   return f;
}

vector scale_vector(vector v, double scalar){
   return create_vector(v.x * scalar, v.y * scalar, v.z * scalar);
}

double dot_vector(vector v1, vector v2){
   return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

double length_vector(vector v){
   return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

vector normalize_vector(vector v){
   double length = length_vector(v);
   return create_vector(v.x/length, v.y/length, v.z/length);
}

vector difference_point(point p1, point p2){
   return create_vector(p1.x-p2.x, p1.y-p2.y, p1.z-p2.z);
}

vector difference_vector(vector v1, vector v2){
   return create_vector(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);
}

point translate_point(point p, vector v){
   return create_point(p.x+v.x, p.y+v.y, p.z+v.z);
}

maybe_point maybePoint(int isPoint, point p){
   maybe_point x;
   x.isPoint=isPoint;
   x.p=p;
   return x;
} 

/*finding discriminant in order to decide whether the program should continue or end depending on real roots or not*/
double discriminant(ray* r, sphere* s){
   double a = dot_vector(r->dir, r->dir); 
   double b = 2 * dot_vector(difference_point(r->p, s->center),r->dir);
   double c = dot_vector(difference_point(r->p, s->center),difference_point(r->p, s->center)) - (s->radius * s->radius);
   return b*b-(4*a*c);;
 }
 
 /*using the quadratic formula. Returns the smallest root.*/
double quad_form(ray r, sphere s){
   double result1;
   double result2;
   double a = dot_vector(r.dir,r.dir); 
   double b = 2 * dot_vector(difference_point(r.p,s.center),r.dir);
   result1 = (((b-2*b) + sqrt(discriminant(&r, &s)))/(2*a));
   result2 = (((b-2*b) - sqrt(discriminant(&r, &s)))/(2*a));

   if(result1 > 0){
      if((result2 > 0) && (result2 < result1))
         return result2;
      else
         return result1;
   }
   else
      return result2;
}  

/*Finding the point on the sphere closest to the ray which is intersected by the ray*/
maybe_point sphere_intersection_point(ray r, sphere s){
   double quad;
   maybe_point intersect;
   ray scaled = r;
   quad = quad_form(r,s);

    if(discriminant(&r, &s) < 0 || (quad < 0)){
      intersect.isPoint = 0;
      return intersect;
   }else{
/*Scaling the vector to meet the sphere. Translating the point according to the the scaled vector*/
      scaled.dir = scale_vector(r.dir,quad);
      scaled.p = translate_point(scaled.p, scaled.dir);
      intersect.p = scaled.p;
      intersect.isPoint = 1;
      return intersect;
   }
}

int find_intersection_points(sphere const spheres[], int num_spheres, ray r, sphere hit_spheres[], point intersection_points[]){
   int i;
   int counter = 0;
   maybe_point mb; 

   //#pragma omp parallel for
   for(i = 0; i < num_spheres; i++)
   {
      mb = sphere_intersection_point(r, spheres[i]);

      if(mb.isPoint)
      {
         intersection_points[counter] = mb.p;
         hit_spheres[counter] = spheres[i];
         counter++;
      }
   }
   return counter;
}  

vector sphere_normal_at_point(point center, point p){
   return normalize_vector(difference_point(p, center));
}


double distance_from(point sphere, point ray) {
   return sqrt((ray.x - sphere.x) * (ray.x - sphere.x) + (ray.y - sphere.y)
      * (ray.y - sphere.y) + (ray.z - sphere.z) * (ray.z - sphere.z));
}

color into_int(color d) {
   if(d.r > 1)
      d.r = 1;
   if(d.g > 1)
      d.g = 1;
   if(d.b > 1)
     d.b = 1;

   d.r *= COLOR_RANGE;
   d.g *= COLOR_RANGE;
   d.b *= COLOR_RANGE;
   return d;
}

double spec_intense(vector lightdir, double visible_light, point pointE, vector n, point eye) {
   vector reflect = difference_vector(lightdir, scale_vector(n, 2 * visible_light));
   vector Vdir = normalize_vector(difference_point(pointE, eye));
   double specular_intensity = dot_vector(reflect, Vdir);
   return specular_intensity;
}

int obscured_point(sphere spheres[], int num_spheres, ray light_ray, point intersect) {
   int result = 0;
   sphere hit_spheres[num_spheres];
   point intersections[num_spheres];
   int found_points = find_intersection_points(spheres, num_spheres, light_ray, hit_spheres, intersections);
   double obscure = distance_from(intersect, light_ray.p);
  
   #pragma omp parallel for
   for(int i = 0; i < found_points; ++i) {
      if(obscure <= distance_from(intersections[i], intersect)) 
         result = 1;
   }

   return result;
}

color compute_ambience(sphere s, point intersect, color c, light light, sphere hit_sphere[], int num_spheres, point eye) {
   vector n = sphere_normal_at_point(s.center, intersect);
   point pointE = translate_point(intersect, scale_vector(n, .01));
   vector lightdir = normalize_vector(difference_point(light.p, pointE)); 
   double visible_light = dot_vector(n, lightdir);
   ray lightray = create_ray(pointE, lightdir);
   int obscure = obscured_point(hit_sphere, num_spheres, lightray, intersect);
   double spec_intensity = spec_intense(lightdir, visible_light, pointE, n, eye);
   int intensity;
   double decreaseCalcs;

   if(spec_intensity > 0)
      intensity = 1;
   else
      intensity = 0;

   color ambience;
      ambience.r = s.finish.ambient * c.r * s.color.r;
      ambience.g = s.finish.ambient * c.g * s.color.g;
      ambience.b = s.finish.ambient * c.b * s.color.b;

   color diffuse;
   color c_int;
   color colors;

   if(visible_light <= 0 || obscure == 1) {
      diffuse.r = 0;
      diffuse.g = 0;
      diffuse.b = 0;
   }
   else {
      decreaseCalcs = visible_light * s.finish.diffuse;
      diffuse.r =  light.c.r * decreaseCalcs * s.color.r;
      diffuse.g =  light.c.g * decreaseCalcs * s.color.g;
      diffuse.b =  light.c.b * decreaseCalcs * s.color.b;
   }

   if(intensity == 0) {
      c_int.r = 0;
      c_int.g = 0;
      c_int.b = 0;
   }
   else {
      decreaseCalcs = s.finish.specular * pow(spec_intensity, (1 / s.finish.roughness));
      c_int.r = light.c.r * decreaseCalcs;
      c_int.g = light.c.g * decreaseCalcs;
      c_int.b = light.c.b * decreaseCalcs;
   }

   colors.r = ambience.r + diffuse.r + c_int.r;
   colors.g = ambience.g + diffuse.g + c_int.g;
   colors.b = ambience.b + diffuse.b + c_int.b;

   return colors;
}

color cast_ray(ray r, sphere spheres[], int num_spheres, color color, light light, point eye) {                    
   int retColoridx = 0;
   struct color sphere_color = create_color(1, 1, 1);
   sphere hit_spheres[10000];

   point intersection_points[10000];
   int intersections = find_intersection_points(spheres, num_spheres, r, hit_spheres, intersection_points);
   double distances[intersections];
 
   if(intersections > 0)
   {
      double closest_distance = distance_from(intersection_points[0], r.p);
      retColoridx = 0;

      #pragma omp parallel for shared(intersection_points, eye, distances)
      for (int i = 1; i < intersections; i++)
      {
         distances[i] = distance_from(intersection_points[i], eye);
      }

      #pragma omp parallel for //reduction(min: closest_distance)
      for(int i = 1; i < intersections; i++) 
      {
        //double current_distance = distance_from(intersection_points[i], eye);
         //if (current_distance != distances[i])
         //   printf("cur: %f  dist: %f\n", current_distance, distances[i]);

         if(distances[i] < closest_distance) {//current_distance) {  
            retColoridx = i;
            closest_distance = distances[i];//current_distance;
         }
      }

      sphere_color = compute_ambience(hit_spheres[retColoridx], intersection_points[retColoridx], color, light, spheres, num_spheres, r.p);
   }

   return sphere_color;
}

#pragma offload_attribute(pop)


void cast_all_rays(double min_x, double max_x, double min_y, double max_y, int width, int height, point eye, int *printSpheres, sphere spheres[], int num_spheres, color color, light light) {
   double pixels_in_y = (max_y - min_y) / height;
   double pixels_in_x = (max_x - min_x) / width; 

#pragma offload target(mic) in(eye, light, color, num_spheres, height, width, pixels_in_x, pixels_in_y, min_x, min_y) in(spheres:length(num_spheres)) out(printSpheres:length(width*height*3)) 
{ 
   
   #pragma omp parallel for collapse(2) schedule(dynamic) shared(eye, light, color, num_spheres, height, width, pixels_in_x, pixels_in_y, min_x, min_y, printSpheres, spheres)
   for(int y = height; y > 0; --y) { 
   //printf("num threads: %d\n", omp_get_num_threads());   
      //#pragma omp parallel for schedule(auto)
      for(int x = 0; x < width; ++x) {
         ray r = create_ray(eye, difference_point(create_point(x * pixels_in_x + min_x, y * pixels_in_y + min_y, 0), eye));
         struct color sphere_color = into_int(cast_ray(r, spheres, num_spheres, color, light, eye));
         printSpheres[(height - y) *  width * 3 + x * 3] = (int)sphere_color.r;
         printSpheres[(height - y) *  width * 3 + x * 3 + 1] = (int)sphere_color.g;
         printSpheres[(height - y) *  width * 3 + x * 3 + 2] = (int)sphere_color.b;
      }
   } 
}
}


