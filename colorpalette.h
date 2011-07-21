#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include "global.h"

typedef struct {
  color * colors;
  int ncolors;
} colorpalette;

//public

extern int init_color_palette();
extern int cleanup_color_palette();

extern color * lookup_color(float index);

#endif
