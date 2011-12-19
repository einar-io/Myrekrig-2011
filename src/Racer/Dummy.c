//Lavet af Jesper Torp Kristensen, u993197.

#include "Myre.h"

#define AntMem struct DummyBrain

#define INITIAL_RADIUS 40
#define RADIUS_STEP 30

#define getRand(mem) (((mem->randseed = mem->randseed*1103515245+12345)>>16) & 65535)
#define destReached(mem) (mem->pos.pos == mem->dest.pos)

#define gotoAdjSqr(mem, r) {\
  if (r&1) {\
    if (r==1) {\
      mem->pos.xy.x++;\
      return 1;\
    } else {\
      mem->pos.xy.x--;\
      return 3;\
    }\
  } else {\
    if (r==4) {\
      mem->pos.xy.y++;\
      return 4;\
    } else {\
      mem->pos.xy.y--;\
      return 2;\
    }\
  }\
}

#define gotoDest(mem) {\
  if (mem->pos.xy.x != mem->dest.xy.x) {\
    if (mem->pos.xy.x < mem->dest.xy.x) {\
      mem->pos.xy.x++;\
      return 1 | mem->hasFood;\
    } else {\
      mem->pos.xy.x--;\
      return 3 | mem->hasFood;\
    }\
  } else {\
    if (mem->pos.xy.y < mem->dest.xy.y) {\
      mem->pos.xy.y++;\
      return 4 | mem->hasFood;\
    } else {\
      mem->pos.xy.y--;\
      return 2 | mem->hasFood;\
    }\
  }\
}

#define ROOKIE 0
#define SOLDIER 1

union DummyPos{
  struct{
    short x,y;
  } xy;
  int pos;
};

struct DummyBrain{
  u_long randseed;
  union DummyPos pos,dest,food;
  unsigned short radius;
  char foodAmount;
  char rang;
  char hasFood;//0 eller 8
};

int Dummy(struct SquareData *felt, AntMem *mem) {
  int i;
  //for (i=0; i<100000; i++) ;
  for (i=1; i<5; i++) if (felt[i].Team) {
    mem->rang = SOLDIER;
    gotoAdjSqr(mem, i);
  }
  if (mem->rang)
    if (getRand(mem)&255) {
      return 0;
    } else {
      mem->rang = ROOKIE;
    }
  if (!mem->pos.pos) {
    if (mem->foodAmount) {
      for (i=felt->NumAnts-1; i>0; i--) if (mem[i].foodAmount && (mem[i].food.pos == mem->food.pos) && (mem[i].foodAmount < mem->foodAmount)){
	mem->foodAmount = 0;
	break;
      } else return 0;
    }
    mem->hasFood = 0;
    for (i=felt->NumAnts-1; i>0; i--) if (mem[i].foodAmount) {
      mem->dest.pos = mem[i].food.pos;
      mem[i].foodAmount--;
      gotoDest(mem);
    }
    if (mem->radius) mem->radius+=RADIUS_STEP; else mem->radius=INITIAL_RADIUS;
    if (getRand(mem)&1) {
      mem->dest.xy.x = (getRand(mem)%(mem->radius<<1)) - mem->radius;
      mem->dest.xy.y = (mem->randseed&2 ? mem->radius : -mem->radius);
    } else {
      mem->dest.xy.x = (mem->randseed&2 ? mem->radius : -mem->radius);
      mem->dest.xy.y = (getRand(mem)%(mem->radius<<1)) - mem->radius;
    }			
    gotoDest(mem);
  }
  if (mem->hasFood) gotoDest(mem);
  if (felt->NumFood >= felt->NumAnts) {
    mem->hasFood = 8;
    if (mem->dest.pos != mem->pos.pos) mem->foodAmount = felt->NumFood-1;
    mem->dest.pos = 0;
    mem->food.pos = mem->pos.pos;
    gotoDest(mem);
  }
  for (i=1; i<5; i++) if (felt[i].NumFood > felt[i].NumAnts) gotoAdjSqr(mem, i);
  //printf("%d %d\n", mem->dest.pos, mem->pos.pos);
  if (destReached(mem)) mem->dest.pos = 0;
  gotoDest(mem);
}

DefineAnt(Dummy, "Dummy", Dummy, struct DummyBrain)
