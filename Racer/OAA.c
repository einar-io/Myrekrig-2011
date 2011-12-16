#include "Myre.h"
#define absv(a) ((a) >= 0 ? (a) : -(a))
#define val(x,y) (absv(x)+absv(y))
#define min(a,b) ((a)>(b) ? (b) : (a))
#define random(t) ((mem->randval=(mem->randval>>3)-(mem->randval<<13)+1) % (t))
// experimential
#ifndef CHICKENPITCH
#define CHICKENPITCH 150 //115
#define NORMALPITCH 180 //160
#define COURAGEOUSPITCH 220 //180
#endif
#define PITCH val(mem->posx, mem->posy) < 35 ? NORMALPITCH : CHICKENPITCH
#define ADVENTURERFORMULA val(mem->posx, mem->posy) < 35 ? COURAGEOUSPITCH : NORMALPITCH
#define REPORTFORMULA felt->NumFood-1
#define RADIUS 50 
#define CONSOLIDATION 12 //15 test RADIUS / 4
#define NOSENTINELS 10
#define CW 64
#define SOLDIER 32
#define CARRYING 8
#define RETURNEE 16
#define ADVENTURER 128
#define AT_HOME (!(mem->posx) && !(mem->posy))
#define SOLDIER_ADVANCE_DIRECTION mem->stat&7
#define SET_SOLDIER_ADVANCE_DIRECTION(d) (mem->stat=(mem->stat&(255-7))|(d))
#define SOLDIER_ADVANCE_SPEED 10
#define ISBIT(b) (mem->stat&(b))
#define SETBIT(b) (mem->stat|=(b))
#define CLRBIT(b) (mem->stat&=(255-(b)))
#define _ISBIT(b,i) (mem[(i)].stat&(b))
#define _SETBIT(b,i) (mem[(i)].stat|=(b))
#define _CLRBIT(b,i) (mem[(i)].stat&=(255-(b)))
#define REVERT SETBIT(RETURNEE); if (ISBIT(CW)) CLRBIT(CW); else SETBIT(CW)

struct OAABrain {
  unsigned short randval;
  signed char reportx;
  signed char reporty;
  signed char posx;
  signed char posy;
  unsigned char reportQty;
  unsigned char stat;
    // antal madstykker observeret eller hørt om UDOVER det vi evt. har taget med
    // ell. antal soldater at rekruttere
};

int OAA(struct SquareData *felt, struct OAABrain *mem);
int OAAMove(struct SquareData *felt, struct OAABrain *mem);
int OAAgo_to(signed char tox, signed char toy,struct OAABrain *mem );
u_long Random(int tal, u_long *randval);

DefineAnt(OAA, "OAA", OAA, struct OAABrain);

int OAA(struct SquareData *felt, struct OAABrain *mem) {
  int retval;
  retval = OAAMove(felt, mem);
  if (felt[retval&7].NumAnts >= MaxSquareAnts) {
      if (retval&1) retval+=2-2*(retval&2);
      else retval+=2-(retval & 4);
      CLRBIT(CARRYING);
  }
  mem->posx+=((retval&7)==1)-((retval&7)==3);
  mem->posy+=((retval&7)==4)-((retval&7)==2);
  if (mem->posx >= RADIUS) {mem->posx = -RADIUS; REVERT;}
  else if (mem->posx < -RADIUS) {mem->posx = RADIUS-1; REVERT;}
  else if (mem->posy >= RADIUS) {mem->posy = -RADIUS; REVERT;}
  else if (mem->posy < -RADIUS) {mem->posy = RADIUS-1; REVERT;}
  return retval | ISBIT(CARRYING);
}

int OAAMove(struct SquareData *felt, struct OAABrain *mem) {
  unsigned short i, temp,temp2;
  temp = 0; temp2=0;
  for (i=1; i<5; i++) 
    if (felt[i].Team && felt[i].NumAnts > temp2) {
      temp2=felt[i].NumAnts;
      temp = i;
    }
  if (temp) { // set fjende
    if (!ISBIT(SOLDIER)) {
      if (!mem->posx && !mem->posy) SET_SOLDIER_ADVANCE_DIRECTION(0);
      else if (!mem->posx) SET_SOLDIER_ADVANCE_DIRECTION (mem->posy>0 ? 4 : 2);
      else if (!mem->posy) SET_SOLDIER_ADVANCE_DIRECTION (mem->posx>0 ? 1 : 3);
      else SET_SOLDIER_ADVANCE_DIRECTION((absv(mem->posx) >= absv(mem->posy)) ? (mem->posx>0 ? 1 : 3) : (mem->posy>0 ? 4 : 2));
      SETBIT(SOLDIER);
    }
    return temp;
  }
  
  if(mem->reportQty) //rapport
    for (i=felt->NumAnts-1;i;i--) 
      if (!(mem[i].reportQty)) {
	mem[i].reportx = mem->reportx; mem[i].reporty = mem->reporty;
	mem[i].reportQty = mem->reportQty-mem->reportQty/2;
	mem->reportQty-=mem[i].reportQty;
	break;
      }
  
  if (!AT_HOME) {
    for (i=felt->NumAnts-1;i;i--)  // og hvis ikke hjemme frigiv yderligere alle soldater HVIS vi SELV har en rapport
      if (!random(4))
	_CLRBIT(SOLDIER,i);
  }
  else {
    if (felt->NumAnts < NOSENTINELS) {
      SETBIT(SOLDIER);
    }
    if (ISBIT(SOLDIER)) return 16;
    else {
      if (!felt->Base) {
	if (felt->NumFood>=NewBaseFood-NewBaseAnts) return 16;
      }
      else {
	mem->randval+=(mem->reportx | mem->reporty<<8 + mem->reportQty);
	mem->stat=random(4)*CW;
	if (mem->reportQty)
	  return (mem->reporty>0) ? 4 : 2;
	else return random(2)+2;
      }
    }
  }
  
  if (ISBIT(SOLDIER)) {
    if (random(1000)<SOLDIER_ADVANCE_SPEED) return SOLDIER_ADVANCE_DIRECTION;
    return 0;
  }
  
  if (ISBIT(CARRYING)) return OAAgo_to(0, 0, mem);
  
  if (mem->reportQty) {
    if (mem->posx == mem->reportx && mem->posy == mem->reporty) {
      if (felt->NumFood) {
	mem->reportQty--;	
	SETBIT(CARRYING);
	return OAAgo_to(0, 0, mem);
      }
      else mem->reportQty = 0;
    }
    else  // gå mod målet
      return OAAgo_to(mem->reportx, mem->reporty, mem);
  }
  
  // tilfældig vandrer (bonde)
  if (felt->NumFood>=felt->NumAnts) { // ramlet ind i mad (og ikke på en would-be base)
    mem->reportx=mem->posx;
    mem->reporty=mem->posy;
    mem->reportQty = REPORTFORMULA;
    SETBIT(CARRYING);
    return OAAgo_to(0, 0, mem);
  }
  else { // se omkring efter mad
    for (i=1; i<5; i++) 
      if (felt[i].NumFood>felt[i].NumAnts) {
	return i;
      }
  }
  
  if (ISBIT(RETURNEE) && val(mem->posx, mem->posy) < CONSOLIDATION) CLRBIT(RETURNEE);
  temp = ISBIT(ADVENTURER) ? ADVENTURERFORMULA : PITCH;
  if (ISBIT(CW)) {
    if (!mem->posx) return mem->posy>0 ? 1 : 3;
    if (!mem->posy) return mem->posx>0 ? 2 : 4;
    if (mem->posy > absv(mem->posx))
      return (random(1000) < temp) ? (ISBIT(RETURNEE) ? 2 : 4) : 1;
    if (mem->posx >= absv(mem->posy)/* || (mem->posx==-mem->posy && mem->posx>0)*/)
      return (random(1000) < temp) ? (ISBIT(RETURNEE) ? 3 : 1) : 2;
    if ((-mem->posy) > absv(mem->posx))
      return (random(1000) < temp) ? (ISBIT(RETURNEE) ? 4 : 2) : 3;
    return (random(1000) < temp) ? (ISBIT(RETURNEE) ? 1 : 3) : 4; 
  }
  else {
    if (!mem->posx) return mem->posy>0 ? 3 : 1;
    if (!mem->posy) return mem->posx>0 ? 4 : 2;
    if (mem->posy > absv(mem->posx)) 
      return (random(1000) < temp) ?(ISBIT(RETURNEE) ? 2 : 4) : 3;
    if (mem->posx >= absv(mem->posy)/* || (mem->posx==-mem->posy && mem->posx>0)*/)
      return (random(1000) < temp) ? (ISBIT(RETURNEE) ? 3 : 1) : 4;
    if ((-mem->posy) > absv(mem->posx))
      return (random(1000) < temp) ? (ISBIT(RETURNEE) ? 4 : 2) : 1;
    return (random(1000) < temp) ?(ISBIT(RETURNEE) ? 1 : 3) : 2; 
  }
}

int OAAgo_to (signed char tox, signed char toy, struct OAABrain *mem) {
  if (mem->posx==tox) return (mem->posy>toy) ? 2 : 4; 
  if (mem->posy==toy) return (mem->posx>tox) ? 3 : 1;
  if (absv(mem->posx-tox) > absv(mem->posy-toy)) 
    return (mem->posx>tox) ? 3 : 1;
  return (mem->posy>toy) ? 2 : 4;
}
#undef absv
#undef val
#undef min
#undef random
#undef CHICKENPITCH
#undef NORMALPITCH
#undef COURAGEOUSPITCH
#undef PITCH
#undef ADVENTURERFORMULA
#undef REPORTFORMULA
#undef RADIUS
#undef CONSOLIDATION
#undef NOSENTINELS
#undef CW
#undef SOLDIER
#undef CARRYING
#undef RETURNEE
#undef ADVENTURER
#undef AT_HOME
#undef SOLDIER_ADVANCE_DIRECTION
#undef SET_SOLDIER_ADVANCE_DIRECTION
#undef SOLDIER_ADVANCE_SPEED
#undef ISBIT
#undef SETBIT
#undef CLRBIT
#undef _ISBIT
#undef _SETBIT
#undef _CLRBIT
#undef REVERT
