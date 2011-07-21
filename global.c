#include <stdlib.h>
#include "global.h"
#include "functions.h"
#include "display.h"
#include "engine.h"

//MASTER DESTRUCTOR!!

extern int master_cleanup(){
  int ret = 1;
  
  ret &= cleanup_functions();  //calls cleanup_variations()
  ret &= cleanup_display(); //calls cleanup_color_palette()
  ret &= cleanup_engine();
  
  return ret;
}
