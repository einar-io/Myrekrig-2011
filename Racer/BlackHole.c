#include "Myre.h"
#define I if
#define R return
#define B Base
#define F NumFood
#define A NumAnts
#define N 25
typedef struct{} hul;int bh(struct SquareData*x,hul*nullpointer){short i;I (!x[3].B)for(i=4;i;i--)I(x[i].Team)R i;I (x[1].B&&x[1].A<N) R 9;I (x[2].B)R 10;I (x[3].B){I (x[3].A<N||x->A>1)R 11;else R 0;}I (x[4].B)R 12;I (x->F>=x->A)R 9;I (!x->B) {for (i=4;i;i--)I (x[i].F>x[i].A) R i;R 3;}I (!x[1].A&&x->A>N)R 3;R  0;}DefineAnt(BlackHole,"BlackHole#000000",bh,hul);
