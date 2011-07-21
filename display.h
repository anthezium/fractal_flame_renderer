#ifndef DISPLAY_H
#define DISPLAY_H

#include <GL/glut.h>
#include "global.h"

//public
extern int init_display(int _winW, int _winH, 
                        coord_t _minX, coord_t _minY, 
                        coord_t _rangeX, coord_t _rangeY,
                        int _nframes, int _frame_period);
extern int cleanup_display();
extern int start_display(float gamma, float vibrancy);

extern int plot(coords * p, float * c, int t); 

#endif
