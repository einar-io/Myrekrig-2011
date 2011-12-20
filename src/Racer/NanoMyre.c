/* Eriks Nano-myre.
 * 14/5-02 - ?
 */
#include "Myre.h"

/* ---- Defs ----------------------------------------*/
#define think NANO_##think
#define main NANO_##main
#define random NANO_##random
#define RX NANO_##RX
#define RY NANO_##RY
#define goHome NANO_##goHome
#define goHomeGridwise NANO_##goHomeGridwise
#define goOut NANO_##goOut
#define goOut2 NANO_##goOut2
#define goRandom NANO_##goRandom
#define goTo NANO_##goTo
#define gridCommand NANO_##gridCommand
#define findGridAnt NANO_##findGridAnt



typedef struct SquareData SquareData;

typedef enum {
    STOP=0, HERE=0,
    EAST, SOUTH, WEST, NORTH,
    MAXDIR = NORTH,
    CARRY=8,
    BUILDBASE=16
} Retning;

enum State {
    stINIT = 0,
    stEXPLORE,
    stCIRKUS_OUT,
    stCIRKUS,
    stGRIDLOCK,
    stKNOWSFOOD,
    stLURK,

    __FOOBAR__
};

typedef struct Mind {
    short mx,my;
    short px,py;
    short rnd;
    unsigned char state; signed char data;
} Mind;

const int RX[] = {0, 1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0,
		     1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0};
const int RY[] = {0, 0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0,
		     0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0};

/* ---- Helper functions ----------------------------------------*/

__inline__ int random(Mind* m) {
    m->rnd = m->rnd*4637+342;
    return m->rnd;
}


__inline__ Retning goRandom(Mind* m) {
    int dir = ((random(m)>>5)&3) + 1;
    //printf("Random dir: %d\n", dir);
    return dir;
}

__inline__ Retning goHome(Mind* m) {
    return m->px? (m->px>0? WEST : EAST) : 
	m->py? (m->py>0? SOUTH : NORTH) : STOP;
}

__inline__ Retning goHomeGridwise(Mind* m) {
    if ((m->px|m->py)&15) { // Not on grid point?
	return m->px&15? (m->px>0? WEST : EAST) : 
	    (m->py>0? SOUTH : NORTH);
    }
    // On grid point:
    if (!m->px) return m->py? (m->py>0? SOUTH : NORTH) : STOP;
    if (!m->py) return (m->px>0? WEST : EAST);
    return random(m)&8 ?
	(m->px>0? WEST : EAST) : (m->py>0? SOUTH : NORTH);
}

__inline__ Retning goOut(Mind* m) { 
    unsigned short r = random(m);
    int d = abs(m->px)+abs(m->py);
    if (d > 128) {
	return STOP;
	/*
	return (r&32)?
	    (m->px>((r&7)-4)? EAST : WEST) : 
	    (m->py>((r&7)-4)? NORTH : SOUTH);
	*/
    } else {
	int d1 = (r&7)-4;
	int d2 = ((r>>3)&7)-4;
	if (m->px > d1) {
	    if (m->py > d2)
		return (m->px*r > (m->py<<15))? EAST : NORTH;
	    else
		return (m->px*r > -(m->py<<15))? EAST : SOUTH;
	} else {
	    if (m->py > d2)
		return (-m->px*r > (m->py<<15))? WEST : NORTH;
	    else
		return (-m->px*r > -(m->py<<15))? WEST : SOUTH;
	}
    }
}

__inline__ Retning goOut2(Mind* m) {
    if ((unsigned)m->data >= 4) {
	m->data -= 4;
    } else {
	int dir;
	int newdata = (random(m) + m->px + m->py)&0x3F;
	if (((m->data ^ newdata)&3) == 2) // Opposite dir
	    newdata++; // Turn a bit!

	dir = (m->data&3)+1;
	if (m->px*RX[dir]<0 || m->py*RY[dir]<0) newdata -= 8*4;
	m->data = newdata;

	m->rnd = (m->rnd<<8)|((unsigned)m->rnd>>8); // Make more random
    }
    return (m->data&3)+1;
}

__inline__ Retning goTo(Mind* m, int x, int y) {
    return m->px!=x? (m->px>x? WEST : EAST) : 
	m->py!=y? (m->py>y? SOUTH : NORTH) : STOP;
}


__inline__ void gridCommand(Mind* g, Mind* m) {
    int dir = g->state & 0xE0;

    if (g->mx|g->my) { // Tell explorer about food
	m->state = stKNOWSFOOD;
	m->mx = g->mx;
	m->my = g->my;
	if ((random(g)&3) == 0) { // Forget about food
	    g->mx = g->my = 0;
	}
    } else {
	m->state = stEXPLORE | dir;
	dir += (1<<5);
	if (dir > (4<<5)) dir = 0;
	g->state = stGRIDLOCK | dir;
    }
}

int findGridAnt(SquareData *sqr, Mind* m) {
    int i;
    for (i=1; i<sqr->NumAnts; i++) {
	if ((m[i].state&0x1F) == stGRIDLOCK) return i;
    }
    return 0;
}


__inline__ Retning think(SquareData *sqr, Mind* m) {
    int i;

    // Defense:
    for (i=1; i<=4; i++) {
	if (sqr[i].Team) {
	    m->state = stLURK; m->data=0;
	    return i;
	}
    }

    switch (m->state & 0x1F) {
    case stINIT: {
	m->rnd = m->mx;
	m->px = m->py = 0;
	m->mx = m->my = 0;
	if (1 || random(m)&16) {
	    m->state = stEXPLORE;
	    goto st_explore;
	} else {
	    int r = sqr->NumAnts*2 + 
		(sqr[1].NumAnts + sqr[2].NumAnts +
		 sqr[3].NumAnts + sqr[4].NumAnts);
	    m->state = stCIRKUS_OUT;
	    m->data = 5 + (unsigned)(random(m)) % (r+10);
	    //printf("Radius: %d\n", m->data);
	    goto st_cirkus;
	}
    } break;
    case stEXPLORE: {
    st_explore:
	if (sqr->NumFood) {
	    if (sqr->NumFood > 1) {
		m->state = stKNOWSFOOD;
		m->mx = m->px; m->my = m->py;
	    }
	    return goHomeGridwise(m) | CARRY;
	}
	for (i=1; i<=4; i++) {
	    if (sqr[i].NumFood > sqr[i].NumAnts) {
		return i;
	    }
	}

	if (((m->px|m->py)&15) == 0) {
	    int gridant = findGridAnt(sqr, m);
	    if (gridant) {
		gridCommand(&m[gridant], m);
	    } else if (random(m)&4) {
		int dir = ((m->px+m->py)>>4)&3; 
		m->state = stGRIDLOCK | (dir<<5);
		//printf("Made gridlock at %d,%d\n", m->px, m->py);
		return STOP;
	    } else {
		m->state = stEXPLORE | (HERE<<5);
	    }
	}

	{
	    int dir = (m->state >> 5); // NSEW or HERE
	    if (dir) return dir; // On road
	    else return goOut2(m); //goRandom(m); //Explore
	}
    } break;

    case stCIRKUS_OUT: {
	if (sqr->NumFood) {
	    if (sqr->NumFood > 1) {
		m->state = stKNOWSFOOD;
		m->mx = m->px; m->my = m->py;
	    }
	    return goHomeGridwise(m) | CARRY;
	}
	for (i=1; i<=4; i++) {
	    if (sqr[i].NumFood > sqr[i].NumAnts) {
		return i;
	    }
	}

	{
	    int a,r;
	    a = m->px*m->px + m->py*m->py;
	    r = m->data*m->data << 2;//m->my*m->my;

	    if (a>r) m->state = stCIRKUS;
	    return goOut(m);
	}	
    } break;

    case stCIRKUS: {
	int a,r;
    st_cirkus:

	if (sqr->NumFood) {
	    if (sqr->NumFood > 1) {
		m->state = stKNOWSFOOD;
		m->mx = m->px; m->my = m->py;
	    }
	    return goHomeGridwise(m) | CARRY;
	}
	for (i=1; i<=4; i++) {
	    if (sqr[i].NumFood > sqr[i].NumAnts) {
		return i;
	    }
	}

	a = m->px*m->px + m->py*m->py;
	r = m->data*m->data << 2;//m->my*m->my;
	if (m->py == 0) {
	    if (m->px>0 && (m->px&1)) m->data += random(m)&7;
	} else { // m->py != 0
	    if (sqr->NumAnts>1) m->data += random(m)&31;
	}

	if (a<r) {
	    if (m->px>0)
		return (m->py>0)? EAST : SOUTH;
	    else
		return (m->py>0)? NORTH : WEST;
	} else {
	    if (m->px>0)
		return (m->py>0)? SOUTH : WEST;
	    else
		return (m->py>0)? EAST : NORTH;
	}
    } break;

    case stGRIDLOCK: {
	for (i=1; i<sqr->NumAnts; i++) if (m[i].state == stEXPLORE) {
	    gridCommand(m, &m[i]);
	}
	return STOP;
    } break;
    case stKNOWSFOOD: {
	for (i=1; i<sqr->NumAnts; i++) {
	    int st = m[i].state&0x1F;
	    if (st == stGRIDLOCK) {
		if (sqr->NumFood && random(m)&8) { // Tell grid
		    m[i].mx = m->mx;
		    m[i].my = m->my;
		}
	    } else if (st==stEXPLORE ||
		       st==stCIRKUS || st==stCIRKUS_OUT)
	    {
		if (random(m)&8) { // Tell explorer
		    m[i].state = stKNOWSFOOD;
		    m[i].mx = m->mx;
		    m[i].my = m->my;
		}
	    }
	}

	if (sqr->NumFood) {
	    if (sqr->NumFood > 1) {
		m->mx = m->px;
		m->my = m->py;
	    }
	    return goHomeGridwise(m) | CARRY;
	}
	for (i=1; i<=4; i++) {
	    if (sqr[i].NumFood > sqr[i].NumAnts) {
		return i;
	    }
	}

	i = goTo(m, m->mx,m->my);
	if (i)
	    return i;
	else {
	    m->state = stEXPLORE;
	    m->mx = m->my = 0;
	    goto st_explore;
	}	
    } break;
    case stLURK: {
	m->data++; //TODO
	return STOP;
    } break;
    default: {
	return SOUTH;
    } break;
    }//switch

    return STOP;   
}

int/*dvs. Retning*/ main(SquareData *sqr, Mind* m) {
    Retning dir;
    //printf("<");
    dir = think(sqr, m);
    m->px += RX[dir]; m->py += RY[dir]; // Update coordinates
    //printf(": %d>\n", dir);
    return dir;
}

DefineAnt(NanoMyre, "NanoMyre#8040FF", main, struct Mind)
