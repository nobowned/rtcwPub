CC = gcc
CFLAGS = -O3 -DGAMEDLL
DCFLAGS = -g -DGAMEDLL

SOURCES = $(shell ls g_*.c q_*.c ai_*.c bg_animation.c bg_misc.c bg_pmove.c bg_slidemove.c ../botai/*.c)
MODULES = $(SOURCES:.c=.o)

all: gameclean qagame.mp.i386.so

gameclean:
	rm -rf *.o


qagame.mp.i386.so: $(MODULES)
	$(CC) -s -shared -Wl,-V -Wl,-soname,$@ -o $@ $?


debug: CFLAGS := \
	$(DCFLAGS)
debug: $(MODULES)
	$(CC) -shared -Wl,-soname,qagame.mp.i386_debug.so -o qagame.mp.i386_debug.so $?


.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so ../botai/*.so ../botai/*.o


