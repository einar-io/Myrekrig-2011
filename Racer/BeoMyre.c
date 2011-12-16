/* Eriks RISC-like myre II.
 * Måske.
 * 26?/9-01 - ?
 */

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// INTERFACE
#define debug(a,b...) printf(a "\n",b);

#include "MyreKrig.h"

//#include "math.h"

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og BEO-DEFs
typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, CARRY=8} Retning;
typedef enum {stINIT=0, stGETFOOD, stRETURNFOOD, stLURK} State;

#define RX BEO_RX
#define RY BEO_RY
#define goround BEO_goround
#define gohome BEO_gohome
#define gotom BEO_gotom
#define delay BEO_delay


const int RX[] = {0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0};
const int RY[] = {0, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1};

typedef struct Mind {
    u_long id; 
    short px,py;
    short mx,my;
    char state, data;
    signed char rnd;
} Mind;
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

Retning goround(Mind* m) {
    int xx, yy;
/*
    xx = -m->py + (m->rnd *= 37)/2;
    yy = m->px + m->py/4;
*/
    xx = -m->py + (m->rnd *= 37)/2 + ((signed char)m->id)/8;
    yy = m->px + (((signed char)(m->id/8))/8) + m->py/4;
    if (abs(xx) > abs(yy)) {
        if (xx>0) return EAST; else return WEST;
    } else {
        if (yy>0) return NORTH; else return SOUTH;
    }
}

Retning gohome(Mind* m) {
    if (abs(m->px) > abs(m->py)) {
        if (m->px>0) return WEST; else return EAST;
    } else {
        if (m->py>0) return SOUTH; else return NORTH;
    }
}

Retning gotom(Mind* m) {
    int xx, yy;
    xx = m->px - m->mx + (m->rnd += 67)/64;
    yy = m->py - m->my;
    if (abs(xx) > abs(yy)) {
        if (m->px > m->mx) return WEST; else return EAST;
    } else {
        if (m->py > m->my) return SOUTH; else return NORTH;
    }
}

void delay() {
    int i,j,k;
    for (i=0; i<10; i++)
        for (j=0; j<1000; j++)
            k = i*j;
}

int /*Retning*/ BeoMain(struct SquareData *sqr, Mind* m) {
    Retning dir = STOP;
    int i;
    
    //delay();
    
retest:
    switch (m->state) {
    case stINIT: {
        m->state = stGETFOOD;
        m->rnd = m->id;
    } // fallthrough
    case stGETFOOD: {
        // Se her efter mad:
        if (sqr->NumFood) { 
            if (sqr->NumFood > 1) {
                m->mx = m->px; m->my = m->py;
            }
            m->state = stRETURNFOOD; goto retest;
        }
        // Ingen mad...
        if (m->mx || m->my) {
            if (m->px == m->mx && m->py == m->my) {
                m->mx = m->my = 0; //Ingen mad her alligevel.
                dir = ((m->rnd+m->id) & 3) + 1;
            }
            dir = gotom(m);
            for (i=1; i<sqr[HERE].NumAnts; i+=2) {
                Mind* mp = &((Mind*)m)[i];
                if (mp->state == stGETFOOD && !(mp->mx||mp->my)) {
                    mp->mx = m->mx;
                    mp->my = m->my;
                }
            }
        } else {
            // Se om efter mad:
            for (i=1; i<=4; i++) {
                if (sqr[i].NumFood > sqr[i].NumAnts) { 
                    m->mx = m->px; m->my = m->py;
                    dir = i;
                    m->state = stRETURNFOOD; goto found;
                }
            }
            dir = goround(m);
        found:
        }
    } break;
    case stRETURNFOOD: {
        if (!sqr->NumFood) { m->state = stGETFOOD; goto retest; }
        dir = gohome(m) | CARRY;

        for (i=1; i<sqr[HERE].NumAnts; i+=2) {
            Mind* mp = &((Mind*)m)[i];
            if (mp->state == stGETFOOD) {
                mp->mx = m->mx;
                mp->my = m->my;
            }
        }
    } break;
    case stLURK: {
        dir = STOP;
        --m->data;
        /*
        if (m->data == 0) {
            m->state = stRETURNFOOD;
            } else*/
        if ((m->data & 7) == 0) {
            dir = goround(m);
        }
    } break;
    }//switch

    // Enemies?
    for (i=1; i<=4; i++) {
        if (sqr[i].Team != 0) {
            dir = i;
            m->state = stLURK; m->data = 255;
            break;
        }
    }

    m->px += RX[dir];
    m->py += RY[dir];
    return dir;
}

DefineAnt(BeoMyre, "BeoMyre", BeoMain, struct Mind)
