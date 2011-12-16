/* Eriks Hex-myre.
   12/1 - ?
 */

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// INTERFACE
#define debug(a,b...) printf(a "\n",b);

#include "Myre.h"

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\// KONSTANTER og HEX-DEFs
typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH, 
              CARRY=8, BUILDBASE=16} Retning;


typedef enum {
    flKNOWSFOOD  = 128,
    flPROTECT    =  64,
    flGOROUNDDIR =  32,
    flONLYNEAR   =  16, //Do not attempt to build new bases (or cross borders).
    flSOLDIER    =   8,
    flGOCLOSE    =   4, //Search-pattern: Go close to base
    flRESETKEEP = flGOROUNDDIR | flGOCLOSE,
    flBASEKEEP = (flGOROUNDDIR | flGOCLOSE | flKNOWSFOOD | flPROTECT)
} Flags;

#define DIRS HEX2_DIRS
#define RX HEX2_RX
#define RY HEX2_RY
#define TURN HEX2_TURN
#define delay HEX2_delay
#define delay2 HEX2_delay2
#define goHome HEX2_goHome
#define goDir HEX2_goDir
#define goThere HEX2_goThere
#define bitswap6 HEX2_bitswap6

const int RX[] = {0, 1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0,
                     1,0,-1,0, 0,0,0,0, 1,0,-1,0, 0,0,0,0};
const int RY[] = {0, 0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0,
                     0,-1,0,1, 0,0,0,0, 0,-1,0,1, 0,0,0,0};
const int TURN[] = {0, 2,3,4,1, 6,7,8,5, 10,11,12,9, 14,15,16,13};

typedef struct { signed char x, y; } DirDescr;
const DirDescr DIRS[256] = {
        {-64,0}, 
        {-63,1}, {-63,3}, {-62,4}, {-61,5}, {-61,7}, {-60,8}, 
        {-59,9}, {-59,11}, {-58,12}, {-57,13}, {-57,15}, {-56,16}, 
        {-55,17}, {-55,19}, {-54,20}, {-53,21}, {-53,23}, {-52,24}, 
        {-51,25}, {-51,27}, {-50,28}, {-49,29}, {-49,31}, {-48,32}, 
        {-47,33}, {-47,35}, {-46,36}, {-45,37}, {-45,39}, {-44,40}, 
        {-43,41}, {-43,43}, {-42,44}, {-41,45}, {-41,47}, {-40,48}, 
        {-39,49}, {-39,51}, {-38,52}, {-37,53}, {-37,55}, {-36,56}, 
        {-35,57}, {-35,59}, {-34,60}, {-33,61}, {-33,63}, {-32,64}, 
        {-30,64}, {-28,64}, {-26,64}, {-24,64}, {-22,64}, {-20,64}, 
        {-18,64}, {-16,64}, {-14,64}, {-12,64}, {-10,64}, {-8,64}, 
        {-6,64}, {-4,64}, {-2,64}, {0,64}, 
        {2,64}, {4,64}, 
        {6,64}, {8,64}, {10,64}, {12,64}, {14,64}, {16,64}, 
        {18,64}, {20,64}, {22,64}, {24,64}, {26,64}, {28,64}, 
        {30,64}, {32,64}, {33,63}, {33,61}, {34,60}, {35,59}, 
        {35,57}, {36,56}, {37,55}, {37,53}, {38,52}, {39,51}, 
        {39,49}, {40,48}, {41,47}, {41,45}, {42,44}, {43,43}, 
        {43,41}, {44,40}, {45,39}, {45,37}, {46,36}, {47,35}, 
        {47,33}, {48,32}, {49,31}, {49,29}, {50,28}, {51,27}, 
        {51,25}, {52,24}, {53,23}, {53,21}, {54,20}, {55,19}, 
        {55,17}, {56,16}, {57,15}, {57,13}, {58,12}, {59,11}, 
        {59,9}, {60,8}, {61,7}, {61,5}, {62,4}, {63,3}, 
        {63,1}, {64,0}, 
        {63,-1}, {63,-3}, {62,-4}, {61,-5}, 
        {61,-7}, {60,-8}, {59,-9}, {59,-11}, {58,-12}, {57,-13}, 
        {57,-15}, {56,-16}, {55,-17}, {55,-19}, {54,-20}, {53,-21}, 
        {53,-23}, {52,-24}, {51,-25}, {51,-27}, {50,-28}, {49,-29}, 
        {49,-31}, {48,-32}, {47,-33}, {47,-35}, {46,-36}, {45,-37}, 
        {45,-39}, {44,-40}, {43,-41}, {43,-43}, {42,-44}, {41,-45}, 
        {41,-47}, {40,-48}, {39,-49}, {39,-51}, {38,-52}, {37,-53}, 
        {37,-55}, {36,-56}, {35,-57}, {35,-59}, {34,-60}, {33,-61}, 
        {33,-63}, {32,-64}, {30,-64}, {28,-64}, {26,-64}, {24,-64}, 
        {22,-64}, {20,-64}, {18,-64}, {16,-64}, {14,-64}, {12,-64}, 
        {10,-64}, {8,-64}, {6,-64}, {4,-64}, {2,-64}, {0,-64}, 
        {-2,-64}, {-4,-64}, {-6,-64}, {-8,-64}, {-10,-64}, {-12,-64}, 
        {-14,-64}, {-16,-64}, {-18,-64}, {-20,-64}, {-22,-64}, {-24,-64}, 
        {-26,-64}, {-28,-64}, {-30,-64}, {-32,-64}, {-33,-63}, {-33,-61}, 
        {-34,-60}, {-35,-59}, {-35,-57}, {-36,-56}, {-37,-55}, {-37,-53}, 
        {-38,-52}, {-39,-51}, {-39,-49}, {-40,-48}, {-41,-47}, {-41,-45}, 
        {-42,-44}, {-43,-43}, {-43,-41}, {-44,-40}, {-45,-39}, {-45,-37}, 
        {-46,-36}, {-47,-35}, {-47,-33}, {-48,-32}, {-49,-31}, {-49,-29}, 
        {-50,-28}, {-51,-27}, {-51,-25}, {-52,-24}, {-53,-23}, {-53,-21}, 
        {-54,-20}, {-55,-19}, {-55,-17}, {-56,-16}, {-57,-15}, {-57,-13}, 
        {-58,-12}, {-59,-11}, {-59,-9}, {-60,-8}, {-61,-7}, {-61,-5}, 
        {-62,-4}, {-63,-3}, {-63,-1}
};

void delay() {
    int i,j,k;
    for (i=0; i<50; i++)
        for (j=0; j<1000; j++)
            k = i*j;
}

void delay2() {
    int i,j,k;
    for (i=0; i<500; i++)
        for (j=0; j<1000; j++)
            k = i*j;
}

typedef struct Mind {
    signed char px, py;
    unsigned char flags;
    unsigned char id;
} Mind;

Retning goHome(Mind* m) {
    if (!m->px) {
        if (!m->py) return STOP;
        return (m->py>0)? SOUTH:NORTH;
    }
    if (!m->py) return (m->px>0)? WEST:EAST;

    if ((m->px+m->py)&1)
        return (m->py>0)? SOUTH:NORTH;
    else
        return (m->px>0)? WEST:EAST;
}

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
Retning goThere(Mind* m, int dstx, int dsty) {
    int dx = abs(m->px - dstx), dy = abs(m->py - dsty);
    int a = dx+dy;
    int aa = bitswap6[a&63];
    if (m->px == dstx) {
        if (m->py == dsty) return STOP;
        return (m->py < dsty)? NORTH : SOUTH;
    }
    if (m->py == dsty) return (m->px < dstx)? EAST : WEST;

    if (aa*(dx+dy)<dx*64)
        return (m->px < dstx)? EAST : WEST;
    else
        return (m->py < dsty)? NORTH : SOUTH;
}

Retning goDir(Mind* m) {
    int dx,dy;
    if (m->flags&flONLYNEAR) {
        dx = DIRS[m->id].x, dy = DIRS[m->id].y;
    } else {
        dx = DIRS[m->id].x*3/2, dy = DIRS[m->id].y*3/2; //TODO: Ny tabel
        //dx = DIRS[m->id].x*5/4, dy = DIRS[m->id].y*5/4; //TODO: Ny tabel
    }

    if (m->px == dx) {
        if (m->py == dy) return STOP;
        return (m->py > dy)? SOUTH:NORTH;
    }
    if (m->py == dy) {
        return (m->px > dx)? WEST:EAST;
    }

    return goThere(m, dx, dy);
}


Retning goDir_old(Mind* m) {
    int dx,dy;
    int wx, wy;
    if (m->flags&flONLYNEAR) {
        dx = DIRS[m->id].x, dy = DIRS[m->id].y;
    } else {
        dx = DIRS[m->id].x*3/2, dy = DIRS[m->id].y*3/2; //TODO: Ny tabel
    }

    if (m->px == dx) {
        if (m->py == dy) return STOP;
        return (m->py > dy)? SOUTH:NORTH;
    }
    if (m->py == dy) {
        return (m->px > dx)? WEST:EAST;
    }

    wx = ((dy)*(m->px)); wy = ((dx)*(m->py));

    if (abs(wy-wx) >= abs(dx)) {
        return (m->py > dy)? SOUTH:NORTH;
    } else {
        return (m->px > dx)? WEST:EAST;
    }
}


int calcDir(int x, int y) { 
    /*    48 b 96
     *     +---+
     *  a /  |  \ c
     * 0 +---+---+ 128
     *  f \  |  / d
     *     +---+
     *  208  e  176
     */
    if (y >= 0) {
        if (y < -2*x) {
            return -96*y/(2*x-y);  //a
        } else if (y < 2*x) {
            return 128 - 96*y/(2*x+y); //c
        } else {
            return 64 + 32*x/y; //b
        }
    } else { // y<0
        if (y > -2*x) {
            return 128 - 96*y/(2*x-y); //d
        } else if (y > 2*x) {
            return 256 - 96*y/(2*x+y); //f
        } else {
            return 192 + 32*x/y; //e
        }
    }
}

/*
int calcDir(int x, int y) {
    int dir = calcDir__(x,y);
    printf("calcDir(%d,%d) = %d (%d,%d)\n",
           x,y, dir, DIRS[dir].x, DIRS[dir].y);
    return dir;
}
*/

Retning HexThink(struct SquareData *sqr, Mind* m) {
    //delay2();

    if (sqr->Base) {
        if (m->px|m->py) {
            int i;

            m->px = m->py = 0;
            m->flags &= flRESETKEEP;

            //Hvor er der mad?
            for (i=1; i<sqr->NumAnts; i++) if (m[i].flags&flKNOWSFOOD) {
                m->id = m[i].id;
                break;
            }
        } else {
            m->flags &= flBASEKEEP;
        }
    }

    if (m->flags & (flSOLDIER|flPROTECT)) {
        if (sqr->NumAnts >= 5) m->flags &=~ (flSOLDIER|flPROTECT);
        if (m->flags & flSOLDIER) {
            if (sqr->NumAnts>1) {
                if (--m->id == 0) m->flags &=~ flSOLDIER; //Off duty
            }
            return STOP;
        } else
            return goHome(m);
    }

    if (sqr->NumFood) {
        if (sqr->NumFood > sqr->NumAnts)
            if (!(m->flags&flKNOWSFOOD)) {
                m->flags |= flKNOWSFOOD;

                // Basebygning:
                if (abs(m->py) > 64) {
                    if (m->py>0) m->py-=128; else m->py+=128;
                    m->id += (m->flags&flGOROUNDDIR)? 64 : -64;
                } else if (abs(m->px)+abs(m->py)/2 > 64) {
                    if (m->px>0) m->px-=96; else m->px+=96;
                    if (m->py>0) m->py-=64; else m->py+=64;
                    m->id += (m->flags&flGOROUNDDIR)? 64 : -64;
                }

                // Set dir:
                if (m->px|m->py) m->id = calcDir(m->px, m->py);
            }

        {
            Retning dir = goHome(m); // goThere(m,0,0);
            if (dir) return dir | CARRY;
            // else: Basebygning...:
            m->flags |= flONLYNEAR;
            if (sqr->NumFood < NewBaseFood) {
                if (sqr->NumAnts <= sqr->NumFood >> 2)
                    return STOP;
                else
                    return goDir(m);
            } else
                return STOP;
        }               
    } else {
        int i;
        if (m->flags&flKNOWSFOOD) { //Fortæl videre
            for (i=1; i<sqr->NumAnts; i++) if (!(m[i].flags&flKNOWSFOOD)) {
                m[i].id = m->id; m[i].flags &=~ flSOLDIER;
            }       
        }

        for (i=1; i<=4; i++) if (sqr[i].NumFood > sqr[i].NumAnts) {
            // Ikke på basepos?
            if (m->px+RX[i] != 0 || m->py+RY[i] != 0) {
                //printf("Found food @ %d,%d\n", m->px+RX[i], m->py+RY[i]);
                return i;
            }
        }

        {
            Retning dir = goDir(m);
            if (dir) return dir;
            //Stopped - change destination:
            // Good deltas: 67 (but near-base area is not searched) ;
            m->flags &=~ flKNOWSFOOD;
            {
                int delta = (m->flags&flGOCLOSE? 123-23 : 67-23) + 23*sqr->NumAnts;
                if (m->flags&flGOROUNDDIR)
                    m->id += delta;
                else
                    m->id -= delta;
            }
            return goHome(m);
        }
    }
}

int /*Retning*/ HexMain(struct SquareData *sqr, Mind* m) {
    Retning dir = HexThink(sqr, m);
    int i;

    if (sqr->NumAnts > NewBaseAnts &&
        sqr->NumFood >= NewBaseFood && !sqr->Base)
    {
        dir = BUILDBASE;
        /*
        printf("Building base @ %d,%d (%d %d %d)\n",
               m->px,m->py,  sqr->NumFood, sqr->NumAnts, sqr->Base);
        */
        m->flags |= flPROTECT;
    }

    //Re-coordinate:
    {
        int i;
        int a = abs(m->px)+abs(m->py);
        for (i=1; i<sqr->NumAnts; i++) {
            if (a < abs(m[i].px)+abs(m[i].py)) {
                m[i].px = m->px; m[i].py = m->py;
            }
        }
    }

    /*
    if (sqr->NumAnts >= MaxSquareAnts) printf("Ant congenstion\n");
    if (sqr->NumFood >= MaxSquareFood) printf("Food congenstion\n");
    */

    if (sqr[(dir&7)].NumAnts >= MaxSquareAnts) {
        dir = 5-(dir&7); //back up
    }

    // Enemies?
    for (i=1; i<=4; i++) {
        if (sqr[i].Team != 0) {
            dir = i;
            //printf("Enemy spotted\n");
            m->flags |= flSOLDIER;
            m->id = 8; //Friend count
            break;
        }
    }

    m->px += RX[dir];
    m->py += RY[dir];
    return dir;
}

DefineAnt(Hex, "Hex", HexMain, struct Mind)
