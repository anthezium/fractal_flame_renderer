#ifndef GLOBAL_H
#define GLOBAL_H

#include <GL/glut.h>

//types

typedef long double coord_t;
typedef GLfloat color_t;

typedef struct {
  coord_t x;
  coord_t y;
} coords;

typedef struct {
  color_t r;
  color_t g;
  color_t b;
} color;

//parameters for linear transformations
typedef struct {
  long double a,b,c,d,e,f;
} F_params;

//parametric coefficients for variational functions that require them
typedef struct {
  long double * p;
  int np;
} V_params;

//MASTER DESTRUCTOR!!

extern int master_cleanup();

#endif
