#ANTFILES = $(wildcard src/Racer/*.c)
ANTFILES = ${shell perl -ne 'if (/^UseAnt\(([^\)]*)\)/ && ! $$e{$$1}) { print "src/Racer/$$1.c "; $$e{$$1} = 1; }' src/MyreHold.c}

CC = gcc
CFLAGS =  -fsigned-char -Wall -O3 -I. $(DEBUG)
LDFLAGS = 
DEBUG = $(if $(findstring debug, $(MAKECMDGOALS)),-ggdb,-DNDEBUG)

MAIN = src/MyreKrig.o src/MyreHold.o
SYS = src/MK_Quiet.o src/MK_Ascii.o src/MK_Count.o src/MK_XWin.o
ANTS = $(ANTFILES:%.c=%.o)

MAINDEP = src/MyreKrig.h src/Myre.h
SYSDEP = src/MyreKrig.h src/Myre.h
ANTSDEP = src/Myre.h

OBJS = $(MAIN) $(SYS) $(ANTS)

all:		mk mk_a mk_c mk_x

mk:		$(MAIN) $(ANTS) MK_Quiet.o
		$(CC) $(LDFLAGS) $+ -o $@

mk_a:		$(MAIN) $(ANTS) MK_Ascii.o
		$(CC) $(LDFLAGS) $+ -o $@

mk_c:		$(MAIN) $(ANTS) MK_Count.o
		$(CC) $(LDFLAGS) $+ -o $@

mk_x:		$(MAIN) $(ANTS) MK_XWin.o
		$(CC) $(LDFLAGS) $+ -L/usr/X11R6/lib -lX11 -o $@

$(MAIN):	$(MAINDEP)
$(SYS):		$(SYSDEP)
$(ANTS):	$(ANTSDEP)

q:		mk
a:		mk_a
c:		mk_c
x:		mk_x

clean:
		rm -f $(OBJS)

.SILENT:	debug
debug:
		test dummy

.PHONY: all clean debug
