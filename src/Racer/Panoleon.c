/* Eriks myre myre.
 * 30/11 2000-?
 */

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// INTERFACE
#define debug(a,...)

#include "MyreKrig.h"

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og PAN-DEFs
#define GENR_FOOD_MEM 10
#define SKAF_FOOD_MEM 1
#define ENEMY_MEM 5
#define PHASE1 100
#define ARMY_SIZE 25
#define SPIRAL_SPACING 20
#define SPIRAL_RADIUS 10
#define BASE_DIST 64
#define BASE_TRESH 128


#define Retning PAN_retning
#define STOP PAN_STOP
#define HERE PAN_HERE
#define EAST PAN_EAST
#define SOUTH PAN_SOUTH
#define WEST PAN_WEST
#define NORTH PAN_NORTH

#define State PAN_State
#define GENRAL PAN_GENRAL
#define SKAFFER PAN_SKAFFER
#define LURKER PAN_LURKER
#define SOLDIER PAN_SOLDIER

#define RX PAN_RX
#define RY PAN_RY

#define Genral_Mind PAN_Genral_Mind
#define Skaffer_Mind PAN_Skaffer_Mind
#define Soldier_Mind PAN_Soldier_Mind
#define Mind PAN_Mind

#define getRandom PAN_getRandom
#define goTo PAN_goTo
#define goToRand PAN_goToRand
#define goToRand2 PAN_goToRand2
#define goRound PAN_goRound
#define goRound2 PAN_goRound2
#define goRound10 PAN_goRound10
#define MakeGeneral PAN_MakeGeneral
#define MakeSkaffer PAN_MakeSkaffer
#define MakeLurker PAN_MakeLurker
#define MakeSoldier PAN_MakeSoldier

#define InitCommand PAN_InitCommand
#define SkafferCommand PAN_SkafferCommand
#define SoldierCommand PAN_SoldierCommand

#define GenralMain PAN_GenralMain
#define SkafferMain PAN_SkafferMain
#define LurkerMain PAN_LurkerMain
#define SoldierMain PAN_SoldierMain

#define FORHVERGAAHVIS(bt, extra)\
 {Retning r;  struct SquareData* rr;\
  for (r=EAST; r<=NORTH; r++)\
  if (rr=&sqr[r], bt) {dir=r; extra; goto done;} }

#define SNAKMEDGENRAL(type)\
  {int i; gen=0;\
   for (i=1; i<sqr[HERE].NumAnts; i++) {\
      Mind* mp = &((Mind*)m)[i];\
     if (mp->basemind.state == GENRAL) {\
       type##Command( (Genral_Mind*)mp, (type##_Mind*)m);\
       gen=1;\
     }\
  } }


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH} Retning;
typedef enum {INIT, GENRAL, SKAFFER, LURKER, SOLDIER/*, STORMER*/ } State;

const int RX[] = {0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0, 1,0,-1,0};
const int RY[] = {0, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1, 0,-1,0,1};
const int RXX[] = {0,1,1,-1,-1};
const int RYY[] = {0,1,-1,-1,1};

#define MINDCOMMON   u_long id; long random; State state; int px,py; int count;

typedef struct {
    MINDCOMMON
} Basic_Mind;

typedef Basic_Mind Init_Mind;
typedef Basic_Mind Lurker_Mind;

typedef struct {
    MINDCOMMON
    short foodx[GENR_FOOD_MEM], foody[GENR_FOOD_MEM], foodcnt[GENR_FOOD_MEM];
    short srchx, srchy, srchx2, srchy2;
    short bases, basespos;
    short enemyx[GENR_FOOD_MEM], enemyy[GENR_FOOD_MEM], enemycnt[GENR_FOOD_MEM];
} Genral_Mind;

typedef struct {
    MINDCOMMON
    int foodx[SKAF_FOOD_MEM], foody[SKAF_FOOD_MEM], foodcnt[SKAF_FOOD_MEM];
    int srchx, srchy;
    int reached;
    int enemyx, enemyy, enemymet;
} Skaffer_Mind;

typedef struct {
    MINDCOMMON
    int srchx, srchy;
    int reached;
} Soldier_Mind;

typedef union {
    Basic_Mind basemind;
    Genral_Mind genr;
    Skaffer_Mind skaf;
} Mind;


#undef mindcommon

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
void InitCommand(Genral_Mind* g, Basic_Mind* m);
void SkafferCommand(Genral_Mind* g, Skaffer_Mind* m);
void SoldierCommand(Genral_Mind* g, Soldier_Mind* m);
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

long getRandom(Basic_Mind* mind) {
#define R (mind->random)
  R = (((R << 4) + (R>>3)) ^ R) + 0x64374785;
  return R;
#undef R;
}


Retning goTo(int x,int y, int dstx, int dsty) {
    int dx = dstx-x;  int dy = dsty-y; 
    if (abs(dx)>abs(dy)) {
        return (dx > 0) ? EAST: WEST;
    } else {
        if (dy == 0) return STOP;
        return (dy > 0) ? NORTH: SOUTH;
    }
}

Retning goToRand(int x,int y, int dstx,int dsty, Basic_Mind* m) {
    if (getRandom(m) & 8) return goTo(x,y,dstx,dsty);

  if ((getRandom(m) & 4) && (x != dstx)) {
    return (x < dstx) ? EAST: WEST;
  } else {
    if (y != dsty) {
      return (y < dsty) ? NORTH: SOUTH;
    } else if (x != dstx) {
      return (x < dstx) ? EAST: WEST;
    } else {
      return STOP;
    }
  }
}
Retning goToRand2(int x,int y, int dstx,int dsty, Basic_Mind* m) {
  if ((getRandom(m) & 4) && (x != dstx)) {
    return (x < dstx) ? EAST: WEST;
  } else {
    if (y != dsty) {
      return (y < dsty) ? NORTH: SOUTH;
    } else if (x != dstx) {
      return (x < dstx) ? EAST: WEST;
    } else {
      return STOP;
    }
  }
}


Retning goRound(int x,int y, int dstx, int dsty) {
    int dx = dstx-x;  int dy = dsty-y; 

    if (dx>dy) {
        if (dx>-dy) return NORTH; else return EAST;
    } else {
        if (dx>=-dy) return WEST; else return SOUTH;
    }
}
Retning goRound2(int x,int y, int dstx, int dsty) {
    int dx = dstx-x;  int dy = dsty-y; 

    if (dx>dy) {
        if (dx>-dy) return NORTH; else return EAST;
    } else {
        if (dx>=-dy+2 ) return WEST; else return SOUTH;
    }
}
Retning goRound10(int x,int y, int dstx, int dsty, int k) {
    int dx = dstx-x;  int dy = dsty-y; 

    if (dx>dy) {
        if (dx>-dy) return NORTH; else return EAST;
    } else {
        if (dx>=-dy+k) return WEST; else return SOUTH;
    }
}

void MakeGeneral(Genral_Mind* g) {
    int i;
    memset(g,sizeof(*g), 0);
    g->state = GENRAL;
    g->srchx = g->srchy = 0;
    for (i=0; i<GENR_FOOD_MEM; i++)
        g->foodcnt[i] = 0;
}

void MakeSkaffer(Skaffer_Mind* m) {
    m->state = SKAFFER;
    m->srchx = m->srchx = 0;
}

void MakeLurker(Lurker_Mind* m) {
    m->state = LURKER;
    m->count = 1000;
}

void MakeSoldier(Soldier_Mind* m, int dc, int dstx, int dsty) {
    m->state = SOLDIER;
    m->count = 100+dc;
    m->srchx = dstx; m->srchy = dsty;
    m->reached = 1;
}

void InitCommand(Genral_Mind* g, Basic_Mind* m) {
    //int i;
    g->count++;
    /*
      for (i=0; i<ENEMY_MEM; i++) {
      if (g->enemycnt[i]) {
      debug("SOLDIER MAKE %d %d\n", i, g->enemycnt[i]);
      MakeSoldier((Soldier_Mind*)m, g->enemycnt[i],
      g->enemyx[i]+m->id&0x15-7,
      g->enemyy[i]+m->id&0x0a-7);
      g->enemycnt[i]--;
      return;
      }
      }
    */

    if (g->srchx*SPIRAL_SPACING>=BASE_TRESH && g->bases == 0) {
        g->bases = (NewBaseAnts+NewBaseFood) * 2;
        g->basespos = (g->basespos%4) + 1;
        debug("HOP");
    }

    m->state = SKAFFER;
    SkafferCommand(g,(Skaffer_Mind*) m);

    if (g->srchx*SPIRAL_SPACING>=BASE_TRESH && g->bases>0) {
        Skaffer_Mind* s = (Skaffer_Mind*) m;
        s->px -= BASE_DIST * RXX[g->basespos];
        s->py -= BASE_DIST * RYY[g->basespos];
        s->srchx -= BASE_DIST * RXX[g->basespos];
        s->srchy -= BASE_DIST * RYY[g->basespos];
        g->bases--;
    }
}

void SkafferCommand(Genral_Mind* g, Skaffer_Mind* m) {
    int dir, i, j;
    int opgivet = (m->reached<0);
    int told = 0;

    if (m->srchx || m->srchy) return; // Don't command - wrong general

    // Skaf -> Genral
    for (i=0; i<SKAF_FOOD_MEM; i++) {
        if (m->foodcnt[i]>0) {
            for (j=0; j<GENR_FOOD_MEM; j++)
                if (g->foodx[j] == m->foodx[i] && g->foody[j] == m->foody[i]) {
                    if (m->foodcnt[i] < g->foodcnt[j]) 
                        g->foodcnt[j] = m->foodcnt[i];
                    goto seen;
                }
            for (j=0; j<GENR_FOOD_MEM; j++)
                if (g->foodcnt[j] == 0) break;
            if (j<GENR_FOOD_MEM) { // G: Husk madpos.
                g->foodx[j] = m->foodx[i];
                g->foody[j] = m->foody[i];
                g->foodcnt[j] = m->foodcnt[j];
                told = 0;
                m->foodcnt[i] = 0;
                //return;
            }
        seen:
        }
        m->foodcnt[i] = 0;
    }

    // Fjender:
    if (m->enemymet) {
        for (j=0; j<ENEMY_MEM; j++) if (g->enemycnt[j])
            if (g->enemyx[j] == m->enemyx && g->enemyy[j] == m->enemyy) {
                goto seen2;
            }
        for (j=0; j<ENEMY_MEM; j++) if (! g->enemycnt[j]) {
            g->enemyx[j] = m->enemyx;
            g->enemyy[j] = m->enemyy;
            g->enemycnt[j] = ARMY_SIZE;
        }
    seen2:
    }

//    if (m->foodcnt[i]) return;

    // Genral -> Skaf
    for (i=0; i<GENR_FOOD_MEM; i++)
        if (g->foodcnt[i]>0) { // Send ud.
            m->srchx = g->foodx[i];
            m->srchy = g->foody[i];
            g->foodcnt[i]--;
            m->count = (g->foodcnt[i]<2)? 4 : 0;
            if (g->basespos < 0) debug("GHI %d,%d\n", m->srchx, m->srchy);
            return;
        }

    if (opgivet) {
        m->srchx = g->srchx2 * SPIRAL_SPACING;
        m->srchy = g->srchy2 * SPIRAL_SPACING;
        m->count = SPIRAL_RADIUS;//(g->count < PHASE1) ? 12 : 12;
        dir = goRound(g->srchx2, g->srchy2, 0, 0);
        g->srchx2 += RX[dir]; g->srchy2 += RY[dir];

        if (g->srchx2 > abs(g->srchx)) {
            g->srchx2 = g->srchy2 = 1;

            m->srchx = g->srchx * SPIRAL_SPACING;
            m->srchy = g->srchy * SPIRAL_SPACING;
            m->count = SPIRAL_RADIUS;//(g->count < PHASE1) ? 12 : 12;
            dir = goRound(g->srchx, g->srchy, 0, 0);
            g->srchx += RX[dir]; g->srchy += RY[dir];
//??:
            if (g->srchx < -10) {
                g->srchx = g->srchy = 1;
            }

        }
    } else {
        m->srchx = g->srchx * SPIRAL_SPACING;
        m->srchy = g->srchy * SPIRAL_SPACING;

        m->count = SPIRAL_RADIUS;//(g->count < PHASE1) ? 12 : 12;
        dir = goRound(g->srchx, g->srchy, 0, 0);
        g->srchx += RX[dir]; g->srchy += RY[dir];
/*      if (g->srchx < -20) {
            g->srchx = g->srchy = 1;
        }
*/
    }
    if (g->basespos < 0) debug("DEF %d\n", dir);
}

Retning GenralMain(struct SquareData *sqr, Genral_Mind* m) {
    Retning dir = STOP;
    FORHVERGAAHVIS(rr->NumAnts>0 && rr->Team != 0 , /**/ ); /* Kill! Kill! */
    dir = goTo(m->px,m->py, 0,0);
done:
    return dir;
}

Retning SkafferMain(struct SquareData *sqr, Skaffer_Mind* m) {
    Retning dir = STOP;
    int i;

    FORHVERGAAHVIS(rr->NumAnts>0 && rr->Team != 0 , 
                   MakeLurker((Lurker_Mind*)m);
                   /*
                   m->enemyx = m->px+RX[r];
                   m->enemyy = m->py+RY[r];
                   m->enemymet = 1;
                   */
//                 m->count = 1000;
                   ); /* Kill! Kill! */

//    if (abs(m->px)+abs(m->py) < 5) return goToRand(

    if (sqr[HERE].NumFood < sqr[HERE].NumAnts) {
//      if (abs(m->px)+abs(m->py) > 10)
        FORHVERGAAHVIS(rr->NumFood > rr->NumAnts && rr->NumAnts==0,
/* Add food spot */
                       if (rr->NumFood>1) {
                           for (i=0; i<SKAF_FOOD_MEM; i++) 
                               if (m->foodcnt[i]==0) {
                                   m->foodx[i] = m->px+RX[r];
                                   m->foody[i] = m->py+RY[r];
                                   m->foodcnt[i] = sqr[r].NumFood;
                                   break;
                               }
                       }
            ); /* Foood! */
        if (m->reached==0) {
            dir = goToRand(m->px, m->py, m->srchx, m->srchy, (Basic_Mind*)m);
//          debug("Her->%d\n", dir);
/*          if (m->id&1) 
                dir = goToRand(m->px, m->py, m->srchx, m->srchy);
            else
                dir = goToRand2(m->px, m->py, m->srchx, m->srchy);
*/
        }
        if (dir == STOP) {
            m->reached = 1;
            if (m->px > m->srchx + m->count) {
                m->srchx = m->srchy = 0;
                for (i=0; i<SKAF_FOOD_MEM; i++)
                    if (m->foodx[i] == m->srchx && m->foody[i] == m->srchy)
                        m->foodcnt[i] = 0;
                m->reached = 0;//-1;
            }
            dir = goRound2(m->px, m->py, m->srchx, m->srchy);
        }
    } else { // Food here. Bring home.
        dir = 8 + goToRand(m->px, m->py, 0,0, (Basic_Mind*)m);
        //TEST:
        if (dir == STOP) dir = goToRand(m->px, m->py, m->srchx, m->srchy, (Basic_Mind*)m);
    }

    if (dir == STOP) debug("STOP @ %d,%d %d,%d  %d \n",
                            m->px, m->py, m->srchx, m->srchy,
                            m->reached
                            );
done:
    return dir;
}

Retning LurkerMain(struct SquareData *sqr, Skaffer_Mind* m) {
    Retning dir = STOP;
    FORHVERGAAHVIS(rr->NumAnts>0 && rr->Team != 0 , 
                   m->count+=10 ); /* Kill! Kill! */
    if (m->count-- == 0) 
        m->state = SKAFFER; // TODO: Not fool-proof.
    if ((m->count & 31) == 0)
        dir = (m->px < 0)? WEST : EAST;

done:
    return dir;
}

Retning SoldierMain(struct SquareData *sqr, Soldier_Mind* m) {
    Retning dir = STOP;
    int gen;
    FORHVERGAAHVIS(rr->NumAnts>0 && rr->Team != 0 , 
                   m->srchx = m->px; m->srchy = m->py;
                   m->count+=10;
                   debug("Kill/Soldier");
                   ); /* Kill! Kill! */
    if (m->px == 0 && m->py == 0) {
        SNAKMEDGENRAL(Soldier);
    }
    if (! m->reached) {
        // TEST(?):
        dir = goToRand(m->px, m->py, m->srchx, m->srchy, (Basic_Mind*)m);
    }
    if (dir == STOP) m->reached = 1;
    if (m->reached) {
        if (m->count-- == 0) 
            MakeSkaffer((Skaffer_Mind*) m); //?
    }
done:
    return dir;
}

void SoldierCommand(Genral_Mind* g, Soldier_Mind* m) {
    int i;
    int gogo = 1;
    for (i=0; i<ENEMY_MEM; i++) if (g->enemycnt[i]) {
        if (g->enemyx[i] == m->srchx && g->enemyy[i] == m->srchy) {
            gogo = 0;
        }
    }
    if (gogo) {
        m->reached=0;
    }
}

/*Retning*/ int PanoleonMain(struct SquareData *sqr, Mind* mind) {
    Basic_Mind* m = &mind->basemind;
    Retning dir = STOP;
    int gen;

/*
  if ((m->id & 0x8F00)==0) {
  dir = goRound10(m->px, m->py, 0,0, 20+((m->id)/100000000));
  goto done;
  }
*/
    switch(m->state) {
    case INIT:
        m->random = m->id;
        SNAKMEDGENRAL(Init);
        if (m->state == INIT) {
            MakeGeneral((Genral_Mind*) m);
            debug("Made general");
        }
        break;
    case GENRAL:
        dir = GenralMain(sqr, (Genral_Mind*) m);
//      if (sqr[HERE].NumFood) debug("Mad her: %d\n", sqr[HERE].NumFood);
        break;
    case SKAFFER:
        SNAKMEDGENRAL(Skaffer);
        if (m->px == 0 && m->py == 0 && ((Skaffer_Mind*)m)->srchx == 0 &&
((Skaffer_Mind*)m)->srchy == 0) {
            debug("STOP...");
        }

        dir = SkafferMain(sqr, (Skaffer_Mind*) m);
        if (m->px == 0 && m->py == 0) {
            if (!gen) {
                MakeGeneral((Genral_Mind*) m);
                debug("NEW GENR");
                ((Genral_Mind*) m)->basespos = -1; //DEBUG
            }
            if (!sqr[HERE].Base) 
                dir = STOP; //Bliv & byg base
        }
        break;
    case LURKER:
        dir = LurkerMain(sqr, (Skaffer_Mind*) m);
        break;
    case SOLDIER:
        dir = SoldierMain(sqr, (Soldier_Mind*) m);
        break;
    }
//done:
    m->px += RX[dir];  m->py += RY[dir];
//    if (sqr[HERE].NumFood) dir+=8; /*Carry - change if NEWBASE implemented*/
    if (sqr[HERE].NumAnts >= NewBaseAnts && sqr[HERE].NumFood >= NewBaseFood) {
        dir=16;
        debug("BASE");  
    }
    return dir;
}

//#undef FORHVERGAAHVIS
//#undef GENRAL
//#undef SOLDIER

DefineAnt(Panoleon, "Panoleon", PanoleonMain, Mind)
