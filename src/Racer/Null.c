/* Null. Eriks første nul-byte-myre.
 * Kreéret 12/8-02.
 * (C) Copyright 2002 Erik Søe Sørensen.
 */

#include "Myre.h"

#define main NULL_MYRE_##main
#define wrapper NULL_MYRE_##wrapper

typedef enum {STOP=0, HERE=0, EAST, SOUTH, WEST, NORTH,
              CARRY=8, BUILDBASE=16} Retning;

typedef struct Mind { } Mind;


__inline__ int main(struct SquareData *sqr, Mind* m) {
    int frem,tilbage,hojre,venstre;
	int i;
    frem = EAST; tilbage = WEST;
    hojre = NORTH; venstre = SOUTH;

	if (sqr->NumFood) {
		//if (sqr[frem].Base) return frem|CARRY;
		for (i=1; i<=4; i++) if (sqr[i].Base) return (i|CARRY);
		if (sqr[venstre].NumAnts) return venstre|CARRY;
		if (sqr[tilbage].NumAnts + sqr[tilbage].NumFood/* + sqr[tilbage].Base*/)
			return tilbage|CARRY;
		return venstre | CARRY;
	}

	if (sqr->NumAnts+3*sqr->NumFood >= 7) return hojre;
	if (sqr->Base || sqr->NumAnts > 1)
		if (sqr[hojre].NumFood) return hojre; else return frem;
	//if (sqr->NumAnts >= 2) return hojre;
	if (sqr[frem].NumAnts == 0 && sqr[frem].NumFood == 0) return hojre;

	if (sqr[tilbage].NumAnts==1 && sqr[frem].NumAnts==1 &&
		sqr[venstre].NumAnts>1)
		return frem;

    return STOP;
}

DefineAnt(Null, "Null", main, struct Mind)
