/* Videreudvikling af Tirsdag.
 * 16/10-01 - ?.
 */

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// INTERFACE
#define debug(a,b...) printf(a "\n",b);

#include "Myre.h"

//#include "math.h"

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og PUNK-DEFs
typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, CARRY=8} Retning;
typedef enum {
    stINIT=0,
    stGETFOOD, stRETURNFOOD,
    stWALL0, stWALLINIT, stWALL,
    stLURK, stSOLDIER
} State;

#define RX PUNK_##RX
#define RY PUNK_##RY
#define goOut PUNK_##goOut
#define goHome PUNK_##goHome
#define goTo PUNK_##goTo
#define goRound PUNK_##goRound
#define delay PUNK_##delay
#define rnd PUNK_##rnd

const int RX[] = {0, 1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0};
const int RY[] = {0, 0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0};

typedef struct Mind {
    u_long id; 
    short px,py;
    short mx,my;
    unsigned char state, data;
    unsigned char rnd;
} Mind;

/*
#define GET_WALL_INDIR(m) ( ((m).data&3)+1 )
#define GET_WALL_OUTDIR(m) ( (((m).data >> 2)&3)+1 )
#define SET_WALL_INDIR(m,d) ( (m).data = (((m).data&~3))|((d)-1) )
#define SET_WALL_OUTDIR(m,d) ( (m).data = (((m).data&~12))| (((d)-1)<<2) )

#define WALL_LONGEVITY(m) 255 //( 4*(abs((m).px)+abs((m).py)) )
*/

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
    /*
    if ((m->id & 1024)? 
        ((m->id & 1023)*xx <= (yy << 8)) :
        ((m->id & 1023)*yy >= (xx << 8)))
    */
    /*
    if ((m->id & 128)?
        ((m->id & 127)*xx <= (yy << 7)) :
        ((m->id & 127)*yy >= (xx << 7)))
    */
    if ((m->id & 1024)? 
        ((m->id & 1023)*xx <= (yy << 10)) :
        ((m->id & 1023)*yy >= (xx << 10)))
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
    /*
    int xx=abs(m->px), yy=abs(m->py);

    if (m->py == 0 || (rnd(m) % (xx+yy)) < xx) {
        return (m->px < 0)? EAST : WEST;
    } else {
        return (m->py < 0)? NORTH : SOUTH;
    }
    */
    if (!m->py) return (m->px < 0)? EAST : WEST;
    if (!m->px) return (m->py < 0)? NORTH : SOUTH;
    if (m->px&7) return (m->px < 0)? EAST : WEST;
    if (m->py&7) return (m->py < 0)? NORTH : SOUTH;
    if ((m->px+m->py)&8) {
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


int/*Retning*/ PunkMain(struct SquareData *sqr, Mind* m) {
    Retning dir = STOP;
    int i;

    //delay();

    switch (m->state) {
    case stINIT: {
        m->rnd = m->id;
        m->state = stGETFOOD;
    } if (m->state != stGETFOOD) break; // else continue:
    case stGETFOOD: {
        if (sqr->NumFood) {
            m->state = stRETURNFOOD;
            if (!(m->mx|m->my) && sqr->NumFood>1) {
                m->mx = m->px; m->my = m->py;
                m->data = sqr->NumFood;
            } else if (m->px == m->mx && m->py == m->my && m->data>0)
                m->data--;
            //continue
        } else {
            if (m->mx || m->my) { // Ved hvor der er mad:
                if (m->px == m->mx && m->py == m->my) {
                    m->mx = m->my = 0;
                    if (rnd(m)&8) m->state = stWALL0;
                } else {
                    dir = goTo(m, m->mx, m->my);
                }
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
    case stLURK: {
        dir = STOP;
        if (--m->data == 0) m->state = stSOLDIER;
        if (sqr->NumFood) {
            m->state = stRETURNFOOD;
            if (sqr->NumFood>1) { m->mx=m->px; m->mx=m->py; }
            else { m->mx = m->my = 0; }
        }
        // Bo-ring!
    } break;
    case stSOLDIER: {
        if ((++m->data&15) == 0)
            dir = goOut(m);
        if (sqr->NumFood) {
            m->state = stRETURNFOOD;
            if (sqr->NumFood>1) { m->mx=m->px; m->mx=m->py; }
            else { m->mx = m->my = 0; }
        }
    } break;
    case stWALL0: {
        if (m->px==0 && m->py==0) {
            m->state = stWALLINIT;
            dir = ((m->id>>1)&3)+1;
        } else {
            dir = goHome(m);
        }
    } if (m->state != stWALLINIT) break;
    case stWALLINIT: {
        int i, wall = 0;
        for (i=1; i<sqr->NumAnts; i++) {
            if (m[i].state == stWALL) wall = i;
        }
        if (wall) {
            if (m->px) dir = (m->px>0)? EAST : WEST;
            else if (m->py) dir = (m->py>0)? NORTH : SOUTH;
            else dir = (m->id&3)+1;
        } else if (m->px|m->py) {
            m->state = stWALL;
            m->mx = m->px; m->my = m->py;
        } else dir = goOut(m);
    } if (m->state != stWALL) break;
    case stWALL: {
        if (sqr->NumFood) {
            dir = goHome(m) | CARRY;
            
            /*if (rnd(m)&32)*/ { // Gå fra mur til madhenter:
                for (i=1; i<sqr->NumAnts; i++) {
                    //if (m->py<0) printf("mur->madhenter %d,%d\n", m->px, m->py);
                    if (m[i].state == stRETURNFOOD && (m[i].mx||m[i].my) && m->data>1) {
                        m->state = stRETURNFOOD;
                        m->mx = m[i].mx; m->my = m[i].my;
                        //m[i].data -= (m->data = m[i].data/2);
                        m->data = m[i].data-1;
                        m[i].data = 1;
                        break;
                    }
                }
            }
            goto walldone;
        }
        if (m->px != m->mx || m->py != m->my) {
            dir = goTo(m, m->mx, m->my);
            goto walldone;
        } else {
        }
        walldone:
    } break;
    default:
    } // switch

    if (m->state != stWALL) {
        int mydist = abs(m->mx)+abs(m->my);
        if (mydist>0 && m->data>1) {
            for (i=1; i<sqr->NumAnts; i+=2) if (m[i].state != stWALL) {
                Mind* mp = &((Mind*)m)[i];
                int yourdist = abs(mp->mx)+abs(mp->my);
                if (yourdist==0) { //closer?
                    // Tell:
                    mp->mx = m->mx; mp->my = m->my;
                    m->data -= (mp->data = m->data/2);
                }
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
            m->data = 255;
            break;
        }
    }

    m->px += RX[dir];
    m->py += RY[dir];
    return dir;
}

DefineAnt(Punk, "Punk#FF28E6", PunkMain, struct Mind)
