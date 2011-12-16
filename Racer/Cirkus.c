/* Eriks pop-myre.
   5/3-02 - ?
 */
#include "Myre.h"
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og IQU-DEFs
typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, CARRY=8, BUILDBASE=16} Retning;

typedef enum {
    stINIT=0, stIDLE, stQUEEN,
    stSEARCH, stKNOWNSFOOD, stGETFOOD,
    stCIRCLE,
    stLURK
} State;

#define RX CIRKUS_RX
#define RY CIRKUS_RY
#define TURN CIRKUS_TURN
#define TURN180 CIRKUS_TURN180
#define delay CIRKUS_delay
#define delay2 CIRKUS_delay2
#define goHome CIRKUS_goHome
#define goDir CIRKUS_goDir
#define goThere CIRKUS_goThere
#define goOut CIRKUS_goOut
#define goTo CIRKUS_goTo
#define bitswap6 CIRKUS_bitswap6

const int RX[] = {0, 1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0,
		     1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0};
const int RY[] = {0, 0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0,
		     0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0};
const int TURN[] = {0, 2,3,4,1, 6,7,8,5, 10,11,12,9, 14,15,16,13};
const int TURN180[] = {0, 3,4,1,2, 7,8,5,6, 11,12,9,10, 15,16,13,14};

enum {FLAG1=16, FLAG2=32, FLAG3=64, FLAG4=128};

typedef struct Mind {
    short id;
    short mx,my;
    short px,py;
    State state; unsigned char data;
} Mind;
/* Queen: data=dir
   Search: data=dir, my=limit
   Getfood: (mx,my)=foodpos
 */


const int bitswap6[64] = {
    0, 32, 16, 48,  8, 40, 24, 56,
    4, 36, 20, 52, 12, 44, 28, 60,
    2, 34, 18, 50, 10, 42, 26, 58,
    6, 38, 22, 54, 14, 46, 30, 62,
    1, 33, 17, 49,  9, 41, 25, 57,
    5, 37, 21, 53, 13, 45, 29, 61,
    3, 35, 19, 51, 11, 43, 27, 59,
    7, 39, 23, 55, 15, 47, 31, 63
};
Retning goOut(Mind* m) {
    int a = abs(m->px)+abs(m->py) + m->id;
    if (bitswap6[a&63] < (m->data&63)) {
	return (m->px>0)? EAST : (m->px==0)? (m->id&FLAG1? EAST:WEST) : WEST;
    } else {
	return (m->py>0)? NORTH : (m->py==0)? (m->id&FLAG2? NORTH:SOUTH) : SOUTH;
    }
}
Retning goHome(Mind* m) {
    int a;
    if (!m->px) {
	if (!m->py) return STOP;
	return (m->py>0)? SOUTH : NORTH;
    }
    if (!m->py)
	return (m->px>0)? WEST : EAST;

    a = abs(m->px)+abs(m->py) + m->id;
    if (bitswap6[a&63] < (m->data&63)) {
	return (m->px<0)? EAST : WEST;
    } else {
	return (m->py<0)? NORTH : SOUTH;
    }
}
Retning goTo(Mind* m, int dstx, int dsty) {
    int a;
    if (m->px==dstx) {
	if (m->py==dsty) return STOP;
	return (m->py>dsty)? SOUTH : NORTH;
    }
    if (m->py==dsty)
	return (m->px>dstx)? WEST : EAST;

    a = abs(m->px)+abs(m->py) + m->id;
    if (bitswap6[a&63] < (m->data&63)) {
	return (m->px<dstx)? EAST : WEST;
    } else {
	return (m->py<dsty)? NORTH : SOUTH;
    }
}


Retning CirkusThink(struct SquareData *sqr, Mind* m) {
    Retning dir;
    int i;

    switch (m->state) {
    case stINIT: {
	int queen_no;
    init:
	m->px = m->py = 0;
	
	queen_no = 0;
	for (i=1; i<sqr->NumAnts; i++) {
	    if (m[i].state == stQUEEN) {
		queen_no = i; break;
	    }
	}

	if (queen_no==0) {
	    m->state = stQUEEN;
	    m->data = m->mx;
	    m->mx = 150;
	    goto queen;
	} else {
	    Mind* q = &m[queen_no];
	    m->state = stSEARCH;
	    m->data = q->data; q->data+=/*4*/71;
	    if (1) {
		int tmp;
		if (m->my < 1000)
		    m->my = 40 + q->my/15 - (m->id&15);
		else
		    m->my = (40 + q->my/15 - (m->id&15)) % 150;
		/*
		if (m->my > q->mx) m->my = (3*m->my)%q->mx;
		q->my++;
		q->mx++;
		/*
		tmp = m->my;
		
		m->my *= 3;

		while (m->my > q->mx) m->my -= (q->mx-20);

		m->my = (3*m->my) % q->mx; 
		if (tmp > 2*q->mx) q->mx += 30;
		*/
	    }
	    goto search;
	}
	
    } break;
    case stIDLE: {
    idle:
	dir = goHome(m);
	if (!dir) goto init;
	return dir;
    } break;
    case stQUEEN: {
    queen:
	/* TODO
	for (i=1; i<sqr->NumAnts; i++) if (m[i].state == stGETFOOD) {
	    if (m[i].data > m->mx) 
	}
	m->mx--;
	*/
	m->my++; //Time...
    } break;

    case stSEARCH: {
    search:
	if (sqr->NumFood) {
	    m->state = stGETFOOD;
	    m->mx = m->px; m->my = m->py;
	    goto getfood;
	}

	for (i=1; i<=4; i++) if (sqr[i].NumFood > sqr[i].NumAnts) {
	    m->state = stGETFOOD; 
	    m->mx = m->px+RX[i]; m->my = m->py+RY[i];
	    return i | CARRY;
	}

	//if (abs(m->px)+abs(m->py) > m->my)
	if (m->px*m->px + m->py*m->py > m->my*m->my)
	    m->state = stCIRCLE;//stIDLE;
	return goOut(m);
    } break;

    case stGETFOOD: {
    getfood:
	if (sqr->NumFood) return goHome(m) | CARRY;
	dir = goTo(m, m->mx, m->my);
	if (!dir) {
	    m->state = stIDLE; goto idle;
	}
	return dir;
    } break;

    case stCIRCLE: {
	int a = m->px*m->px + m->py*m->py;
	int r = m->my*m->my;

	for (i=1; i<sqr->NumFood; i++) if (m[i].state==stGETFOOD) {
	    m->state = stGETFOOD;
	    m->mx = m[i].mx; m->my = m[i].my;
	    goto getfood;
	}

	if (sqr->NumFood) {
	    m->state = stGETFOOD;
	    m->mx = m->px; m->my = m->py;
	    goto getfood;
	}

	for (i=1; i<=4; i++) if (sqr[i].NumFood > sqr[i].NumAnts) {
	    m->state = stGETFOOD; 
	    m->mx = m->px+RX[i]; m->my = m->py+RY[i];
	    return i | CARRY;
	}

	//if (m->py==0) m->my+=5; //m->my+=m->my/20;
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
	    
	//return goOut(m);
    } break;

    case stLURK: {
	return STOP;
    } break;

    default: { } break;
    }//switch
    return STOP;
}

int /*Retning*/ CirkusMain(struct SquareData *sqr, Mind* m) {
    Retning dir;
    int i;

    int mydist = abs(m->px)+abs(m->py);
    for (i=1; i<sqr->NumAnts; i++) {
	int yourdist = abs(m[i].px)+abs(m[i].py);
	if (mydist > yourdist) {
	    m->px = m[i].px;
	    m->py = m[i].py;
	    //TODO: Synkroniser evt. mx, my
	}
    }

    // Enemies?
    for (i=1; i<=4; i++) if (sqr[i].Team != 0) {
	dir = i;
	m->state = stLURK;
	goto done;
    }

    dir = CirkusThink(sqr, m);

    if (sqr[dir&7].NumAnts > MaxSquareAnts*3/4) {
	//printf("Mange!\n");
	if (dir&CARRY) {
	    if (sqr[dir&7].Base && sqr[dir&7].NumAnts < MaxSquareAnts) {
		/*go on*/
	    } else {
		dir = TURN180[dir];
		if (sqr[dir&7].NumAnts > MaxSquareAnts*3/4) {
		    dir = STOP;
		}
	    }
	} else {
	    dir = TURN[dir];
	    if (sqr[dir&7].NumAnts > MaxSquareAnts*3/4) {
		dir = TURN180[dir];
		if (sqr[dir&7].NumAnts > MaxSquareAnts*3/4) {
		    dir = STOP;
		}
	    }
	}
    }

done:
    m->px += RX[dir];
    m->py += RY[dir];
    return dir;
}

DefineAnt(Cirkus, "Cirkus#ff5d00", CirkusMain, struct Mind)
