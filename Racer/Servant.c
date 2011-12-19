/*
2011-12-19
Den kære leder er død.

*/

#include "Myre.h"



struct brain {
   u_long random;
   short x,y;
   short foodx[10],foody[10];
   u_long timeout;
};
/*
struct SquareData {
   u_char NumAnts;
   u_char Base;
   u_char Team;
   u_char NumFood;
};
*/


int Servant(struct SquareData *felter, struct brain *mem) {
	return 0 ;
}
	
DefineAnt(Servant, "Servant", Servant, struct brain);
