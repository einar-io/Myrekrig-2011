
/* Eriks humanistmyre.
 * 18/10-01 - ?
 */

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// INTERFACE
#define debug(a,b...) printf(a "\n",b);

#include "Myre.h"

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og HUM-DEFs
typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, CARRY=8} Retning;
typedef enum {stINIT=0, stNORMAL, stTELL, stTOLD, stALERT, stLURK} State;

#define HUM_SPIRLEN 100
#define HUM_DUMDIST 500

#define RX HUM_RX
#define RY HUM_RY
#define rnd HUM_rnd
#define goTo HUM_goTo
#define delay HUM_delay
#define goOut HUM_goOut

const int RX[] = {0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0};
const int RY[] = {0, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1};

typedef struct Mind {
    u_long id; 
    short px,py;
    short mx,my;
    unsigned char lastdir;
    char state;
    unsigned /*short*/ int rnd;
} Mind;

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

void delay() {
    int i,j,k;
    for (i=0; i<50; i++)
        for (j=0; j<1000; j++)
            k = i*j;
}

unsigned int rnd(Mind* m) {
    m->rnd = (m->rnd * 3 + m->id);
    return m->rnd;
}

Retning goTo(Mind* m, int dx, int dy) {
    int xx=abs(m->px-dx), yy=abs(m->py-dy);

    if (m->py == dy || (rnd(m) % (xx+yy)) < xx) {
        return (m->px < dx)? EAST : WEST;
    } else {
        return (m->py < dy)? NORTH : SOUTH;
    }
}

Retning goTo2(Mind* m, int dx, int dy) {
    if (m->py == dy /* || (rnd(m) % (xx+yy)) < xx*/) {
        return (m->px < dx)? EAST : WEST;
    } else {
        return (m->py < dy)? NORTH : SOUTH;
    }
}

Retning goSpiral(Mind* m, int dstx, int dsty) {
    int dx = dstx - m->px;  int dy = dsty - m->py; 

    if (dx>dy) {
        if (dx>-dy) return NORTH; else return EAST;
    } else {
        if (dx>=-dy+2 ) return WEST; else return SOUTH;
    }
}


Retning goOut(Mind* m) {
    int xx=abs(m->px), yy=abs(m->py);

    if ((m->id & 1024)? 
        ((m->id & 1023)*xx+((signed) rnd(m))%10 < (yy << 8)) :
        ((m->id & 1023)*yy+((signed) rnd(m))%10 > (xx << 8)))
    {
        if (m->px > 0) return EAST;
        if (m->px < 0) return WEST;
        return (rnd(m) & 2)? EAST : WEST;
    } else {
        if (m->py > 0) return NORTH;
        if (m->py < 0) return SOUTH;
        return (rnd(m) & 2)? NORTH : SOUTH;
    }
}



int/*Retning*/ HumanistMain(struct SquareData *sqr, Mind* m) {
    Retning dir = STOP;
    int i;

    //delay();

    switch (m->state) {
    case stINIT: {
        m->rnd = m->id;
        m->state = stNORMAL;
    } break;

    case stNORMAL: case stTOLD: {
        if (m->mx || m->my) {
            if (sqr->NumFood > sqr->NumAnts+1) {
                dir = goTo(m,0,0) | CARRY;
                m->mx = m->px; m->my = m->py;
            } else if (sqr->NumFood >= sqr->NumAnts) {
                dir = goTo(m,0,0) | CARRY;
            } else {
                dir = goTo(m, m->mx, m->my);
                if (m->px == m->mx && m->py == m->my) { // No more food here.
                    if (rnd(m) & 1024) { // Semi-forget (guess)
                        if (m->id & 2048) {
                            int tmp = m->mx;
                            m->mx = m->my * 3/2;
                            m->my = -tmp;
                        } else {
                            int tmp = m->my;
                            m->my = m->mx * 3/2;
                            m->mx = -tmp;
                        }
                    } else {
                        m->mx = m->my = 0; // Forget food place.
                    }
                }
            }
        } else {
            if (sqr->NumFood >= sqr->NumAnts) {
                m->mx = m->px; m->my = m->py;
                dir = goTo(m,0,0) | CARRY;

                if (m->state != stTOLD) m->lastdir = HUM_SPIRLEN;
                m->state = stTELL;
            } else {
                /*
                if (m->id & (3<<10)) {
                    i=4;
                    do {
                        dir = (rnd(m) % 5);
                    } while (i-- && (dir == STOP
                                     || dir == m->lastdir
                                     || sqr[dir].NumAnts));
                    m->lastdir = dir;
                } else {
                    dir = goOut(m);
                }
                */
                if (abs(m->px)+abs(m->py) > HUM_DUMDIST) {
                    dir = (rnd(m) % 5);
                    if (dir == STOP || dir == m->lastdir)
                        dir = ((m->lastdir+1)&3)+1;
                    m->lastdir = dir;
                } else {
                    dir = goOut(m);
                }

            }
        }
    } break;

    case stTELL: {
        for (i=1; i<=4; i++) {
            if (sqr[i].NumAnts && !sqr[i].NumFood) {
                dir = i | CARRY;
            }
        }
        if (dir == STOP) {
            dir = goSpiral(m, m->mx, m->my) | CARRY;
        }
        if (--m->lastdir <= 0) m->state = stNORMAL;
    } break;
    case stALERT: {
        if (m->px || m->py)
            dir = goTo(m, 0,0);
        else
            m->state = stNORMAL;
    } break;
    case stLURK: {
        dir = STOP;
        if (sqr->NumFood) m->state = stNORMAL;
    } break;
    } //switch


    {
        int mydist = abs(m->mx)+abs(m->my);
        for (i=1; i<sqr->NumAnts; i+=2) {
            Mind* mp = &((Mind*)m)[i];
            int yourdist = abs(mp->mx)+abs(mp->my);
            if (mydist>0 && (yourdist==0)) { //closer?
                // Tell:
                mp->mx = m->mx; mp->my = m->my;
                if (m->state == stTELL) {
                    mp->state = stTOLD;
                    mp->lastdir = m->lastdir/2;
                }
            }
        }
    }

    if (m->state != stTELL) {
        // Food?
        if (!sqr->NumFood) {
            for (i=1; i<=4; i++) {
                if (sqr[i].NumFood > sqr[i].NumAnts) {
                    dir = i;
                    break;
                }
            }
        }
    }

    // Enemies?
    for (i=1; i<=4; i++) {
        if (sqr[i].Team != 0) {
            dir = i;
            if (!sqr->NumFood)
                m->state = stLURK;
            /*
            m->state = stALERT;
            m->mx = m->px + RX[dir];
            m->my = m->py + RY[dir];
            */
            break;
        }
    }

    m->px += RX[dir];
    m->py += RY[dir];
    return dir;
}

DefineAnt(Humanist, "Humanist#FF006A", HumanistMain, struct Mind)
