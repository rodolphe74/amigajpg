# The name of your C compiler:
CC= gcc

# You may need to adjust these cc options:
CFLAGS= -O3 -Ijpeg-6b  -I. -std=c99
# Generally, we recommend defining any configuration symbols in jconfig.h,
# NOT via -D switches here.

# Link-time cc options:
LDFLAGS= -lm -lSDL2

# To link any special libraries, add the necessary -l commands here.
LDLIBS= 

# miscellaneous OS-dependent stuff
# linker
LN= $(CC)
# file deletion command
RM= rm -f
# library (.a) file creation command
AR= ar rc
# second step in .a creation (use "touch" if not needed)
AR2= ranlib

COPY = cp
COPY_TO = /cygdrive/u/68000/uae/hd/bin

# End of configurable options.

all: amigajpg

Amiga_m68k_load_test: load_test.o jpeg-6b/libjpeg.a
	$(CC) $(CFLAGS) -I. $^ -o $@ $(LDFLAGS)
	
Amiga_m68k_load_test.o: load_test.c
	$(CC) $(CFLAGS) -c -o $@ $<
	


amigajpg: jpeg.o pixel.o powf.o log.o gopt.o gopt-errors.o jpeg-6b/libjpeg.a amigajpg.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

jpeg.o: jpeg.c
	$(CC) $(CFLAGS) -c $< -o $@

pixel.o: pixel.c
	$(CC) $(CFLAGS) -c $< -o $@
	
powf.o: powf.c
	$(CC) $(CFLAGS) -c $< -o $@

log.o: log.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
gopt.o: gopt.c gopt-errors.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
gopt-errors.o: gopt-errors.c
	$(CC) $(CFLAGS) -c -o $@ $<

	
	
clean:
	rm -f *.o *.a Amiga_m68k_load_test amigajpg
	
indent:
	uncrustify --replace -c /usr/share/uncrustify/linux.cfg *.c  *.h
	rm *unc-backup*

indent_clean:
	rm *unc-backup*
