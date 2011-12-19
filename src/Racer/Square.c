/* Myre basseret på Turbo myren.    *\
|* Square myren er lidt langsommere *|
|* end Turbo myren, tilgengæld er   *|
\* den meget mere intelligent.      */

#include "Myre.h"

static int square(struct SquareData*sq,u_char*mem)
{
  int i;
  for (i=1;i<5;++i) {
    if (sq[i].Team) return i;
    if ((sq[i].Base)&&(i!=((*mem>>6)^2)+1)) {
      if (!sq[0].NumFood) (*mem)+=3;
      if (sq[i].NumAnts > MaxSquareAnts) return (i&3)+1;
      if (((1+*mem)>>6)+1!=i) return i+8;
    }
  }
  if (sq[0].NumFood)
    return (++*mem>>6)+9;
  for (i=1;i<5;++i)
    if ((sq[i].NumFood)&&(!sq[i].NumAnts)&&
	(!sq[(((u_char)(1+*mem))>>6)+1].NumFood))
      return i;
  return (++*mem>>6)+1;
}

DefineAnt(Square,"Square#00FFFF",square,u_char);
