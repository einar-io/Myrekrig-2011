#include <stdio.h>
#include <stdlib.h>

#include "MyreKrig.h"

static char *map;

int SysGameInit(int argc, char *argv[]) { return 1; }

int SysBattleInit() {
  int i,j;
  map = calloc((Used.MapWidth+4)*Used.MapHeight-3,1);
  if(map) {
    char *ptr = map;
    for(j = 0 ; j < Used.MapHeight ; j++) {
      for(i = 0 ; i < Used.MapWidth ; i++) {
        SysSquareChanged(i,j);
      }
      ptr += Used.MapWidth;
      if (j < Used.MapHeight-1) {
	*ptr++ = '\033';
	*ptr++ = '[';
	*ptr++ = 'K';
	*ptr++ = '\n';
      } else {
	*ptr++ = 0;
      }
    }
  }
  return 1;
}

void SysDrawMap() {
  fprintf(stderr,"\033[H%s    %d\033[J",map,CurrentTurn);
  fflush(stderr);
}

int SysCheck() { return 0; }

void SysBattleExit() {
  if(map) {
    free(map);
    map = 0;
  }
}

void SysGameExit() {}

void SysSquareChanged(int x, int y) {
  struct SquareData felt = MapDatas[x+y*Used.MapWidth];
  map[x+y*(Used.MapWidth+4)] = (felt.Base ? '#'
                           : felt.NumAnts ? felt.Team+(felt.NumFood>0 ? 'A' : 'a')
                           : felt.NumFood ? '*' : ' ');
} 


