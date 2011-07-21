/* Author: Ted Cooper
 * Last revised: 7-2-2009
 * FRACTAL FLAME RENDERER
 * See top of engine.c for program description.
 *
 * functions.h: see functions.c for description.
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "variations.h"

//DATA TYPES

//linear transformation
typedef struct {
  int (*f)(coords * c,
           F_params * fp);
  F_params fp;
} F_func;

//a linear function Fi, excluding final transformation Ffinal
typedef struct {
  //going from inside out in function structure
  
  //1. initial linear transformation
  F_func f;
  
  //2. array of nonlinear variations and variational coefficients
  V_func * v;
  long double * v_coeff;
  int nv;
  
  //3. linear post transformation
  F_func p;
  
  //color index associated with this function
  float c;
  
  //weight associated with this function
  float w;
  float startw; //where in the weight vector this function's weight range
                //begins

} F;

//FUNCTIONS

//public

//init/teardown:
//this init should take care of _everything_
extern int init_functions(int nframes);
extern int cleanup_functions();

//invoke functions:
extern int run_function(float vector_pos, coords * c, float * ci);
extern int run_final(coords * c, float * cfinal);

//accessors
extern float get_weight_vector_len();

//mutators
extern int set_frame(int t);
#endif
