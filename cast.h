#pragma offload_attribute(push,target(mic))
#ifndef CAST_H
#define  CAST_H
#define PI 3.141592
#define COLOR_RANGE 255

typedef struct point {
  double x;
  double y;
  double z;
} point;

typedef struct maybe_point
{
  int isPoint;
  struct point p;
} maybe_point;

typedef struct vector {
  double x;
  double y;
  double z;
} vector;

typedef struct ray {
  point p;
  vector dir;
} ray;

typedef struct color {
   double r;
   double g;
   double b;
} color;

typedef struct finish {
  double ambient;
  double diffuse;
  double specular;
  double roughness;
} finish;

typedef struct sphere {
  point center;
  double radius;
  color color;
  finish finish;
} sphere;

typedef struct light {
   point p;
   color c;
} light;

typedef struct view
{
   double min_x;
   double max_x;
   double min_y;
   double max_y;
   int width;
   int height;
} view;

typedef struct flag
{
   point eye;
   view view;
   light light;
   color ambient;
} flag;

//Vector_math
vector scale_vector(vector v, double scalar);
double dot_vector(vector v1, vector v2);
double length_vector(vector v);
vector normalize_vector(vector v);
vector difference_point(point p1, point p2);
vector difference_vector(vector v1, vector v2);
point translate_point(point p, vector v);

//Data
point create_point(double x, double y, double z);
vector create_vector(double x, double y, double z);
ray create_ray(point p, vector dir);
sphere create_sphere(point center, double radius, color c, finish f);
color create_color(double r, double g, double b);  
finish create_finish(double ambient, double diffuse, double specular, double roughness);
light create_light(point p, color c);
view create_view( double min_x, double max_x, double min_y, double max_y, int width, int height);
flag create_flag(point eye, view view, light light, color ambient);

//Collisions
double discriminant(ray* r, sphere* s);
double quad_form(ray r, sphere s);
maybe_point maybePoint(int isPoint, point p);
maybe_point sphere_intersection_point(ray r, sphere s);
int find_intersection_points(sphere const spheres[], int num_spheres, ray r, sphere hit_spheres[], point intersection_points[]);
vector sphere_normal_at_point(point center, point p);

//Cast
double distance_from(point sphere, point ray);
color into_int(color d);
double spec_intense(vector lightdir, double visible_light, point pointE, vector n, point eye);  
int obscured_point(sphere spheres[], int num_spheres, ray light_ray, point intersect);
color compute_ambience(sphere s, point intersect, color c, light light, sphere hit_sphere[], int num_spheres, point eye);
void cast_all_rays(double min_x, double max_x, double min_y, double max_y, int width, int height,point eye, int *printSpheres, sphere spheres[], int num_spheres, color color, light light);  
#endif 
#pragma offload_attribute(pop)
