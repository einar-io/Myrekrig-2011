#include <stdio.h>

#include "MyreKrig.h"

bool SysGameInit(int argc, char *argv[]) { return 1; }
bool SysBattleInit(void) { return 1; }

void SysDrawMap(void) {
//   fprintf(stderr,"%5d\033[5D", CurrentTurn);
   fprintf(stderr,"\r%ul", CurrentTurn);
   fflush(stderr);
}

bool SysCheck(void) { return 0; }
void SysBattleExit(void) {}
void SysGameExit(void) {}
void SysSquareChanged(int x, int y) {}
