/* Eriks tirsdagsmyre.
 * 16/10-01.
 */

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// INTERFACE
#define debug(a,b...) printf(a "\n",b);

#include "Myre.h"

//#include "math.h"

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og TIRS-DEFs
typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, CARRY=8} Retning;
typedef enum {stINIT=0, stWAIT, stGETFOOD, stRETURNFOOD, stLURK} State;

#define RX TIRS2_RX
#define RY TIRS2_RY
#define goOut TIRS2_goOut
#define goHome TIRS2_goHome
#define goTo TIRS2_goTo
#define goRound TIRS2_goRound
#define delay TIRS2_delay
#define rnd TIRS2_rnd

const int RX[] = {0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0};
const int RY[] = {0, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1};

typedef struct Mind {
    u_long id; 
    short px,py;
    short mx,my;
    char state/*, data*/;
    unsigned char rnd;
} Mind;

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
void delay() {
    int i,j,k;
    for (i=0; i<10; i++)
        for (j=0; j<1000; j++)
            k = i*j;
}

unsigned int rnd(Mind* m) {
    m->rnd = (m->rnd * 7813) ^ (m->rnd >> 2);
    return m->rnd;
}

Retning goOut(Mind* m) {
    int xx=abs(m->px), yy=abs(m->py);
    if (
        (m->id & (2048|4096))?
        (m->id & 1024)? 
        ((m->id & 1023)*xx < (yy << 6)) :
        ((m->id & 1023)*yy > (xx << 6))
        :
        (m->id & 1024)? 
        ((m->id & 1023)*xx < (yy << 10)) :
        ((m->id & 1023)*yy > (xx << 10)) )
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

Retning goRound(Mind* m) {
    int xx=abs(m->px), yy=abs(m->py);

    if (xx < 2*yy) {
        if (m->py > 0) return EAST;
        else return WEST;
    } else {
        if (m->px > 0) return SOUTH;
        else return NORTH;
    }
}

Retning goHome(Mind* m) {
    int xx=abs(m->px), yy=abs(m->py);

    if (m->py == 0 || (rnd(m) % (xx+yy)) < xx) {
        return (m->px < 0)? EAST : WEST;
    } else {
        return (m->py < 0)? NORTH : SOUTH;
    }
}

Retning goTo(Mind* m, int dx, int dy) {
    int xx=abs(m->px-dx), yy=abs(m->py-dy);

    if (m->py == dy || (rnd(m) % (xx+yy)) < xx) {
        return (m->px < dx)? EAST : WEST;
    } else {
        return (m->py < dy)? NORTH : SOUTH;
    }
}


int/*Retning*/ TirsdagMain(struct SquareData *sqr, Mind* m) {
    Retning dir = STOP;
    int i;

//    delay();

    switch (m->state) {
    case stINIT: {
        m->rnd = m->id;
        //m->state = stGETFOOD;
        m->state = (m->id & (1024|512))? stGETFOOD : stWAIT;
    } // continue:
    case stGETFOOD: {
        if (sqr->NumFood) {
            m->state = stRETURNFOOD;
            m->mx = m->px; m->my = m->py;
            //continue
        } else {
            if (m->mx || m->my) { // Ved hvor der er mad:
                if (m->px == m->mx && m->py == m->my)
                    m->mx = m->my = 0;
                else
                    dir = goTo(m, m->mx, m->my);
            } else {
                dir = goOut(m);
            }
            break;
        }
    }; //break or continue
    case stRETURNFOOD: {
        if (!sqr->NumFood) {
            m->state = stGETFOOD;
        } else {
            dir = goHome(m) | CARRY;
        }
    }; break;
    case stWAIT: {
        if (sqr->NumFood) {
            m->state = stRETURNFOOD;
            m->mx = m->px; m->my = m->py;
        } else {
            if (m->mx || m->my) { // Ved hvor der er mad:
                m->state = stGETFOOD;
                //printf("Snapped out of it\n");
                if (m->px == m->mx && m->py == m->my)
                    m->mx = m->my = 0;
                else
                    dir = goTo(m, m->mx, m->my);
            } else {
                dir = STOP;
            }
        }
        break;
    };
    case stLURK: {
        dir = STOP;
        // Bo-ring!
    } break;
    default:
        //printf("What!?\n");
    } // switch

    {
        int mydist = abs(m->mx)+abs(m->my);
        for (i=1; i<sqr->NumAnts; i+=2) {
            Mind* mp = &((Mind*)m)[i];
            int yourdist = abs(mp->mx)+abs(mp->my);
            if (mydist>0 && (yourdist==0/* || mydist < yourdist*/)) { //closer?
                // Tell:
                mp->mx = m->mx; mp->my = m->my;
            }
        }
    }


    // Food?
    if (!sqr->NumFood) {
        for (i=1; i<=4; i++) {
            if (sqr[i].NumFood > sqr[i].NumAnts) {
                dir = i;
                break;
            }
        }
    }

    // Enemies?
    for (i=1; i<=4; i++) {
        if (sqr[i].Team != 0) {
            dir = i | CARRY /*?*/;
            m->state = stLURK;
            break;
        }
    }

    m->px += RX[dir];
    m->py += RY[dir];
    return dir;
}

DefineAnt(Tirsdag, "Tirsdag", TirsdagMain, struct Mind)

