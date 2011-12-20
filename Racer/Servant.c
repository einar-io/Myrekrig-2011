/*
2011-12-19
Den kære leder er død.

*/

#include "Myre.h"



struct brain {
   u_long random;
   int i;
   short x,y;
   short foodx[10],foody[10];
   u_long timeout;
} hukk;



/*
struct SquareData {
   u_char NumAnts;
   u_char Base;
   u_char Team;
   u_char NumFood;
};
*/


int Servant(struct SquareData *felter, struct brain *mem) {
//	mem->turn++;
//	return brain.turn % 5;//(brain.random^brain.turn % 4) + 1;
       //*mem = &hukk;
int i;
return (hukk.random)^(hukk.i)++ % 4 ;
	
}
	
DefineAnt(Servant, "Servant", Servant, struct brain);
