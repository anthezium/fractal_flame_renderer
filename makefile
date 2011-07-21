CC=gcc -Wall -UDEBUG

FLAGS = -I/usr/include
LIBDIRS = -L/usr/X11R6/lib
LIBS = -lGLU -lGL -lglut -lXmu -lXext -lX11 -lXi -lm

OBJECTS = engine.o display.o functions.o variations.o colorpalette.o global.o

all: $(OBJECTS)
	$(CC) $(FLAGS) -o engine $(OBJECTS) $(LIBDIRS) $(LIBS)

engine.o: engine.c engine.h
	$(CC) -c engine.c
	
functions.o: functions.c functions.h variations.o variations.h
	$(CC) -c functions.c

variations.o: variations.c variations.h
	$(CC) -c variations.c
	
display.o: display.c display.h
	$(CC) -c display.c 
	
colorpalette.o: colorpalette.c colorpalette.h
	$(CC) -c colorpalette.c 
	
global.o: global.c global.h
	$(CC) -c global.c 

clean:
	rm -f *.o engine
