#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "variations.h"

//globals

#define NVARIATIONS 5

//variations
V_func * variations;
int nv=0;

//final transformation
V_func * final;
F_params * finalfp;

//nonlinear functions.  these are externally linked because pointers to them
//will be used in functio of this file

#define RSQUARED(c) ((c)->x*(c)->x + (c)->y*(c)->y)
#define INVRSQUARED(c) (1.0/RSQUARED(c))
#define R(c) (sqrtl(RSQUARED(c)))
#define INVR(c) (1.0/R(c))

//linear
//NO FP, NO VP
extern int v0(coords * c,
              F_params * fp,
              V_params * vp){
  //v0 doesn't modify anything
  return 1;              
}

//sinusoidal
//NO FP, NO VP
extern int v1(coords * c,
              F_params * fp,
              V_params * vp){
  c->x=sinl(c->x);
  c->y=sinl(c->y);            
  return 1;            
}

//spherical
//NO FP, NO VP
extern int v2(coords * c,
              F_params * fp,
              V_params * vp){
  static long double invrsquared;
  invrsquared = INVRSQUARED(c);
  c->x=c->x*invrsquared;
  c->y=c->y*invrsquared;            
  return 1;            
}

//swirl
//NO FP, NO VP
extern int v3(coords * c,
              F_params * fp,
              V_params * vp){
  static long double rsquared; 
  static long double sinrs;
  static long double cosrs;
  
  rsquared = RSQUARED(c);
  sinrs = sinl(rsquared);
  cosrs = cosl(rsquared);
  c->x = c->x*sinrs - c->y*cosrs;
  c->y = c->x*cosrs + c->y*sinrs;
  return 1;
}

//horseshoe
//NO FP, NO VP
extern int v4(coords * c,
              F_params * fp,
              V_params * vp){
  static long double invr;
  invr = INVR(c);
  c->x = invr*(c->x - c->y)*(c->x + c->y);
  c->y = invr*2.0*c->x*c->y;
  return 1;              
}

//public functions (in the header)

//loads variations, allocates memory for variations and assigns them for use
//returns the number of variations initialized on success, 0 on failure
extern int init_variations(){
  int j;
  int (*v[])(coords * c,
           F_params * fp,
           V_params * vp) = { &v0, &v1, &v2, &v3, &v4 };
  V_params vp;
  //try and load variations
  /*
  if(!load_variations()){
    printf("init_variations: load_variations failed... returning FALSE\n");
    return 0;
  }
  */
  //fill variations array
  nv = NVARIATIONS;
  vp.p = NULL;
  vp.np = 0;
  variations = malloc(sizeof(V_func) * nv);
  for(j=0; j<nv; j++){
    variations[j].v=v[j];
    variations[j].use_fp = 0;
    variations[j].use_vp = 0;
    variations[j].vp = vp;
  }
  
  //final transformation
  final = malloc(sizeof(V_func));
  final->v=&v0;
  final->use_fp = 0;
  final->use_vp = 0;
  //final->vp will just contain some random bit pattern
  finalfp = NULL;
  /*
  for(j=0; j<nv; j++){
    variations[j] = get_variation(j);
    if(variations[j] == NULL){
      printf("init_variations: get_variation(%d) failed... returning FALSE\n");
      return 0;
    }
  }
  */
  
  return nv;
}

//only call this after init_variations has been called, so nv will be set
extern int cleanup_variations(){
  int j;
  
  printf("cleanup_variations: about to free\n");
  
  for(j=0; j<nv; j++){
    if(variations[j].vp.np != 0)
      free(variations[j].vp.p);
  }
  free(variations);
  free(final);
  return 1;
}

//accessors

extern V_func * get_variations(){
  return variations;
}

extern V_func * get_final(){
  return final;
}

#define NONLINEAR(v,c,fp) ((*(v)->v)(c, fp, &(v)->vp))

//run nonlinear function
//returns v->v's return value
extern int run_v(V_func * v, coords * c, F_params * fp){
  return NONLINEAR(v,c,fp);
}
