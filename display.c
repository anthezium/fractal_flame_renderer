#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "display.h"
#include "functions.h"
#include "variations.h"
#include "colorpalette.h"

//private globals

static GLint winW, winH;
static coord_t minX, minY, rangeX, rangeY;

typedef unsigned int plotcount_t;

plotcount_t ** framecounts = NULL;

static int frame_period;
static int nframes;
static int dt;
static color_t ** frames = NULL;
static color_t * pixels = NULL;

//FUNCTIONS

//event handlers

#define EITHERCASE(key, c) (((key) == (c)) || ((key) == ((char)((c) - 32))))

void keyboard(unsigned char key, int mouseX, int mouseY) {
	//printf("received char %c, value %d\n", key, key);
	if(EITHERCASE(key, 'q')){
	  master_cleanup();
		exit(0);
	}
		
	//if keys were pressed, redraw	
	glutPostRedisplay();
	return;
}

//switch to next frame
void update(int t){
  //if we are at an end, switch direction
  if(t >= nframes - 1 || t <= 0)
    dt*=-1;
  
  t+=dt;
  
  pixels = frames[t];
  //reset timer
  glutTimerFunc(frame_period, update, t);
  //time to redraw
  glutPostRedisplay();
}

//initialization and cleanup

extern int init_display(int _winW, int _winH, 
                        coord_t _minX, coord_t _minY, 
                        coord_t _rangeX, coord_t _rangeY,
                        int _nframes, int _frame_period){
  int t;
  
  winW = _winW;
  winH = _winH;
  minX = _minX;
  minY = _minY;
  rangeX = _rangeX;
  rangeY = _rangeY;
  nframes = _nframes;
  frame_period = _frame_period;
  dt = 1; //start going forward
  
  //set up color palette
  init_color_palette();
  
  //allocate space for frames
  frames = malloc(sizeof(color_t *) * nframes);
  framecounts = malloc(sizeof(plotcount_t *) * nframes);
  
  for(t=0; t<nframes; t++){
    //we'll store counts and colors accumulated in plot() in these arrays
    frames[t] = malloc(sizeof(color_t) * winW * winH * 3);
    framecounts[t] = malloc(sizeof(plotcount_t) * winW * winH);
  }
  
  return 1;
}

extern int cleanup_display(){
  printf("cleanup_display: about to free\n");
  int i;
  
  for(i=0; i<nframes; i++){
    free(frames[i]);
    free(framecounts[i]);
  }
  free(frames);
  free(framecounts);
  
  //done with color palette
  cleanup_color_palette();
  
  return 1;
}

//plot points
//params coordinate pair, index into color palette
extern int plot(coords * p, float * c, int t){
  int x;
  int y;
  int i;
  color * ccolor;
  
#if defined(DEBUG)
  printf("plot: received p:(%LG,%LG)\n", p->x, p->y);
  printf("      minX: %LG, rangeX: %LG\n", minX, rangeX);
  printf("      minY: %LG, rangeY: %LG\n", minY, rangeY);
#endif

  x = (int)((p->x - minX)/rangeX * winW + 0.5);
  y = (int)((p->y - minY)/rangeY * winH + 0.5);
  
  //don't try to plot if out of range
  if(x < 0 || x >= winW || y < 0 || y >= winH){
    //printf("plot: coordinates (%d,%d) out of range.  not plotting.\n",x,y);
    return 0;
  }

#if defined(DEBUG)
  printf("plot: about to increment framecounts[t][%d]. (x,y):(%d,%d)\n",
         i, x, y);
#endif

  i = y*winW + x;

  //increment count where point is in grid
  framecounts[t][i]++;
  
  //look up color in palette using index
  ccolor = lookup_color(*c);

  //accumulate color values
  frames[t][3*i] += ccolor->r;
  frames[t][3*i+1] += ccolor->g;
  frames[t][3*i+2] += ccolor->b;
  
  return 1;
}

//display functions
//vibrancy [0.0,1.0], gamma somewhere ~[2.0,4.0]
static int compute_pixels(float gamma, float vibrancy, int t){
  int i;
  color_t r,g,b;
  float invgamma, compvib, alpha_gamma;
  GLfloat brightness, brightness_scale;
  GLfloat alpha, alpha_scale, max_alpha_scale, first_scale;
  
  printf("compute_pixels:  starting actual frame image computation\n");
  
  invgamma = 1.0/gamma;
  compvib = 1.0 - vibrancy;
  
  plotcount_t max = framecounts[t][0];
  
  //find largest count
  for(i=1; i<winW*winH; i++){
    if(framecounts[t][i] > max)
      max = framecounts[t][i];
  }
  /*
  //grayscale
  //solve for brightness_scale to fix max's log at 1.0
  brightness_scale = (GLfloat)1.0/(logf((float)max));
  
  //store scaled color for each pixel
  for(i=0; i<winW*winH; i++){
    brightness = brightness_scale*logf((float)framecounts[t][i]);
    
    //don't worry about color for the moment, just do grayscale
    frames[t][3*i] = brightness;
    frames[t][3*i+1] = brightness;
    frames[t][3*i+2] = brightness; 
  }
  */
  
  
  //color
  //try doing count-based log scale per channel like on p.10 of paper, then 
  //divide by log of largest count to ensure all values <= 1.0
  
  //solve for brightness_scale to fix max's log at 1.0
  max_alpha_scale = (GLfloat)1.0/(logf((float)max));
  
  for(i=0; i<winW*winH; i++){
  
    //this would create weird behavior
    if(M_E > framecounts[t][i]){
      frames[t][3*i] = 0.0;
      frames[t][3*i+1] = 0.0;
      frames[t][3*i+2] = 0.0;
      continue;
    }
  
    //basic color scaling
    alpha = (GLfloat)framecounts[t][i];    
    alpha_scale = (GLfloat)logf((float)alpha);
    brightness = alpha_scale*max_alpha_scale;
        
    //first scaling factor
    first_scale = brightness/alpha;
    
    
    //scale colors (already accumulated in pixels array) based on this pixel's
    //alpha and the entire image's max alpha
    if((frames[t][3*i] *= first_scale) > 1.0 ||
       (frames[t][3*i+1] *= first_scale) > 1.0 ||
       (frames[t][3*i+2] *= first_scale) > 1.0
      ){
      printf("compute_pixels: scaling isn't working right\n");
      exit(1);  
    }
    
    
    //gamma correction and vibrancy
    //vibrancy determines how much of gamma correction is determined by
    //alpha channel's brightness (as opposed to each individual channel's)
    alpha_gamma = vibrancy*powf(brightness, invgamma);
    
    frames[t][3*i] *= compvib*powf(frames[t][3*i], invgamma) + alpha_gamma;
    frames[t][3*i+1] *= compvib*powf(frames[t][3*i+1], invgamma) + alpha_gamma;
    frames[t][3*i+2] *= compvib*powf(frames[t][3*i+2], invgamma) + alpha_gamma;
    
    /*
    frames[t][3*i] = ( frames[t][3*i] > 1.0 ? 1.0 : frames[t][3*i]);
    frames[t][3*i+1] = ( g > 1.0 ? 1.0 : g);
    frames[t][3*i+2] = ( b > 1.0 ? 1.0 : b);
    */
    
    if(frames[t][3*i]   > 1.0 ||
       frames[t][3*i+1] > 1.0 ||
       frames[t][3*i+2] > 1.0
      ){
      printf("compute_pixels: gamma put channels out of range\n");
      //exit(1);  
    }
    
  }
  
  printf("compute_pixels:  finished frame %d image computation\n", t);
  
  
  return 1;
}

//display
void display(void) {

  //fill the framebuffer
  glDrawPixels(winW, winH, GL_RGB, GL_FLOAT, pixels);

  //frame buffer is complete, so move it to "front" for screen display
  glutSwapBuffers();

  //detect rendering errors (note that the call to glGetError() clears
  // the error condition too, so call it once and save the error code).
  int error = glGetError();
  if (error != GL_NO_ERROR)
    printf("OpenGL Error: %s\n", gluErrorString(error));
}


extern int start_display(float gamma, float vibrancy){
  int i=0;
  
  //initialize window
  glutInit(&i, NULL);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(winW, winH);
  glutInitWindowPosition(100,150);
  glutCreateWindow("Fractal Flame");
  glViewport(0, 0, winW, winH); //set size of viewport (in pixels)

  printf("start_display: before compute_pixels loop\n");
  
  //generate the images
  for(i=0; i<nframes; i++){
    compute_pixels(gamma, vibrancy, i);
  }
  
  printf("start_display: past compute_pixels loop\n");
  
  pixels = frames[1];
  
  //register callback functions for glut (other events exist too, if needed...)
  glutDisplayFunc(display);
  //glutMouseFunc(mouseclick);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(frame_period, update, 1);

  glutMainLoop(); //infinite event-driven loop (managed by glut)
  return 0;
}
