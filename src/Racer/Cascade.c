/* Cascade: Eriks anden énbyte-myre.
 * Kreéret 13/7-02.
 * (C) Copyright 2002 Erik Søe Sørensen.
 */
/* - Beskyt basen
 * - Undgå leaching.
 */

#include "Myre.h"

typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH,
              CARRY=8, BUILDBASE=16} Retning;

//#define FLAG1 1
#define FLAG2 1
#define FLAG3 2
#define MULT1 4

typedef struct Mind {
    unsigned char data;
} Mind;

static __inline__ int main(struct SquareData *sqr, Mind* m) {
    int frem,tilbage,hojre,venstre;
    frem = EAST; tilbage = WEST;
    hojre = NORTH; venstre = SOUTH;

    if (sqr->Base) {
		if (sqr->NumAnts+sqr[frem].NumAnts+sqr[frem].NumFood > 5 &&
			(m->data&0xE8) == 0xE8)
		{
			int dir = (m->data&3)+1;
			if (dir != frem && sqr[dir].NumAnts < 7) {
				m->data = 0xF7;
				return dir;
			}
		}
		m->data = 0;
    }

    if (sqr[EAST].Team  + sqr[WEST].Team +
		sqr[NORTH].Team + sqr[SOUTH].Team)
    {
		m->data = 0xF7&~FLAG3;
		if (sqr[EAST].Team)  return EAST;
		if (sqr[WEST].Team)  return WEST;
		if (sqr[NORTH].Team) return NORTH;
		if (sqr[SOUTH].Team) return SOUTH;
    }
    if ((m->data&~FLAG3) == (0xF7&~FLAG3)) return STOP; // Guard

    if (sqr->NumFood > 0) {
		if (m->data&FLAG3 && sqr->NumAnts>1) {
			m[1].data |= FLAG3;
		}

		if (sqr[hojre].Base   + sqr[venstre].Base +
			sqr[tilbage].Base + sqr[frem].Base) {
			if (sqr[hojre].Base) return hojre | CARRY;
			if (sqr[venstre].Base) return venstre | CARRY;
			if (sqr[frem].Base) return frem | CARRY;
			if (sqr[tilbage].Base) return tilbage | CARRY;
		}

		if (sqr[tilbage].NumAnts > 0) return tilbage | CARRY;
		if (sqr[hojre].NumAnts > 0) return hojre | CARRY;
		if (sqr[tilbage].NumFood > 0) return tilbage | CARRY;
		return hojre | CARRY;
    }

    if (sqr->NumAnts > 1 || sqr->Base) {
		if (sqr[venstre].NumFood || (m->data&FLAG3)) { // Signal...
			if (sqr->NumAnts>=3) {
				m[1].data |= FLAG3;
			}
			m->data |= FLAG2;
			return venstre;
		}

		return frem;
    }

    if (sqr[venstre].NumFood) { // Signal... (but none available)
		m->data |= FLAG3;
    }

    if (sqr[frem].NumAnts == 0 && sqr[frem].NumFood == 0) {
		if (sqr[tilbage].NumAnts > 0) {
			if (m->data >= 3*MULT1) {
				m->data |= FLAG2;
				return venstre;
			} else {
				m->data += MULT1;
			}
		} else if (m->data&FLAG2) {
			if (m->data > MULT1) {
				m->data &= (MULT1-1); m->data += MULT1;
			}
			if (sqr[tilbage].Base || sqr[tilbage].NumAnts) {
				m->data &=~ FLAG2; return STOP;
			}
			return venstre;
		}
    }

    return STOP;
}

int CascadeWrapper(struct SquareData *sqr, Mind* m) {
    int dir = main(sqr,m);
    if (dir!=STOP) m->data &=~ FLAG3;
    return dir;
}

DefineAnt(Cascade, "Cascade#8080F0", CascadeWrapper, struct Mind)
