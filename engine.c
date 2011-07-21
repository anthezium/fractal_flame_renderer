/* Author: Ted Cooper
 * Last revised: 7-2-2009
 * FRACTAL FLAME RENDERER
 * This program renders "Fractal Flame" images as described in Scott Draves'
 * paper "The Fractal Flame Algorithm" (http://flam3.com/flame.pdf) and 
 * popularized in the Electric Sheep screensaver 
 * (http://electricsheep.org).  My eventual goal is to support all features 
 * described in the paper and use the XML input files flam3-render takes as
 * input (http://electricsheep.wikispaces.com/flam3-render), but currently
 * only a subset of the rendering steps and variation functions are 
 * implemented, flame specifications are hardcoded in various  
 * unintuitive places, and no image or video file output is supported - it just
 * renders and displays an animation.  I also need to pull the animation
 * stuff out of this file too and write a separate animator analogous to
 * flam3-animate. 
 *
 * engine.c: initialization and main rendering/display loop.  Currently 
 * renders a simple animation with NFRAMES frames and plays it on loop. 
 */
 
//INCLUDES (INCLUSIONS?)

#include <GL/glut.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include "global.h"
#include "display.h"

//GLOBALS

#define MINV -1.0
#define RANGE 2.0
//these should eventually all be specified in a file or on the command line
#define MINITERATIONS 20
#define NITERATIONS 20000000
#define WINW 800
#define WINH 600
#define GAMMA 4.0
#define VIBRANCY 0.6
#define NFRAMES 100
#define FRAME_PERIOD 30

//MACROS

//random floating-point value in range [0.0,1.0)
#define RANDD (rand()/(RAND_MAX + 1.0))

//random floating-point value in range [MINV, (RANGE + MINV))
#define RANDU (RANDD * RANGE + MINV)

//FORWARD DECLARATIONS

int render(int niterations, int miniterations, float vector_len, int t);

//MAIN

//function: main
//purpose: runs initializations and outermost loops for rendering and display.
//         exit status 1 on failure, 0 on success.
int main(){

  int t;

  //initializations

  printf("main: before function initialization\n"); 

  if(!init_functions(NFRAMES)){
    fprintf(stderr,"main: init_functions failed.  exiting...\n");
    return 1;
  }
  
  printf("main: past function initialization\n");
  
  if(!init_display(WINW, WINH, MINV, MINV, RANGE, RANGE, NFRAMES, FRAME_PERIOD)){
    fprintf(stderr,"main: init_display failed.  exiting...\n");
    return 1;
  }
  
  printf("main: past display initialization\n");
  
  //rendering
  
  //render frames
  for(t=0; t<NFRAMES; t++){    
    //start rendering loop for the tth frame
    render(NITERATIONS, MINITERATIONS, get_weight_vector_len(), t);
  }
  
  printf("main: past rendering loops\n");
  
  //display
  
  //start display loop
  start_display(GAMMA, VIBRANCY);
  
  //cleanup (these will never actually get called here unless the glutMainLoop 
  //call somehow fails)
  master_cleanup();  //from global.c
  
  return 1;
}

//DESTRUCTOR

//function: cleanup_engine
//purpose: doesn't do much at the moment.  will make more sense when this file
//         no longer has main().
extern int cleanup_engine(){
  return 1;
}

//RENDERING FUNCTIONS

//function: render
//purpose: render a single fractal flame image using Draves' random walk loop.
//         functions and display need to be initialized before this is called.
//params: niterations - number of times to run the random walk.  the more the 
//        better, generally, if you have time to kill.
//        miniterations - minimum number of times the random walk must be run 
//        before its results can be considered meaningful and plotted.  systems
//        are supposed to be "contractive on average," so I guess this makes 
//        sense as a mechanism for getting within a reasonable range.
//        vector_len - floating-point length of the vector used to randomly
//        select a function to run.
//        t - frame number in animation.  just need this to tell display's 
//        plot() where to put image data in array of frames.
int render(int niterations, int miniterations, float vector_len, int t){

  int i,outside;
  float vector_pos;

  coords p;
  float c, ci, cfinal, cf;
  struct timeval now; 
  
  if(niterations <= miniterations){
    fprintf(stderr,"render: Rendering won't work unless n > %d. returning...\n", 
            miniterations);
    return 0; //return FALSE  
  }
  
  //seed the random number generator with the time
  gettimeofday(&now, NULL);
  srand(now.tv_usec * now.tv_sec);
  
  //fill vars with random values
  p.x = (coord_t)RANDU;
  p.y = (coord_t)RANDU;
  c = (float)RANDD;
  
  //initialize count of points outside the range the algorithm attempts to plot
  outside = 0;
  
  //set current frame in animation
  set_frame(t);
  //MAIN LOOP
  for(i=0; i<niterations; i++){
#if defined(DEBUG)
    fprintf(stderr,"render: top of main loop.  p:(%LG,%LG)\n",p.x,p.y);
#endif
    //get random function index
    vector_pos = RANDD; //between 0.0 and 1.0
    vector_pos = vector_len*vector_pos; //between 0.0 and vector_len 
    
    //comments follow steps in loop outline on p.9 in Draves' paper
    
    //p = Fi(p) (run initial linear transformation)
#if defined(DEBUG)
    fprintf(stderr,"render: about to call run_function\n");
#endif
    run_function(vector_pos, &p, &ci);
    
    //c = (c + ci)/2 (average color index with current function's color index)
    c = (c + ci)/2.0;
    
    //pf=Ffinal(p) (run final linear transformation)
#if defined(DEBUG)
    fprintf(stderr,"render: about to call run_function\n");
#endif
    run_final(&p, &cfinal);
    
    //cf = (c + cfinal)/2; (average color index with final function's color
    //                      index)
    cf = (c + cfinal)/2.0;
 
    //plot (pf,cf) to image except during the first MINITERATIONS iterations
    if(i >= miniterations){
#if defined(DEBUG)
      fprintf(stderr,"render: calling plot on (%LG, %LG, %G)\n", p.x, p.y, cf);
#endif
      //attempt to plot the current point with the current color.
      //increment out-of-range plot attempt count if point is out of range for
      //display.  again, since systems are "contractive on average" this happens
      //sometimes and shouldn't be considered a problem, but if this number 
      //grows significant relative to the total number of plot attempts, image
      //quality and detail will suffer.
      //TODO: figure out a way to quantify that and check it...
      if(!plot(&p, &cf, t))
        outside++;
    }
  }
  
  //printf("render: rendering complete.  %d/%d points were outside the range\n",
  //       outside, niterations-miniterations);
  

  return 0;

}
