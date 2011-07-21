/* Author: Ted Cooper
 * Last revised: 7-2-2009
 * FRACTAL FLAME RENDERER
 * See top of engine.c for program description.
 *
 * functions.c: with functions.h, provides an interface for initializing and 
 * running functions (linear (F-type) and nonlinear (V-type)) needed for 
 * rendering.  Obtuse nomenclature is mostly from the paper, but I could have
 * done better.
 */
 
//INCLUDES

#include <stdio.h> //for debugging
#include <stdlib.h>
#include <math.h>
#include "functions.h"
#include "variations.h"

//GLOBALS

//variations
static V_func * variations;
static coord_t * v_coeff;
static int nv;

//functions
static F * functions;
static int nfunctions;
float weight_vector_len;

//final transformation
static V_func * final;
static F_params * finalfp;
static float cfinal;

//animation
static int nframes;
static coord_t dv_coeff;  //rate of variation coefficient change

//FUNCTIONS

//private

//function: linear_transformation
//purpose: perform linear transformation specified by fp on c
//returns TRUE
extern int linear_transformation(coords * c,
                                 F_params * fp){
  c->x = c->x*fp->a + c->y*fp->b + fp->c;
  c->y = c->x*fp->d + c->y*fp->e + fp->f;
  
  return 1;                                
}

//function: identity_transformation
//purpose: don't change c
//returns TRUE
extern int identity_transformation(coords *c,
                                   F_params * fp){
  return 1;                                  
}

//function: run_f
//purpose: run the specified linear function on the specified coordinate pair.
//         roughly corresponds to Fi definition on p.5 of Draves' paper.
//params: func - pointer to the function we want to apply.
//        coords - pointer to coordinate pair to which we want to apply the
//        function.
//return TRUE on success, FALSE on failure

//quick linear function
#define LINEAR(f_struct, c) ((*(f_struct).f)(c, &(f_struct).fp))

static int run_f(F * func, coords * c){
  static int j;
  static coords ccopy;
  static coords ctemp;
  
  //first linear transformation associated with this function
  if(!LINEAR(func->f, c)){
    fprintf(stderr,
            "run_f: first linear transformation failed.  returning...\n");
    return 0;
  }
#if defined(DEBUG)  
  fprintf(stderr,"run_f: past first linear transformation. c:(%LG,%LG)\n",
          c->x,c->y);
#endif  
  //keep a copy of original coordinate pair values around
  ccopy = *c;
  
  //compute sum of this function's associated variations
  //TODO: at some point may want to consider threads here???? probably not worth
  //it
  
  //clear actual storage space for sum
  c->x = 0.0;
  c->y = 0.0;
  //loop through this function's associated variations
  for(j=0; j<func->nv; j++){
    //if coefficient for this variation is zero, no reason to compute it!
    if(v_coeff[j] != 0.0){
      //variations modify coordinates, so make a copy of the copy :)
      ctemp = ccopy;
      //run the variation!
      if(!run_v(&func->v[j], &ctemp, &func->f.fp)){
        fprintf(stderr,"run_f: run_v failed.  returning...\n");
        return 0;
      }
#if defined(DEBUG)
      fprintf(stderr,"run_f: inside variation loop. j: %d, ctemp:(%LG,%LG)\n",
             j,ctemp.x,ctemp.y);
      fprintf(stderr,"run_f: c before additions: (%LG,%LG)\n",c->x,c->y);
#endif
      //scale result by this function's coefficient for this variation and add
      //to sum
      c->x += func->v_coeff[j] * ctemp.x;
      c->y += func->v_coeff[j] * ctemp.y;
#if defined(DEBUG)
      fprintf(stderr,"run_f: inside variation loop. j: %d, c:(%LG,%LG)\n",
             j,c->x,c->y);
#endif
    }
  }
#if defined(DEBUG)
  fprintf(stderr,"run_f: past variations. c:(%LG,%LG)\n",c->x,c->y);
#endif
  //linear post transformation associated with this function
  if(!LINEAR(func->p, c)){
    fprintf(stderr,"run_f: linear post transformation failed.  returning...\n");
    return 0;
  }
#if defined(DEBUG)
  fprintf(stderr,"run_f: past second linear transformation. c:(%LG,%LG)\n",
          c->x,c->y);
#endif

  //if all went well, return TRUE
  return 1;
}

//public

//function: init_functions
//purpose: initialize function lists, variations, etc.  ready to run after this!
//         TODO: find all the constants somewhere reasonable, rather
//         than writing them into this function.
//returns number of functions loaded on success, 0 on failure
extern int init_functions(int _nframes){
  int i,j;
  F bigf;
  F_func first;
  F_func post;
  float ci_scale;
  //test with linear functions for something like a sierpinski gasket
                    //upper-right quadrant
  F_params fp[] = { {  0.5,  0.0,  0.0,  0.0,  0.5,  0.0 },
                    {  0.5,  0.0,  0.5,  0.0,  0.5,  0.0 },
                    {  0.5,  0.0,  0.0,  0.0,  0.5,  0.5 },
                    //upper-left quadrant
                    { -0.5,  0.0,  0.0,  0.0,  0.5,  0.0 },
                    { -0.5,  0.0, -0.5,  0.0,  0.5,  0.0 },
                    { -0.5,  0.0,  0.0,  0.0,  0.5,  0.5 },
                    //lower-left quadrant
                    { -0.5,  0.0,  0.0,  0.0, -0.5,  0.0 },
                    { -0.5,  0.0, -0.5,  0.0, -0.5,  0.0 },
                    { -0.5,  0.0,  0.0,  0.0, -0.5, -0.5 },
                    //lower-right quadrant
                    {  0.5,  0.0,  0.0,  0.0, -0.5,  0.0 },
                    {  0.5,  0.0,  0.5,  0.0, -0.5,  0.0 },
                    {  0.5,  0.0,  0.0,  0.0, -0.5, -0.5 }
                  }; 
  
  //set up variations  
  nv = init_variations();
  
  if(!nv){
    printf("init_functions: init_variations failed... cannot continue\n");
    return 0;
  }
  //get list of variation functions
  variations = get_variations();
  
  //initialize animation parameters
  nframes = _nframes;
  dv_coeff = 0.3;
  
  //specify list of complete functions
  nfunctions = 9;
  functions = malloc(sizeof(bigf) * nfunctions);
  
  //can use the same v_coeff for all cases right now, since we're just 
  //zeroing it
  v_coeff = malloc(sizeof(coord_t) * nv);
  for(j=0; j<nv; j++){
    v_coeff[j] = 0.0;
  }
  
  //set up scaling factor so color indices are evenly distributed among
  //functions
  ci_scale = 1.0/(nfunctions-1);
  
  //fill F structs in functions array
  post.f = &identity_transformation;
  weight_vector_len = 0.0;
  //don't need to fill post.fp since we aren't using it
  for(i=0; i<nfunctions; i++){
    //initial linear transformation
    first.f = &linear_transformation;
    first.fp = fp[i];
    functions[i].f = first;
    
    //variations and weights
    functions[i].v = variations;
    functions[i].v_coeff = v_coeff;
    functions[i].nv = nv;
    
    //linear post transformation
    functions[i].p = post;
    
    //these aren't used yet
    //color
    functions[i].c = ci_scale*i;
    
    //probabilistic function weight
    functions[i].w = 1.0;
    functions[i].startw = weight_vector_len;
    weight_vector_len += functions[i].w; 
  }
  
  //set up final nonlinear transformation (set up in init_variations since it's
  //nonlinear)
  final = get_final();
  cfinal = 0.5; //make final transform the middle color for no particular reason
  
  return nfunctions;
}

//function: cleanup_functions
//purpose: free whatever needs freeing, etc. don't try to run after calling 
//         this!
//returns TRUE on success, FALSE on failure
extern int cleanup_functions(){
  //int i;
  
  //let this take care of memory init_variations allocated
  cleanup_variations();
  
  printf("cleanup_functions: about to free\n");
  
  /*
  //free anything allocated in init_functions that's specific to each function
  for(i=0; i<nfunctions; i++){
    free(functions[i].v_coeff);
  }
  */
  //only using 1 variational coefficient vector at the moment
  free(v_coeff);
  
  free(functions);
  
  return 1;
} 

//function: set_frame
//purpose: create animation by messing around with variation weights
//TODO: figure out how animations are actually done and write something less
//      ad hoc.
extern int set_frame(int t){
  static float y;
  float x, s,ss,ssd, c,cc,ccd;
  x = ((float)t)/nframes*M_PI;
  y = ((float)t)/(3*nframes)*M_PI + 1;
  //take advantage of sin^2 + cos^2 = 1
  s = sinf(x);
  c = cosf(x);
  ss = s*s;
  cc = c*c;
  ssd = ss*dv_coeff;
  ccd = ss*dv_coeff;
   
  //actual nonzero coefficient list
  v_coeff[0] = 1.0 - s*dv_coeff;
  v_coeff[1] = sinf(y)*dv_coeff;
  v_coeff[3] = ssd;
  v_coeff[2] = ccd;
  
  return 1; 
}

//function: run_function
//purpose: invoke linear function, grab associated color index
//params: vector_pos - random floating-point value used to select the function.
//                     this allows functions to have different probabilistic
//                     weights.
//        c - input coordinates for function
//        ci - current color index
extern int run_function(float vector_pos, coords * c, float * ci){
  //TODO
  //figure out some clever constant-time way to index from vector_pos to some
  //Fi.  maybe build an array in init that divides the range up into distinct
  //chunks of size=(gcd weights) populated with pointers to appropriate Fis?
  
  //for now, linear search
  int i;
  for(i=0; i<=nfunctions-1; i++){
    if(functions[i].startw <= vector_pos && vector_pos < functions[i+1].startw){
      *ci = functions[i].c;
      return run_f(&functions[i], c);
    }
  }
  //check last one
  if(functions[i].startw <= vector_pos && 
     vector_pos <= (functions[i].startw + functions[i].w)){ 
    *ci = functions[i].c;  
    return run_f(&functions[i], c);
  }
  
  //otherwise, vector_pos doesn't correspond to a function
  return 0;
}

//function: run_final
//purpose: run final transformation
extern int run_final(coords * c, float * _cfinal){
  *_cfinal = cfinal;
  return run_v(final, c, finalfp);
}


//accessors

//function: get_weight_vector_len
//purpose: engine needs this to define range for random function selection value
extern float get_weight_vector_len(){
  return weight_vector_len;
}

