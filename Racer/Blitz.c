/* Copyright (C) 2003 Kasper Dupont
 * vbtvtluucumhntiuxlwbq@skrammel.yaboo.dk
 *
 * Denne myre startede som et eksperiment, men allerede i
 * sin første rigtige kamp vandt den over alle andre 1-bytes
 * myrer på mindre end 1000 træk. Derfor fik den straks navnet
 * Blitz. Og da den endelig blev opdateret til at også kunne
 * angribe fjender fik ingen andre et ben til jorden.
 */

#include "Myre.h"
struct _ {
  u_char a:2;
  u_char b:6;
};

#define A (_->b)
#define B (_->a)
#define C (_[__].b)

const int n[64]={47,26,45,31,50,9,10,14,6,54,20,40,5,52,22,58,21,8,12,30,63,56,61,24,48,29,23,28,15,59,36,34,38,53,3,19,0,55,39,32,2,44,99,43,62,11,37,57,1,27,41,7,49,33,18,16,46,35,13,60,25,51,4,17};
const int p[64]={36,48,40,34,62,12,8,51,17,5,6,45,18,58,7,28,55,63,54,35,10,16,14,26,23,60,1,49,27,25,19,3,39,53,31,57,30,46,32,38,11,50,99,43,41,2,56,0,24,52,4,61,13,33,9,37,21,47,15,29,59,22,44,20};
const int d[64]={0,0,1,1,0,1,0,1,0,0,0,0,1,1,1,0,1,0,1,1,1,1,1,0,0,1,0,1,0,1,1,1,0,0,0,1,0,1,0,1,0,0,42,0,1,0,1,0,1,1,0,0,1,1,0,1,0,0,0,0,1,1,1,0};
static int _(struct SquareData*x,struct _*_)
{
  u_char u,i,__,___=0;
  int _a=0,_b=0;

  if (x[0].NumAnts > 70)
    for (__=1;__<65;++__)
      if (C==42)
	C=__;
  /*
  if (x[0].Base) {
    if (x[0].NumAnts > 64)
      for (__=1;__<65;++__)
	if (C==42)
	  C=__;
  } else {
    for (__=1;__<x[0].NumAnts;++__)
      if (C==42)
	C=A;
  }
  */

  for (__=1;__<5;++__)
    if (x[__].Team) {
      int _c=x[__].NumAnts+(((int)x[__].Base)<<24);
      if (_c>_a) {
	_a=_c;
	_b=__;
      }
    }

  if (_b) {
    A=42;
    return _b;
  }

  if (A==42) return 0;

  u=((d[A]+B)&3)+1;
  i=((d[p[A]]+B+2)&3)+1;

  for (__=1;__<5;++__) {
    if ((__!=i)&&(x[__].Base)) return __+8;
    if (x[__].NumFood && !x[__].NumAnts) ___=__;
  }

  if (x[0].NumFood) {
    A=p[A];
    return i+8;
  }
  if (___&&!(x[0].NumFood||x[u].NumFood||x[i].NumFood)) return ___;

  //  if (x[0].Base && (x[0].NumAnts<5)) return 0;

  A=n[A];
  return u;
}
DefineAnt(Blitz,"Blitz#EEFF77",_,struct _);
