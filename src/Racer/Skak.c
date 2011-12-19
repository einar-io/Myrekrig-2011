
/* Eriks gitter-mur-myre.
 * 9/1-02 - ?
 */

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// INTERFACE
#define debug(a,b...) printf(a "\n",b);

#include "Myre.h"

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og KAK-DEFs
typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, 
              CARRY=8, BUILDBASE=16} Retning;
typedef enum {
    stINIT=0,
    stTRANSP,
    stFOOD,
    stWALL0,
    stWALL,
    stLURK,
    stSOLDIER,
    stQUEEN
} State;

#define STIPLET_BITS 3
#define STIPLET ((1<<STIPLET_BITS)-1)

#define RX SKAK_RX
#define RY SKAK_RY
#define TURN SKAK_TURN
#define Mind SKAK_Mind
#define delay SKAK_delay
#define goTo SKAK_goTo

enum { FALSE, TRUE };
const int RX[] = {0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0};
const int RY[] = {0, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1};
const int TURN[] = {0, 2,3,4,1, 6,7,8,5, 10,11,12,9, 14,15,16,13};

typedef struct Mind {
    short mx,my;   //4: Besked (om position) ; Random.
    short px,py;   //4: Position
    unsigned char state;    //1: Myretype
    unsigned char id;       //1: Identitet
    //char flags; char pos;
    unsigned short pos;
} Mind;

//ID-felter:
#define idX 32
#define idY 64
#define idMISC 128
//#define idMISC 16
#define ID_POS (m->id & 15)
#define ID_X (m->id & idX)
#define ID_Y (m->id & idY)
#define ID_MISC (m->id & idMISC)

#define quCOUNT id
#define flKNOWSFOOD 128
#define KNOWS_FOOD (m->state & flKNOWSFOOD)

void delay() {
    int i,j,k;
    for (i=0; i<50*5; i++)
        for (j=0; j<1000; j++)
            k = i*j;
}


Retning goBackFollowingWall(Mind* m) {
    if ((m->px&63) == 0) {
        if ((m->py&63) == 0) { // På gitterhjørne
            if ((ID_MISC && m->py) || !m->px)
                return (m->py > 0)? SOUTH : NORTH;
            else
                return (m->px > 0)? WEST : EAST;
        }
        return (m->py > 0)? SOUTH : NORTH;
    } else if ((m->py&63) == 0) {
        return (m->px > 0)? WEST : EAST;
    } else {
        int dx = ((m->px-32)&63)-32;
        int dy = ((m->py-32)&63)-32;
        if (abs(dx) < abs(dy))
            return (dx > 0)? WEST : EAST;
        else
            return (dy > 0)? SOUTH : NORTH;
    }
}

Retning goToFollowingWall(Mind* m, int dstx, int dsty) {
    int dx = m->px - dstx;
    int dy = m->py - dsty;
    if (!dx)
        return (dy > 0)? SOUTH : NORTH;
    if (!dy)
        return (dx > 0)? WEST : EAST;

    if ((m->px&63) == 0) {
        if ((m->py&63) == 0) { // På gitterhjørne
            if ((ID_MISC && (m->py != dsty)) || (m->px == dstx))
                return (m->py > dsty)? SOUTH : NORTH;
            else
                return (m->px > dstx)? WEST : EAST;
        }
        return (m->py > dsty)? SOUTH : NORTH;
    } else if ((m->py&63) == 0) {
        return (m->px > dstx)? WEST : EAST;
    } else {
        if (abs(dx) > abs(dy))
            return (dx > 0)? WEST : EAST;
        else
            return (dy > 0)? SOUTH : NORTH;
    }
}

Retning myOutwards(Mind* m) {
    if (ID_MISC)
        return (ID_X)? EAST : WEST;
    else
        return (ID_Y)? NORTH : SOUTH;
}

Retning myInwards(Mind* m) {
    /*
    if (ID_MISC)
        return (ID_X)? WEST : EAST;
    else
        return  (ID_Y)? SOUTH : NORTH;
    */
    return goBackFollowingWall(m);
}

Retning goOutFollowingWall(Mind* m) {
    if ((m->px&63) == 0) {
        if ((m->py&63) == 0) { // På gitterhjørne
            return myOutwards(m);
        }
        return (m->py < 0)? SOUTH : NORTH;
    } else if ((m->py&63) == 0) {
        return (m->px < 0)? WEST : EAST;
    } else {
        if (abs(m->px) > abs(m->py))
            return (m->px < 0)? WEST : EAST;
        else
            return (m->py < 0)? SOUTH : NORTH;
    }
}

Retning goOutSlinger(Mind* m) {
    int *rnd = (int*) &m->mx;
    //int r = (m->mx<<16) + m->my;
    //r = r*3467+8234;
    *rnd = *rnd*4637+342;
    if (*rnd > (m->id<<24)) {
        *rnd = *rnd*4637+342;
        if (m->px > 0) return EAST;
        if (m->px < 0) return WEST;
        return (*rnd > 0)? EAST : WEST;
    } else {
        *rnd = *rnd*4637+342;
        if (m->py > 0) return NORTH;
        if (m->py < 0) return SOUTH;
        return (*rnd > 0)? NORTH : SOUTH;
    }
    //m->mx = r>>16; m->my = r;
}

int/*Retning*/ SkakMain(struct SquareData *sqr, Mind* m) {
    Retning dir = STOP;
    int i;
    int mcase = -1;

    //delay();
    
    switch (m->state &~ (128|64)) {
    case stINIT: {
        int queen = 0;
        m->id = m->mx;
        m->px = m->py = 0;
        
        for (i=1; i<sqr->NumAnts; i++) {
            if (m[i].state == stQUEEN) {
                queen = i; break;
            }           
        }
        if (!queen) {
            if (sqr->NumAnts > 1) {
                m->state = stQUEEN;
                m->quCOUNT = 0;
                break; //switch
            }
        }

        if (queen) {
            m[queen].quCOUNT++;
            /*
            if (queen && m[queen].quCOUNT >= 250) {
                m->state = stSOLDIER;
                goto initdone;
            } else
            */
            if (m[queen].quCOUNT > 5 && (m[queen].quCOUNT&2)) {
                m->state = stWALL0;
                goto initdone;
            }
        }
        m->state = stFOOD;
        m->pos = m[queen].quCOUNT>>2;
    initdone:
    } if (m->state != stFOOD) break;
    case stFOOD: {
        if (sqr->NumFood) { // Mad!
            if (sqr->NumFood>1 && !(KNOWS_FOOD)) {
                m->state |= flKNOWSFOOD;
                m->mx = m->px; m->my = m->py;
                m->pos = sqr->NumFood-1;
                // TODO - rygtespredningsmindskning
            }
            dir = goBackFollowingWall(m) | CARRY;
        } else { // Ik' mad
            // Se omkring for mad:
            for (i=1; i<=4; i++) if (sqr[i].NumFood > sqr[i].NumAnts) { 
                dir = i; goto fooddone;
            }

            if (KNOWS_FOOD) { // Gå til mad
                if (m->px == m->mx && m->py == m->my) { // Hvad - ikke mere mad!?
                    m->state &=~flKNOWSFOOD;
                } else {
                    // Fortæl videre:
                    for (i=1; i<sqr->NumAnts && m->pos > 1; i++) {
                        if (m[i].state == stFOOD) { //Kender ikke mad
                            m[i].state |= flKNOWSFOOD;
                            m[i].mx = m->mx; m[i].my = m->my;
                            m->pos -= (m[i].pos = m->pos>>1); // Del maden
                            i++;
                        }
                    }
                    
                    dir = goToFollowingWall(m, m->mx, m->my);
                    goto fooddone;
                }
            }
            
            /*
            if (sqr->NumAnts&1) m->state ^= 128; //Aie - randomishity
            if (sqr->NumAnts&2) m->state ^= 64; //Aie - randomishity
            */
            //Ellers afsøg:
            if (m->pos > 2) {
                int ax = abs(m->px), ay = abs(m->py);
                if (m->py == 0 && ax < 3*m->pos) {
                    if (m->px == 0) m->pos++;
                    dir = (ID_X)? EAST : WEST;
                } else {
                    if (ay < 3*m->pos && m->px)
                        dir = (ID_Y)? NORTH : SOUTH;
                    else
                        if (m->px)
                            dir = (m->px<0)? EAST : WEST;
                        else
                            dir = (m->py<0)? NORTH : SOUTH;
                }
            } else {
                dir = goOutSlinger(m);
            }
        }
    fooddone:
    } break;
    case stWALL0: { // Ikke placeret mur-myre
        bool found = FALSE;
        //printf("Uplaceret myre @ %d,%d\n", m->px, m->py);
        if ((m->px | m->py) && ((m->px&STIPLET)==0 && (m->py&STIPLET)==0)) {
            for (i=1; i<sqr->NumAnts; i++) {
                if (m[i].state == stWALL && m[i].pos) {
                    found = TRUE; break;
                }
            }
        } else found = TRUE;
        if (found) { // Gå videre
            int pp = (m->px | m->py);
            if ((pp&63) == 0 && (pp != 0)) {
                m->id += (m->id << 4) + (m->id >> 4);
                // Byg ikke mur indad:
                if (ID_X? (m->px<0):(m->px>0)) m->id ^= idX;
                if (ID_Y? (m->py<0):(m->py>0)) m->id ^= idY;
                //printf("Skiftede retning...\n");
            }
            dir = myOutwards(m);
            /*
            if (ID_MISC)
                dir = (ID_X)? EAST : WEST;
            else
                dir = (ID_Y)? NORTH : SOUTH;
            */
        } else { // Her er min plads!
            m->state = stWALL;
            m->pos = (m->px>>STIPLET_BITS)<<8 | ((m->py>>STIPLET_BITS)&0xFF);
            /*
            printf("Assigned %d,%d (%x,%x) (%x)\n",
                   m->px, m->py, m->px, m->py, m->pos);
            */
            //m->pos = abs(m->px) + abs(m->py);
            dir = STOP;
        }
    } if (m->state != stWALL) break;
    case stWALL: { // Mur-myre på plads
        short wx, wy;//ax, ay;
        mcase = 0;

        // Flyt mad?
        if (sqr->NumFood) {
            mcase = 1;
            dir = myInwards(m) | CARRY;
            goto walldone;
        }

        // Se omkring for mad:
        for (i=1; i<=4; i++) if (sqr[i].NumFood > sqr[i].NumAnts) { 
            dir = i; goto walldone;
        }

        // Ude af plads?
        //wx = m->pos>>6; wy = (m->pos<<8)>>6;
        if (m->px != (wx = ((signed short)m->pos>>(8-STIPLET_BITS))&~STIPLET)) {
            mcase = 2;
            dir = (m->px > wx)? WEST : EAST;
        } else if (m->py != (wy = ((short)(m->pos<<8))>>(8-STIPLET_BITS))) {
            mcase = 3;
            dir = (m->py > wy)? SOUTH : NORTH;
        } else {
            dir = STOP;
        }

        // Find ny plads? (TEST)
        for (i=1; i<sqr->NumAnts; i++ ) {
            if (m[i].state == stWALL && m[i].pos == m->pos) {
                m->state = stWALL0; break;
            }
        }
    walldone:
    } break;
    case stLURK: {
        dir = STOP;
        if (!sqr->NumFood) {
            // Se omkring for mad:
            for (i=1; i<=4; i++) if (sqr[i].NumFood > sqr[i].NumAnts) { 
                dir = i; break;
            }
        }
        if (--m->pos == 0) m->state = stSOLDIER;
    } break;
    case stSOLDIER: {
        if (abs(m->px) < 64 && abs(m->py) < 64) {
            dir = goOutSlinger(m);
        } else if (((++m->pos)&15) == 0) {
            dir = goOutSlinger(m);
        } else
            dir = STOP;
    } break;
    case stQUEEN: {
        dir = STOP;
    } break;
    } //switch

    // Enemies?
    for (i=1; i<=4; i++) {
        if (sqr[i].Team != 0) {
            dir = i;
            if (!m->state != stWALL) {
                m->state = stLURK;
                m->pos = abs(m->px)+abs(m->py);
            }
            break;
        }
    }

    m->px += RX[dir];
    m->py += RY[dir];
    
    /*
    if (m->px > 200 && dir == (EAST|CARRY))
        printf("Escaping: %d,%d %d,%d, (%d)\n", m->px, m->py, m->mx, m->my, m->state);
    */

    /*
    if (m->px > 200 && (dir&7) == EAST)
        printf("Escaping: %d,%d %d,%d, %d (%d/%d)\n",
               m->px,m->py, m->mx,m->my,
               m->pos, m->state, mcase);
    */
    return dir;
}

DefineAnt(Skak, "Skak", SkakMain, struct Mind)
