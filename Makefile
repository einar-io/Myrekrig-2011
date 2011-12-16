# ANTFILES = $(wildcard Racer/*.c)
ANTFILES = ${shell perl -ne 'if (/^UseAnt\(([^\)]*)\)/ && ! $$e{$$1}) { print "Racer/$$1.c "; $$e{$$1} = 1; }' MyreHold.c}

CC = gcc
CFLAGS = -fsigned-char -Wall -O3 -I. $(DEBUG)
DEBUG = $(if $(findstring debug, $(MAKECMDGOALS)),-ggdb,-DNDEBUG)

MAIN = MyreKrig.o MyreHold.o
SYS = MK_Quiet.o MK_Ascii.o MK_Count.o MK_XWin.o
ANTS = $(ANTFILES:%.c=%.o)

MAINDEP = MyreKrig.h Myre.h
SYSDEP = MyreKrig.h Myre.h
ANTSDEP = Myre.h

OBJS = $(MAIN) $(SYS) $(ANTS)

all:		mk mk_a mk_c mk_x

mk:		$(MAIN) $(ANTS) MK_Quiet.o
		$(CC) $+ -o $@

mk_a:		$(MAIN) $(ANTS) MK_Ascii.o
		$(CC) $+ -o $@

mk_c:		$(MAIN) $(ANTS) MK_Count.o
		$(CC) $+ -o $@

mk_x:		$(MAIN) $(ANTS) MK_XWin.o
		$(CC) $+ -L/usr/X11R6/lib -lX11 -o $@

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
