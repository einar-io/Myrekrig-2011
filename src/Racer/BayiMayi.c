#include "Myre.h"
#define absv(a) ((a) >= 0 ? (a) : -(a))
#define val(x,y) (absv(x)+absv(y))
#define xd(d) (((d)==1)-((d)==3))
#define yd(d) (((d)==4)-((d)==2))
#define hr (m->stat&7)
#define _hr(i) (m[(i)].stat&7)
#define NS 6
#define BD 120
#define PITCH (!B1r(&seed, m->repx.ls/19+2))
#define REHIRE 40
#define CF 8
#define GI 16
#define RECGI 32
#define CW 64

#define IB(b) (m->stat&(b))
#define SB(b) (m->stat|=(b))
#define CB(b) (m->stat&=(~b))
#define _IB(b,i) (m[(i)].stat&(b))
#define _SB(b,i) (m[(i)].stat|=(b))
#define _CB(b,i) (m[(i)].stat&=(~b))
union squeeze {
  signed char repco;
  unsigned char ls;
};
struct B1 {
  signed char x;
  signed char y;
  union squeeze repx;
  union squeeze repy;
  unsigned char stat;
};
unsigned char B1r(unsigned char *seed, unsigned short i) {
  unsigned char rs[] = {120,240,165,224,51,190,206,248,21,164,111,235,142,63,56,245,110,47,243,134,32,242,185,123,177,71,36,232,121,83,94,233,202,53,201,209,57,6,227,74,128,99,101,79,87,15,197,97,68,228,98,145,25,34,192,143,108,170,44,138,86,119,114,64,92,72,140,210,81,95,166,131,62,182,146,208,179,19,183,249,37,205,115,150,184,117,172,136,214,129,13,231,252,27,178,70,186,222,88,61,149,48,213,82,107,122,29,188,60,106,7,116,181,69,153,4,225,76,80,66,246,234,78,133,198,215,139,3,135,90,171,18,9,38,1,250,160,158,189,141,124,200,251,2,244,23,212,96,39,17,16,230,253,132,237,127,223,148,65,194,204,193,33,46,187,241,31,255,54,24,220,137,147,236,100,40,151,58,157,12,126,159,207,155,41,93,163,109,218,105,75,5,174,103,216,254,35,161,77,42,67,175,118,229,144,152,49,154,11,180,52,104,247,169,112,130,85,0,50,168,43,22,196,167,28,125,203,162,156,10,238,199,14,239,219,217,20,30,55,89,59,221,84,45,226,102,195,26,113,211,73,191,91,173,8,176};
  //  if (i<=0) printf("Ups : %d\n", i);
  return ((*seed=rs[*seed])%(i));
}
int B1go_to (short tox, short toy, struct B1 *m, unsigned char seed) {
  if (B1r(&seed, val(m->x-tox, m->y-toy)) >= absv(m->x-tox)) return (m->y>toy) ? 2 : 4;
  return (m->x>tox) ? 3 : 1;
}
int B1m(struct SquareData *felt, struct B1 *m) {
  short i;
  short temp;
  signed char xt, yt;
  unsigned char seed = (-m->x + m->y*2 - m->repx.ls*4 + m->repy.ls*8 - m->stat);
  for (i=0; i<5; i++) {
    if (felt[i].Team && felt[i].NumAnts) {
      SB(GI|RECGI);
      return i;
    }
    seed += ((felt[i].NumAnts<<i)+(felt[i].NumFood<<i)*3);
  }
  if (!felt->Base && (temp = val(m->x, m->y))) {
    for (i=felt->NumAnts-1;i;i--) {
      if (val(m[i].x, m[i].y) > temp) {//coord mod
	if (_hr(i)) {
	  m[i].repx.repco+=(m->x-m[i].x);
	  m[i].repy.repco+=(m->y-m[i].y);
	} else {
	  if (!hr) m[i].repx.ls=m->repx.ls;
	  else {
	    m[i].repx.ls=val(m->x, m->y);
	  }
	}
	m[i].x=m->x;
	m[i].y=m->y;
	// test drop dette
	if (_IB(CW,i)) _CB(CW,i); else _SB(CW,i);
      }
    }
    if (!IB(GI)) for (i=felt->NumAnts-1;i;i--) {//vi er ikke GI
      if (_IB(GI, i)) {//he's  one
	if (_IB(RECGI, i)) {// AND recruiting
	  if (!IB(RECGI) && B1r(&seed, 50)>=REHIRE) _CB(RECGI, i);
	  SB(RECGI); // we go looking for place now
	} else if (IB(CF) || !B1r(&seed,25)) { // set em free
	  _CB(GI | RECGI, i);
	}
      }
    }
  }
  if (hr>1) {
      for (i=felt->NumAnts-1; i; i--) {
	if (!_hr(i) && !_IB(GI, i)) {
	  m->stat--;
	  m[i].stat = ((m[i].stat)&(~7)) | hr;
	  m[i].repx.repco = m->repx.repco; m[i].repy.repco = m->repy.repco;
	  break;
	}
      }
    }
  if (felt->Base)m->x=m->y=0;
  // HOME RULE
  if (!(m->x%BD)&&!(m->y%BD)){
    if (hr) {
      m->repx.repco-=m->x; m->repy.repco-=m->y;
      if (!m->repx.repco && !m->repy.repco) m->stat&=~7;
    }
    m->x=m->y=0;
    if (felt->NumAnts < NS && !B1r(&seed,4)) SB(GI);
    if (IB(GI)) return 16;
    m->stat=B1r(&seed, 4)*CW | (m->stat&7);
    if (felt->NumFood>=NewBaseFood-NewBaseAnts) return 16;
    if (hr) return B1go_to(m->repx.repco, m->repy.repco, m, seed);
    m->repx.ls=0;
    return B1r(&seed,4)+1;
  }
  if (IB(GI)) {
    if (!hr) m->repy.ls++;
    if (B1r(&seed,25)) return 0;
    else {
      CB(GI);
      SB(RECGI);
    }
  }
  if (IB(RECGI) && !IB(GI) && felt->NumAnts<2) {
    temp = 0;
    for (i=1; i<5; i++) if (felt[i].NumAnts) {temp=1; break;}
    if (!temp) {
      SB(GI);
      CB(RECGI);
      return 0;
    }
    else return B1go_to(m->x*2, m->y*2, m, seed);
  }
  if (hr && m->x==m->repx.repco && m->y==m->repy.repco) {
    if (felt->NumFood >= felt->NumAnts) {	
      if (!B1r(&seed,1<<(hr-1))) m->stat--;
      SB(CF);
      return B1go_to(0,0,m,seed);
    }
    else {
      m->stat&=(~7);
      m->repx.ls = val(m->x, m->y);
    }
  } 
  for (i=1; i<5; i++) { //look-out
    if (((temp=(felt[i].NumFood - felt[i].NumAnts))>0) && 
	!(m->x==-xd(i) && m->y==-yd(i)) &&
	(!hr || m->x+xd(i)!=m->repx.repco || m->y+yd(i)!=m->repy.repco)) {
      m->stat&=(~7);
      m->repx.repco = m->x+xd(i); m->repy.repco = m->y+yd(i);
      do m->stat++; while ((temp=temp>>1) && (m->stat&7)<7);
      if (!IB(CF)) return i;
    }
  }
  if (IB(CF)) return B1go_to(0,0,m, seed);
  if (hr) return B1go_to (m->repx.repco, m->repy.repco, m, seed);
  if (m->repx.ls<127) {if (PITCH) m->repx.ls++;}  else m->repx.ls=BD-5;
  xt = IB(CW) ? m->y>0 ? 1 : -1 : m->y<0 ? 1 : -1;
  yt = IB(CW) ? m->x<0 ? 1 : -1 : m->x>0 ? 1 : -1;
  return (absv
	  (m->repx.ls*m->repx.ls - (m->x+xt)*(m->x+xt) - m->y*m->y)
	  <
	  absv
	  (m->repx.ls*m->repx.ls - m->x*m->x - (m->y+yt)*(m->y+yt))
	  ) ? 2-xt : 3+yt;
}
/*
void status (struct B1 *m) {
  printf ("\nMy position : (%d, %d)\n", m->x, m->y);
  printf ("My food-rep : (%d, %d) : %d\n", m->repx, m->repy, m->stat>7);
  printf ("CF : %d\n", IB(CF) ? 1 : 0);
  printf ("RECGI : %d\n", IB(RECGI) ? 1 : 0);
  printf ("CW : %d\n", IB(CW) ? 1 : 0);
  printf ("ADVENTURER : %d\n", IB(ADVENTURER) ? 1 : 0);
}
*/
int B1w(struct SquareData *felt, struct B1 *m) {
  int dir, retval;
  retval = B1m(felt, m);
  if((dir = retval&7)) {
    if (felt[dir].NumAnts >= MaxSquareAnts) {
      CB(CF);
      retval = 0;
      for (dir=4; dir; dir--) if (felt[dir].NumAnts<MaxSquareAnts) retval = dir;
    }
    m->x += xd(dir); m->y += yd(dir);
  }
  return retval | IB(CF);
}
DefineAnt(BayiMayi, "BayiMayi", B1w, struct B1);
#undef absv
#undef sign
#undef val
#undef xd
#undef yd
#undef max
#undef min
#undef r

#undef NS
#undef BD
#undef COSINE
#undef PITCH
#undef PATIENCE
#undef REHIRE

#undef CF
#undef GI
#undef CW
#undef RECGI

#undef IB
#undef SB
#undef CB
#undef _IB
#undef _SB
#undef _CB
