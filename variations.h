#include "global.h"

#ifndef VARIATIONS_H
#define VARIATIONS_H

//POSSIBLE OPTIMIZATION.  instead of using function pointers, just use a macro 
//to make function names instead of function pointers.  can we make function
//names at runtime... probably not, actually.

//nonlinear transformation
typedef struct {
  int (*v)(coords * c,
           F_params * fp,
           V_params * vp);
  int use_fp;
  int use_vp;
  V_params vp;
} V_func;

//public

//initialization/teardown

extern int init_variations();
extern int cleanup_variations();

//accessors  
        
extern V_func * get_variations();
extern V_func * get_final();

//run functions
extern int run_v(V_func * v, coords * c, F_params * fp);

#endif
