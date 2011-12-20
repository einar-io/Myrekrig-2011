#include "Myre.h"
#define printf(...)

/*
  Triumfant (Nomamyrmex Triumphans)
  af Martin Carlsen, mca@mip.sdu.dk
  
  En trehovedet mutantmyre med amnesitendenser og storhedsvanvid.
  Triumfant skulle oprindeligt have heddet Kvadrant, 
  men det gav ikke nok prestige, så hermed indledes trebyteæraen!
*/

struct Triumfant
{
  u_char state : 3;
  u_char oct   : 3; 
  u_char leg   : 2; 
  u_char pos   : 7; 
  u_char bit1  : 1; 
  u_char rad   : 6; 
  u_char bit2  : 2; 
};

enum { none=0, east=1, south=2, west=3, north=4 }; 
enum { ne=0, nw=1, sw=2, se=3 }; 
enum { en=4, wn=5, ws=6, es=7 }; 
enum { cw=0, fwd=1, ccw=2, rev=3 }; 

enum { Queen, Build, Return1, Fetch1, Search2, Return2, Fetch2, XXX,
       Init=8, Guard, Search1, Engineer }; 

static const char* stateNames[] = {
  "Queen", "Build", "Return1", "Fetch1", "Search2", "Return2", "Fetch2", "XXX",
  "Init", "Guard", "Search1", "Engineer"
};
static const char* dirNames[] = {
  "none", "east", "south", "west", "north"
};
static const char* octantNames[] = {
  "ne", "nw", "sw", "se", "en", "wn", "ws", "es"
};
static const char* rotationNames[] = {
  "cw", "fwd", "ccw", "rev"
};

static const u_char route[8][4] = {
  {north, east, south, west}, 
  {north, west, south, east}, 
  {south, west, north, east}, 
  {south, east, north, west}, 
  {east, north, west, south}, 
  {west, north, east, south}, 
  {west, south, east, north}, 
  {east, south, west, north}  
};

static const u_char octants[8][4] = {
  {ne, ne, en, en}, 
  {nw, nw, wn, wn}, 
  {sw, sw, ws, ws}, 
  {se, se, es, es}, 
  {en, en, ne, ne}, 
  {wn, wn, nw, nw}, 
  {ws, ws, sw, sw}, 
  {es, es, se, se}  
};

static const u_char returnRoute[8][2] = {
  {west, south}, 
  {east, south}, 
  {east, north}, 
  {west, north}, 
  {south, west}, 
  {south, east}, 
  {north, east}, 
  {north, west}  
};

static const char DX[5] = {0, 1,  0, -1, 0};
static const char DY[5] = {0, 0, -1,  0, 1};

enum { 
  npos = 0x00,
  ppos = 0x01,
  nrad = 0x10,
  prad = 0x11 
};

static const char octantXY[8][2] = {
  {ppos, prad}, 
  {npos, prad}, 
  {npos, nrad}, 
  {ppos, nrad}, 
  {prad, ppos}, 
  {nrad, ppos}, 
  {nrad, npos}, 
  {prad, npos}  
};


struct XY
{
  char x;
  char y;
};

struct HR
{
  char h;
  char r;
};

struct OR
{
  u_char oct;
  u_char rad;
  u_char pos;
};

#define ARGS struct SquareData* m, struct Triumfant* a
#define ANTS struct Triumfant* a
#define MAP  struct SquareData* m
#define CASE(X) case X: return Triumfant##X(m,a)
#define STATE(X) int Triumfant##X(ARGS)
#define GOTO return Triumfant(m,a)
#define KILLKILLKILL { int kill = findEnemy(m); if (kill) { setState(a,Guard); return kill; } }

int Triumfant(ARGS);

#define getX(a) ((char*)a)[1]
#define getY(a) ((char*)a)[2]
#define setX(a,x) ((char*)a)[1] = x;
#define setY(a,y) ((char*)a)[2] = y;
#define X ((char*)a)[1]
#define Y ((char*)a)[2]

inline static int getState(ANTS)
{
  if (a->state < 7) return a->state;
  return 8 + a->bit2;
}
inline static void setState(ANTS, int state)
{
  if (state<7) {
    a->state = state;
  }
  else {
    a->state = 7;
    a->bit2 = state - 8;
  }
}

inline static u_char getNum(ANTS)
{
  return (a->leg<<3) + (a->bit1<<2) + (a->bit2);
}
inline static void setNum(ANTS, u_char num)
{
  a->leg = num>>3;
  a->bit1 = num>>2;
  a->bit2 = num;
}

inline static int max(int a, int b) { return (a>b) ? a : b; }
inline static int min(int a, int b) { return (a<b) ? a : b; }
inline static int sign(int a) { return a<0 ? -1 : 1; }
inline static int abs(int a)  { return a<0 ? -a : a; }
inline static int rnd(ARGS, u_long num)
{
  int n = 0, i = 0;
  n += ((char*)&a[m->NumAnts-1])[0]
    +  ((char*)&a[m->NumAnts-1])[1]
    + (((char*)&a[m->NumAnts-1])[2]<<8);
  for (i=1; i<5; ++i)
    n += m[i].NumFood+m[i].NumAnts;
  
  return n%num;
}

#define COORXY(m,r,p) (((m)&0x10) ? (((m)&0x01 ? 1 : -1)*(r)) : ((m)&0x01 ? 1 : -1) * (p))
#define COORX(o,r,p) COORXY(octantXY[o][0],r,p);
#define COORY(o,r,p) COORXY(octantXY[o][1],r,p);

struct XY getXY(u_char oct, u_char leg, u_char rad, u_char pos)
{
  struct XY coor;
  int o = octants[oct][leg];
  if (leg==2) pos = rad-pos;
  
  coor.x = COORX(o,rad,pos);
  coor.y = COORY(o,rad,pos);
  
  return coor;
}

struct OR getOR(char x, char y)
{
  struct OR coor;
  
  coor.oct = 0;
  if (abs(x)>abs(y)) coor.oct += 4;
  if (y<0) coor.oct += 2;
  if (sign(x) != sign(y)) coor.oct += 1;
  coor.rad = (abs(x)>abs(y)) ? abs(x) : abs(y);
  coor.pos = (coor.oct < en) ? abs(x) : abs(y);
  
  return coor;
}

struct Triumfant* findAnt(ARGS, u_char state)
{
  struct Triumfant *i,*z;
  for (i=a,z=a+m->NumAnts; i!=z; ++i) {
    if (getState(i) == state) return i;
  }
  return 0;
}

int findFood(MAP)
{
  int i;
  for (i=1; i<5; ++i) {
    if (m[i].NumFood > m[i].NumAnts) return i;
  }
  return 0;
}

int findEnemy(MAP)
{
  int i;
  for (i=1; i<5; ++i) {
    if (m[i].Team) return i;
  }
  return 0;
}

STATE(Build)
{
  if (X>0) { --X; return 8 + west; }
  if (X<0) { ++X; return 8 + east; }
  if (Y>0) { --Y; return 8 + south; }
  if (Y<0) { ++Y; return 8 + north; }
  return 16;
}

STATE(Return1)
{
  struct Triumfant* q = findAnt(m,a,Queen);
  
  if (q) {
    if (!a->pos) {
      if (getNum(a)) {
	a->pos = getNum(a) * 2;
	setNum(a,0);
	return 0;
      }
      else {
	setState(a,Init);
	GOTO;
      }
    }
    else {
      int i;
      for (i=1; i<m->NumAnts; ++i) {
	if (getState(&a[i])==Return1 && 
	    a[i].rad==a->rad && 
	    a[i].oct==a->oct && 
	    getNum(&a[i]) <= getNum(a)) {
	  setState(a,Init);
	  GOTO;
	}
      }
      return 0;
    }
  }
  else {
    if (a->pos==127) {
      a->pos = 0;
      return 8 + west;
    }
    else if (a->pos) {
      --a->pos;
      return 8 + returnRoute[a->oct][0];
    }
    if (m[west].Base) {
      a->pos = 127;
      return 8 + north;
    }
    else {
      return 8 + returnRoute[a->oct][1];
    }
  }
  assert(!!!"rotten code");
  return *((int*)0);
}

STATE(Fetch1)
{
  if (a->rad==0) {
    setState(a,Init);
    GOTO;
  }
  
  if (a->leg == 1 || a->leg == 2) {
    int f = findFood(m);
    if (f) {
      struct XY xy;
      struct OR or;
      xy = getXY(a->oct,a->leg,a->rad*3,a->pos);
      xy.x += DX[f];
      xy.y += DY[f];
      
      if (a->bit1) {
	struct XY B2;
	B2.x = sign(xy.x)*96;
	B2.y = sign(xy.y)*96;
	
	setX(a,1-B2.x+xy.x);
	setY(a,1-B2.y+xy.y);
	setState(a,Build);
      }
      else {
	or = getOR(xy.x,xy.y);
	setState(a,Return1);
	a->oct = or.oct;
	a->rad = (or.rad+1)/3;
	a->pos = or.pos;
	setNum(a,0);
      }
      return f;
    }
  }
  if (a->leg == 2 && a->pos == 0) {
    int cw;
    struct XY xy;
    xy = getXY(a->oct,a->leg,a->rad*3,a->pos);
    cw = !((a->oct&1)^((a->oct>>2)&1));
    if (cw) {
      a->oct = route[a->oct][a->leg];
      a->leg = 2-cw*2;
      X = xy.x;
      Y = xy.y;
      setState(a,Search2);
      GOTO;
    }
  }
  
  ++a->pos;
  if (a->pos > 3*a->rad) {
    if (a->leg == 3) {
      setState(a,Init);
      return west;
    }
    ++a->leg;
    a->pos = 1;
  }
  return route[a->oct][a->leg];
}

STATE(Search1)
{
  if (a->leg == 0) {
    struct Triumfant *r;
    int i;
    
    for (i=1; i<m->NumAnts; ++i) {
      r = a+i;
      if (getState(r) == Return1 && getNum(r)) {
	setState(a,Fetch1);
	a->rad = r->rad;
	a->oct = r->oct;
	if (!a->bit1) setNum(a,1);
	if (a->pos&1) setNum(r,getNum(r)-1);
	GOTO;
      }
    }
  }
  else if (a->leg == 1 || a->leg == 2) {
    int f = findFood(m);
    if (f) {
      struct XY xy;
      struct OR or;
      xy = getXY(a->oct,a->leg,a->rad*3,a->pos);
      xy.x += DX[f];
      xy.y += DY[f];
      or = getOR(xy.x,xy.y);
      setState(a,Return1);
      a->oct = or.oct;
      a->rad = (or.rad+1)/3;
      a->pos = or.pos;
      setNum(a,(m[f].NumFood-1-m[f].NumAnts)/2);
      return f;
    }
  }
  
  ++a->pos;
  if (a->pos > 3*a->rad) {
    if (a->leg == 3) {
      setState(a,Init);
      return west;
    }
    ++a->leg;
    a->pos = 1;
  }
  
  return route[a->oct][a->leg];
}

STATE(Search2)
{
  int dir, f;
  
  f = findFood(m);
  if (f) {
    struct OR or;
    X += DX[f];
    Y += DY[f];
    or = getOR(X,Y);
    setState(a,Return1);
    a->oct = or.oct;
    a->rad = (or.rad+1)/3;
    a->pos = or.pos;
    setNum(a,(m[f].NumFood-1-m[f].NumAnts)/2);
    return f;
  }
  
  if (X==0 && (Y-1)%3) {
    if (Y>0) {
      dir = north;
    }
    else {
      dir = south;
    }
  }
  else if (Y==0 && (X+1)%3) {
    if (X>0) {
      dir = east;
    }
    else {
      dir = west;
    }
  }
  else {
    dir = a->oct;
    if (abs(X)==abs(Y)) {
      
      dir = (dir+a->leg)%4 + 1;
      a->oct = dir;
    }
    else if (m[dir].NumAnts > 1) {
      
      int a = (dir+2)%4 + 1;
      int b = dir%4 + 1;
      if (m[dir].NumAnts > m[a].NumAnts) dir = a;
      if (m[dir].NumAnts > m[b].NumAnts) dir = b;
    }
  }
  
  X += DX[dir];
  Y += DY[dir];
  
  if (abs(X)==127 || abs(Y)==127) {
    setState(a,Guard);
    GOTO;
  }
  
  return dir;
}

STATE(Engineer)
{
  if (a->pos == 0) {
    a->pos = 100;
    a->oct += 1;
    a->oct %= 4;
  }
  return 0;
}

STATE(Init)
{
  struct Triumfant* q;
  struct Triumfant* r;
  struct Triumfant* e;
  struct Triumfant* j;
  int i;
  int food[4];
  
  q = findAnt(m,a, Queen);
  if (!q && m[south].Base) {
    setState(a,Queen);
    a->rad = 1;
    a->oct = 0;
    a->pos = 0;
    setNum(a,0);
    return 0;
  }
  else if (!q) {
    setState(a,Guard);
    GOTO;
  }
  
  if (m->NumFood) {
    return 8 + south;
  }
  
  if (!findAnt(m,a,Fetch1)) {
    setState(a,Fetch1);
    a->rad = 0;
    return 0;
  }
  
  e = 0;
  r = 0;
  a->bit1 = 0;
  for (i=0; i<4; ++i) food[i] = 0;
  for (i=1; i<m->NumAnts; ++i) {
    j = a+i;
    if (getState(j) == Return1) {
      if (!j->pos && getNum(j)) {
	j->pos = getNum(j)*2;
	setNum(j,0);
      }
      if (j->pos) {
	food[j->oct&3] += j->pos;
	if (!r || j->rad < r->rad) {
	  r = a+i;
	}
      }
    }
    else if (getState(j) == Engineer) {
      e = j;
    }
  }
  
  if (r) {
    setState(a,Fetch1);
    a->rad = r->rad;
    a->oct = r->oct;
    a->leg = 0;
    a->pos = 0;
    setNum(a,1);
    
    if (e) {
      if (e->pos > 0 && (r->oct&3) == (e->oct&3)) {
	a->bit1 = 1;
	--e->pos;
      }
    }
    else if (getNum(q)>=2) {
      a->pos = 0;
      a->oct = -1;
      setState(a,Engineer);
      GOTO;
    }
    
    --r->pos;
    
    if (getNum(q) < 2 && !q->pos) {
      q->rad = 1;
      q->oct = (getNum(q)&1) ? 4 : 0;
    }
  }
  else {
    setState(a,Search1);
    a->rad = q->rad;
    a->oct = q->oct;
    a->pos = 0;
    a->leg = 0;
    
    if (e && e->pos > 0 && (a->oct&3) == (e->oct&3)) {
      a->bit1 = 1;
    }
    
    q->pos = 0;
    if (getNum(q)&1) {
      if (q->oct == 7) {
	++q->rad;
	q->oct = 4;
      }
      else ++q->oct;
    }
    else {
      if (q->oct == 3) {
	++q->rad;
	q->oct = 0;
      }
      else ++q->oct;
    }
    if (q->rad > 42) {
      setNum(q,getNum(q)+1);
      q->oct = (getNum(q)&1) ? 4 : 0;
      q->rad = 1;
      
    };
  }
  
  return east;
}

STATE(Queen)
{
  return 0;
}

STATE(Guard)
{
  if (findFood(m)) return 0;
  ++a->pos;
  a->rad += m->NumAnts - 1;
  if (m->NumFood || m->NumAnts>1 || !a->pos) return (a->rad&3)+1;
  return 0;
}

int Triumfant(ARGS)
{
  KILLKILLKILL;
  
  if (m->Base) {
    if (m->NumAnts == 1) {
      return 0;
    }
    setState(a,Init);
    return north;
  }
  
  switch (getState(a)) {
    CASE(Queen);
    CASE(Search1);
    CASE(Return1);
    CASE(Fetch1);
    CASE(Search2);
    CASE(Init);
    CASE(Guard);
    CASE(Build);
    CASE(Engineer);
  default: assert(!!!"bogus state"); 
  }
  return *((int*)0);
}
DefineAnt(Triumfant, "Triumfant#90e090", Triumfant, struct Triumfant)

