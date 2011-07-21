#include "colorpalette.h"
#include <stdio.h>
#include <stdlib.h>

//stole from sheep 138022's color palette from genome file:
//http://sheepserver.net/v2d6/gen/202/138022/spex 
//(then did some string processing to make valid C syntax for this purpose.
//see palette_preprocessor.sh for conversion)
//a little manual intervention was involved in this process... grabbed the 
//palette section from the genome file, read the number of colors, and deleted 
//the last comma in the preprocessor output
extern color somecolors[] = {
#include "sheep_138022_color_palette_processed.inc"
                            };

extern colorpalette palette = { somecolors, 256 };


extern int init_color_palette(){
  return 1;
}
extern int cleanup_color_palette(){
  return 1;
}

//index is a float in [0.0, 1.0]
extern color * lookup_color(float index){
  int i = (int)(index*palette.ncolors);
  if(i < 0 || i >= palette.ncolors){
    printf("lookup_color: index out of bounds\n");
    exit(1);
  }
  return &palette.colors[i];
}
