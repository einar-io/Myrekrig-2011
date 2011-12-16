/* Eriks mur-myre.
 * 5/1-02 - ?
 */

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// INTERFACE
#define debug(a,b...) printf(a "\n",b);

#include "Myre.h"

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og KAK-DEFs
typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, 
              CARRY=8, BUILDBASE=16} Retning;
typedef enum {
    stINIT=0,
    stFOOD,
    stWALLINIT, stWALLCALI, stWALL,
    stLURK,
    stPIONEER,
    stSIGNALATTACK,
    stATTACK
} State;

#define WALL_Y 0
#define WALL_X 0
//#define SPIRAL_SPEED 200 //I forhold til 128
//#define SPIRAL_SPEED2 (128*128/SPIRAL_SPEED)
#define BASE_DIST 96

#define RX KAK_RX
#define RY KAK_RY
#define TURN KAK_TURN
#define random KAK_random
#define delay KAK_delay
#define goTo KAK_goTo
#define goTo2 KAK_goTo2
#define goToFollowingWall KAK_goToFollowingWall
#define goHome KAK_goHome
#define goOut KAK_goOut
#define goOutSlinger KAK_goOutSlinger
#define goOutSpir KAK_goOutSpir
#define goOutSpir2 KAK_goOutSpir2
#define goOutSpir3 KAK_goOutSpir3
#define initWall KAK_initWall
#define initLurk KAK_initLurk
#define takeHitchHiker KAK_takeHitchHiker

#ifndef SIGNAL_ATTACK_TRESHOLD
#define SIGNAL_ATTACK_TRESHOLD 23//18
#endif
#ifndef ATTACK_GO_EAST
#define ATTACK_GO_EAST 15
#endif
#ifndef SLINGER_TRESH
#define SLINGER_TRESH 1000//140//50
#endif
#ifndef SPIRAL_SPEED
#define SPIRAL_SPEED 200 //I forhold til 128
#endif

#define SPIRAL_SPEED2 (128*128/SPIRAL_SPEED)

const int RX[] = {0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0};
const int RY[] = {0, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1};
const int TURN[] = {0, 2,3,4,1, 6,7,8,5, 10,11,12,9, 14,15,16,13};

#define FOREACH_R for(r=1; r<=4; r++)
typedef struct Mind {
    u_long rnd;    //4
    short px,py;   //4
    short mx,my;   //4
    char state; char id;
    short pos;
    //unsigned short int rnd;
} Mind;

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//


void delay() {
    int i,j,k;
    for (i=0; i<50; i++)
        for (j=0; j<1000; j++)
            k = i*j;
}

int random(Mind* m) {
    m->rnd = m->rnd*4637+342;
    return m->rnd;    
}

Retning goOut_(Mind* m) {
    int dx = m->px, dy = m->py;
    int ax = abs(m->px), ay = abs(m->py);

    if (ax+ay>= 25 *(1+(m->id&7))) {
        int a = ax+ay + (m->rnd&7);
        dx += (a*m->py)/8;
        dy -= (a*m->px)/8;
    } else {
        int r = random(m);
        dx += (r&255)-128; dy += (r>>24);
    }
    if (abs(dx)>abs(dy))
        return dx>0? EAST : WEST;
    else
        return dy>0? NORTH : SOUTH;
}

Retning goOutSpir(Mind* m) {
    if (m->id & 16) {
        if (m->px > 0) {
            if (m->py > 0) { //NE
                int a = ((random(m)&255)*abs(m->py) > SPIRAL_SPEED2*abs(m->px));
                return a? EAST: SOUTH;
            } else { //SE
                int a = ((random(m)&255)*abs(m->py) > SPIRAL_SPEED*abs(m->px));
                return a? WEST : SOUTH;
            }
        } else {
            if (m->py > 0) { //NW
                int a = ((random(m)&255)*abs(m->py) > SPIRAL_SPEED*abs(m->px));
                return a? EAST: NORTH;
            } else { //SW
                int a = ((random(m)&255)*abs(m->py) > SPIRAL_SPEED2*abs(m->px));
                return a? WEST : NORTH;
            }
        }
    } else {
        if (m->px > 0) {
            if (m->py > 0) { //NE
                int a = ((random(m)&255)*abs(m->py) > SPIRAL_SPEED*abs(m->px));
                return a? WEST : NORTH;
            } else { //SE
                int a = ((random(m)&255)*abs(m->py) > SPIRAL_SPEED2*abs(m->px));
                return a? EAST : NORTH;
            }
        } else {
            if (m->py > 0) { //NW
                int a = ((random(m)&255)*abs(m->py) > SPIRAL_SPEED2*abs(m->px));
                return a? WEST : SOUTH;
            } else { //SW
                int a = ((random(m)&255)*abs(m->py) > SPIRAL_SPEED*abs(m->px));
                return a? EAST : SOUTH;
            }
        }
    }
}

Retning goOutSlinger(Mind* m) {
    if ((m->id<<24) > (random(m)))
        /*
        ((m->id & 127)*4*xx+((signed) rnd(m))%10 < (yy << 4)) :
        ((m->id & 127)*4*yy+((signed) rnd(m))%10 > (xx << 4)))
        */
    {
        if (m->px > 0) return EAST;
        if (m->px < 0) return WEST;
        return (random(m) & 2)? EAST : WEST;
    } else {
        if (m->py > 0) return NORTH;
        if (m->py < 0) return SOUTH;
        return (random(m) & 2)? NORTH : SOUTH;
    }
}

Retning goOut(Mind* m) {
    int dx = m->px, dy = m->py;
    int ax = abs(m->px), ay = abs(m->py);

    if (m->id&32 && ax+ay>=20 /*ax+ay >= SLINGER_TRESH*/) {
        return goOutSlinger(m);
    } else {
        int r = random(m);
        dx += (r&255)-128; dy += (r>>24);
    }
    if (abs(dx)>abs(dy))
        return dx>0? EAST : WEST;
    else
        return dy>0? NORTH : SOUTH;
}

Retning goOutSpir2(Mind* m) {
    int ax = abs(m->px), ay = abs(m->py);
    if (ax+ay < SLINGER_TRESH) return goOutSlinger(m); else return goOutSpir(m);
}

Retning goOutSpir3(struct SquareData *sqr, Mind* m) {
    if (m->id & 32) {
        if (sqr->NumAnts >= 2 || (m->px==0 && m->py==0)) {
            if (m->px == 0 && m->id&4)
                return m->id&1? NORTH : SOUTH;
            if (m->py == 0)
                return m->id&1? EAST : WEST;
        }
    }
    return goOutSpir2(m);
}

Retning goTo(Mind* m, int dstx, int dsty) {
    int dx = m->px - dstx, dy = m->py - dsty;
    if (!dx && !dy) return STOP;
    if (abs(dx)>abs(dy))
        return dx>0? WEST : EAST;
    else
        return dy>0? SOUTH : NORTH;
}

Retning goToFollowingWall(Mind* m, int dstx, int dsty) {
    if (m->px == dstx && m->py == dsty) return STOP;
    if (abs(dstx) > abs(dsty)) { //Horizontal wall
        if (m->px != dstx)
            return m->px > dstx? WEST : EAST;
        else
            return m->py > dsty? SOUTH : NORTH;
    } else {
        if (m->py != dsty)
            return m->py > dsty? SOUTH : NORTH;
        else
            return m->px > dstx? WEST : EAST;
    }
}


Retning goTo2(Mind* m, int dstx, int dsty) {
    int dx = m->px - dstx, dy = m->py - dsty;
    if (!dx && !dy) return STOP;
    if (abs(dx)>abs(dy) && dy || !dx) {
        return dy>0? SOUTH : NORTH;
    } else {
        return dx>0? WEST : EAST;
    }
}

Retning goHome(Mind* m) {
    return goTo2(m, 0, 0);
}

void initWall(Mind* m) {
    m->state = stWALLINIT;
    m->mx = m->my = m->pos = 0;
}

void initLurk(Mind* m) {
    m->state = stLURK;
    m->pos = 2000; //Lurk time
}

Retning takeHitchHiker(struct SquareData *sqr, Mind* m) {
    int i;
    for (i=1; i<sqr->NumAnts; i++) {
        if (0/*TEST*/ && m[i].state == stFOOD && (m[i].mx || m[i].my)) {
            Mind tmp = *m;
            *m = m[i];
            m[i] = tmp;
            //printf("Hitchhike! (%d,%d)\n", m->px, m->py);
            return goToFollowingWall(m, m->mx, m->my);
        }
    }
    return STOP;
}

int/*Retning*/ KakaoMain(struct SquareData *sqr, Mind* m) {
    Retning dir = STOP;
    int i;

    //delay();

    switch (m->state) {
    case stINIT: {
        m->px = m->py = 0;
        m->id = m->rnd;
        m->state = stFOOD;
        /*
        if (sqr->NumAnts > 10) {
            //printf("On base: %d\n", sqr->NumAnts);
            m->state = stPIONEER;
            dir = EAST;
        }
        */
        if (sqr->NumAnts > SIGNAL_ATTACK_TRESHOLD) {
            //printf("On base: %d\n", sqr->NumAnts);
            m->state = stSIGNALATTACK;
            m->pos = 0;
            dir = EAST;
        }       
    } //fall-through
    case stFOOD: {
        if (sqr->NumFood >= sqr->NumAnts) { // Mad her
            m->state = stFOOD;//?
            if (!(m->mx || m->my)) {
                m->mx = m->px; m->my = m->py; m->pos = sqr->NumFood-sqr->NumAnts; //TTL
                //printf("Set foodpos to %d,%d\n", m->mx, m->my);
            } else if (sqr->NumFood == 1 && m->mx == m->px && m->my == m->py) {
                //Hook 3
                initWall(m);
            }
            dir = goHome(m) | CARRY;
        } else { // Ingen mad her
            if (m->mx || m->my) {
                dir = goToFollowingWall/*TEST*/(m, m->mx,m->my);
                if (dir == STOP) { // Mad slap op
                    //printf("Reset foodpos from %d,%d\n", m->mx, m->my);
                    m->mx = m->my = 0; // Glem mad-pos
                    dir = goOut(m); //Hook 1
                    if (random(m)&4) initWall(m);
                } else {
                    // Fortæl videre:
                    int i;
                    if (m->pos >= 2) for (i=1; i<sqr->NumAnts; i+=2)
                        if (m[i].state == stFOOD && !(m[i].mx || m[i].my)) {
                            m[i].mx = m->mx; m[i].my = m->my;
                            m[i].pos = m->pos = m->pos-1/* /2*/; //TTL
                        }
                }
            } else {
                Retning r;
                FOREACH_R {
                    if (sqr[r].NumFood > sqr[r].NumAnts)
                        dir = r; // Found food
                }
                if (!dir) dir = goOutSpir2(m); // Spir2(m);
                //if (sqr->NumAnts >= 20) {//Hook 2 }
            }
        }
    } break;
    case stWALLINIT: {
        dir = goHome(m);
        if (dir) {
            dir |= CARRY; break;
        } else
            m->state = stWALLCALI;
    } //fall-through
    case stWALLCALI: {
        if (m->id&1) { // Horizontal wall
            if (m->py != WALL_Y) { dir = m->py>WALL_Y? SOUTH : NORTH; goto donecali; } // On line
            
            if (m->px == 0) { dir = m->id&2 ? EAST : WEST; goto donecali; }
            if (m->px > 0) { // Eastern wall
                int i;
                for (i=1; i<sqr->NumAnts; i++)
                    if (m[i].state == stWALL || m[i].state == stWALLCALI) {
                        if (m[i].px < 0) { m->state = stFOOD; goto donecali; }
                        dir = EAST; goto donecali;
                    }
            } else { // Western wall
                int i;
                for (i=1; i<sqr->NumAnts; i++)
                    if (m[i].state == stWALL || m[i].state == stWALLCALI) {
                        if (m[i].px > 0) { m->state = stFOOD; goto donecali; }
                        dir = WEST; goto donecali;
                    }
            }

            // Wall Complete?
            if (m->px && sqr->Base) {
                m->state = stFOOD; m->px = m->py = 0;
                goto donecali;
            }

            // Assigning wall position:
            m->state = stWALL;
            m->pos = m->px;
        } else { // Vertical wall
            if (m->px != WALL_X) { dir = m->px>WALL_X? WEST : EAST; goto donecali; } // On line
            
            if (m->py == 0) { dir = m->id&2 ? NORTH : SOUTH; goto donecali; }
            if (m->py > 0) { // Northern wall
                int i;
                for (i=1; i<sqr->NumAnts; i++)
                    if (m[i].state == stWALL || m[i].state == stWALLCALI) {
                        if (m[i].py < 0) { m->state = stFOOD; goto donecali; }
                        dir = NORTH; goto donecali;
                    }
            } else { // Southern wall
                int i;
                for (i=1; i<sqr->NumAnts; i++)
                    if (m[i].state == stWALL || m[i].state == stWALLCALI) {
                        if (m[i].py > 0) { m->state = stFOOD; goto donecali; }
                        dir = SOUTH; goto donecali;
                    }
            }

            // Wall Complete?
            if (m->py && sqr->Base) {
                m->state = stFOOD; m->px = m->py = 0;
                goto donecali;
            }

            // Assigning wall position:
            m->state = stWALL;
            m->pos = m->py;
            // done
        }
    donecali:
    } if (m->state != stWALL) break;
    case stWALL: {
        if (m->id&1) { // Horizontal wall
            if (m->py != WALL_Y) { // Out of line?
                dir = (m->py>WALL_Y? SOUTH : NORTH) | CARRY; goto walldone;
            }
            if (m->px == 0 && m->my) {
                int i;
                // Fortæl om mad:
                for (i=1; i<sqr->NumAnts; i+=2) {
                    if (m[i].state == stFOOD && !(m[i].mx || m[i].my)) {
                        m[i].mx = m->mx; m[i].my = m->my; m[i].pos = 4; //TTL
                    }
                }
                m->mx = m->my = 0; //Glem mad igen.
            }
            if (sqr->NumFood) {
                //Hvorfra kommer maden?
                for (i=1; i<sqr->NumAnts; i++)
                    if ((m[i].state == stFOOD || m[i].state == stWALL) && 
                         (m[i].mx || m[i].my))
                    {
                        m->mx = m[i].mx; m->my = m[i].my;
                        if (m[i].state == stWALL) { m[i].mx = m[i].my = 0; } // Glem
                        break;
                    }
                if (m->px > 0) { // Eastern wall
                    dir = WEST | CARRY; goto walldone;
                } else if (m->px < 0) { // Western wall
                    dir = EAST | CARRY; goto walldone;
                }
            }
            if (m->px < m->pos) { dir = EAST; goto walldone; }
            if (m->px > m->pos) { dir = WEST; goto walldone; }
            
            if (sqr[NORTH].NumFood > sqr[NORTH].NumAnts) { dir = NORTH; goto walldone; }
            else if (sqr[SOUTH].NumFood > sqr[SOUTH].NumAnts) { dir = SOUTH; goto walldone; }

            dir = takeHitchHiker(sqr, m);
        } else { // Vertical wall
            if (m->px != WALL_X) { // Out of line?
                dir = (m->px>WALL_X? WEST : EAST) | CARRY; goto walldone;
            }
            if (m->py == 0 && m->mx) {
                int i;
                // Fortæl om mad:
                for (i=1; i<sqr->NumAnts; i++)
                    if (m[i].state == stFOOD && !(m[i].mx || m[i].my)) {
                        m[i].mx = m->mx; m[i].my = m->my; m[i].pos = 4; //TTL
                    }
                m->mx = m->my = 0; //Glem mad igen.
            }
            if (sqr->NumFood) {
                //Hvorfra kommer maden?
                for (i=1; i<sqr->NumAnts; i+=2)
                    if ((m[i].state == stFOOD || m[i].state == stWALL) && 
                         (m[i].mx || m[i].my))
                    {
                        m->mx = m[i].mx; m->my = m[i].my;
                        if (m[i].state == stWALL) { m[i].mx = m[i].my = 0; } // Glem
                        break;
                    }
                if (m->py > 0) { // Northern wall
                    dir = SOUTH | CARRY; goto walldone;
                } else if (m->py < 0) { // Southern wall
                    dir = NORTH | CARRY; goto walldone;
                }
            }

            //Tilbage på plads:
            if (m->py < m->pos) { dir = NORTH; goto walldone; }
            if (m->py > m->pos) { dir = SOUTH; goto walldone; }
            
            // Tag mad ved siden af mur:
            if (sqr[EAST].NumFood > sqr[EAST].NumAnts) { dir = EAST; goto walldone; }
            else if (sqr[WEST].NumFood > sqr[WEST].NumAnts) { dir = WEST; goto walldone; }

            dir = takeHitchHiker(sqr, m);
        }
    walldone:
    } break;
    case stLURK: {
        dir = STOP;
        if (--m->pos == 0 || sqr->NumFood>=sqr->NumAnts) m->state = stFOOD;
    } break;
    case stPIONEER: {
        int i;
        //printf("Got %d food, %d ants\n", sqr->NumFood, sqr->NumAnts);
        /*if (m->id&1)*/ { // Horizontal
            if (m->id&2) { // East
                dir = goToFollowingWall(m, BASE_DIST, 0) | CARRY;
                if (dir == (STOP|CARRY)) {
                    if (sqr->Base) {m->px = m->py = 0; m->state = stFOOD; }
                    else dir = BUILDBASE;
                }
                //printf("dir=%d\n", dir);

                /*if (m->px > BASE_DIST/2)*/ {
                    for (i=1; i<sqr->NumAnts; i++) if (m[i].state == stWALL) {
                        //m[i].px = BASE_DIST-m[i].px;
                        m[i].state = m->state; m[i].id = m->id;
                    }
                }
            } else {
                dir = goToFollowingWall(m, -BASE_DIST, 0) | CARRY;
                if (dir == (STOP|CARRY)) {
                    if (sqr->Base) {m->px = m->py = 0; m->state = stFOOD; }
                    else dir = BUILDBASE;
                }
                /*if (m->px < -BASE_DIST/2)*/ {
                    for (i=1; i<sqr->NumAnts; i++) if (m[i].state == stWALL) {
                        //m[i].px = -BASE_DIST-m[i].px;
                        m[i].state = m->state; m[i].id = m->id;
                    }
                }
            }
        }
    } break;
    case stSIGNALATTACK: {
        if (m->pos == 0) {
            int found = 0;
            for (i=1; i<sqr->NumAnts; i++) if (m[i].state == stWALL) {
                found = 1; break;
            }
            if (found && m->px>0)
                dir = EAST;
            else
                m->pos = m->px; dir = WEST;
        } else {
            for (i=1; i<sqr->NumAnts; i++) if (m[i].state == stWALL) {
                m[i].state = stATTACK;
                break; //Skræl kun ét lag af(?)
            }
            dir = WEST;
        }
    } break;
    case stATTACK: {
        m->id ^=128;
        if (m->id & 128) {
            //printf("AGE = %d\n", ATTACK_GO_EAST);
            if (((m->px+m->py)&ATTACK_GO_EAST) == 0)
                dir = EAST;
            else
                dir = NORTH;
        } else
            dir = STOP;
    } break;
    } // switch

    // Enemies?
    for (i=1; i<=4; i++) {
        if (sqr[i].Team != 0) {
            dir = i;
            if (!sqr->NumFood && m->state != stWALL) initLurk(m);
            break;
        }
    }

    if ((dir&CARRY) && (m->px||m->py) &&
        (m->px%BASE_DIST) == 0 && (m->py%BASE_DIST) == 0)
    {
        for (i=1; i<sqr->NumAnts; i++) if (m[i].state == stPIONEER) {
            dir &=~ CARRY; break;
        }       
    }

    m->px += RX[dir];
    m->py += RY[dir];
    return dir;
}

DefineAnt(Kakao, "Kakao#C46B00", KakaoMain, struct Mind)
