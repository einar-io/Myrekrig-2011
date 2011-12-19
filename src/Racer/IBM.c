/* Eriks store blå myre.
 * 17/5-02 - ?
 */
#include "Myre.h"

#define main IBM_##main
#define think IBM_##think
#define RX IBM_##RX
#define RY IBM_##RY

typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, 
              CARRY=8, BUILDBASE=16} Retning;
const int RX[] = {0, 1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0,
		     1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0};
const int RY[] = {0, 0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0,
		     0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0};

#define FOREACH(foo,lim,s) {int foo; for(foo=1; foo<=(lim); foo++) {s}}
#define MOD_X(dx) ((m->px>(dx))? WEST : EAST)
#define MOD_Y(dy) ((m->py>(dy))? SOUTH : NORTH)

typedef struct Mind {
    unsigned short rnd; //2
    unsigned char data, mata; //2
    short px,py;   //4
    short mx,my;   //4
} Mind;


Retning think(struct SquareData *sqr, Mind* m) {
    if (sqr->Base) {m->px=m->py=m->data=m->mata=0; }
    FOREACH(r,4,if (sqr[r].Team) return r;);
    if (sqr->NumAnts>1 && (m->mx|m->my)) {
	FOREACH(i,sqr->NumAnts-1,
		if (!(m[i].mx|m[i].my)) {m[i].mx=m->mx; m[i].my=m->my;});
    }
    if (sqr->NumFood) {
	if (sqr->NumFood>1) {m->mx=m->px; m->my=m->py;}
	if (m->px) return MOD_X(0) | CARRY;
	if (m->py) return MOD_Y(0) | CARRY;
    }
    if (m->mx|m->my) {
	if (m->px!=m->mx) return MOD_X(m->mx);
	if (m->py!=m->my) return MOD_Y(m->my);
	m->mx = m->my = 0;
    }
    FOREACH(r,4,if (sqr[r].NumFood>sqr[r].NumAnts) return r;);
    if (m->data==0) {
	m->data = ++m->mata;
	m->rnd=m->rnd*4237+54;
	{
	    int oldrnd = m->rnd;
	    m->rnd = (m->rnd>>5)+(m->rnd<<3);
	    if (((oldrnd^m->rnd)&3)==0) m->rnd--;
	}
    } else m->data--;
    return (m->rnd&3)+1;
}

int/*Retning*/ main(struct SquareData *sqr, Mind* m) {
    Retning dir = think(sqr,m);
    m->px += RX[dir];
    m->py += RY[dir];
    return dir;
}

DefineAnt(IBM, "IBM#6699FF", main, struct Mind)
