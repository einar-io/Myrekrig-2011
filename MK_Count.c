#include <stdio.h>

#include "MyreKrig.h"

int SysGameInit(int argc, char *argv[]) { return 1; }
int SysBattleInit() { return 1; }

void SysDrawMap() {
   fprintf(stderr,"%5d\033[5D", CurrentTurn);
   fflush(stderr);
}

int SysCheck() { return 0; }
void SysBattleExit() {}
void SysGameExit() {}
void SysSquareChanged(int x, int y) {}

