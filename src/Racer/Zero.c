/* ZeroAnt, final version (0.0.0)
   Copyright: Jacob Johannsen aka CNN (cnn@daimi.au.dk)
   This ant is developed for use in Aske Simon Christensen's "MyreKrig" (http://www.myrekrig.dk).
   This program may be distributed freely for non-commercial purposes. For commercial purposes, forget it; the price is too high. 
   Although anyone who trusts programs with version numbers this low is probably too stupid to even read this, and therefore should be allowed to suffer the consequences, it should be noted that by no means will the author be held responsible for any injury, be it physical, mental or any other type of injury, any monetary or otherwise financial loss, including, but not limited to, loss of goodwill, any catastrophes, be they earthquakes, floods, mud slides, avalanches, tornados or hurricanes, power outages, loss of internet connection, burglary, thunderstorms, fires, vomitting, bombardements, famine, plague, religious fanatism, falling meteors, falling buildings or parts thereof, dinosaur- or alien attacks, terrorist activities, ANY activity involving ANYONE buying someone one or more beers (or sodas), cosmic radiation, or the heat death of the universe. You get the basic idea.*/

#include "Myre.h"

struct Brain {
    u_long random;
    short xpos, ypos, xfood, yfood;
};

int antfunc(struct SquareData *fields, struct Brain *mem){
    int i, move = 0, max = 0;

    if (fields[0].Base) {
	mem->xpos = mem->ypos = 0;
    }

    for (i = 1; i <= 4; i++){
	if (fields[i].Team){
	    if (fields[i].Base) {
		max = MaxSquareAnts + 1;
		move = i;
	    }
	    else if (fields[i].NumAnts > max){
		max = fields[i].NumAnts;
		move = i;
	    }
	}
    }

    if (fields[0].NumFood && !move){

	if (fields[0].NumFood > fields[0].NumAnts){
	    mem->xfood = mem->xpos;
	    mem->yfood = mem->ypos;
	}	
	if (fields[0].NumFood >= NewBaseFood && fields[0].NumAnts >= NewBaseAnts){
	    move = 16;
	}
	else if (mem->xpos){
	    move = ((mem->xpos > 0) ? 3 : 1);
	}
	else {
	    move = ((mem[0].ypos > 0) ? 2 : 4);
	}
	
    }
    else {
	for (i = 1; !move && i <= 4; i++){
	    if (fields[i].NumFood - fields[i].NumAnts > 0){
		move = i;
		}
	    }
	if (!move && (mem->xfood || mem->yfood)) {
	    if (mem->xfood - mem->xpos){
		move = (( mem->xpos > mem->xfood) ? 3 : 1);
	    }
	    else if (mem->yfood - mem->ypos) {
		move = ((mem->ypos > mem->yfood) ? 2 : 4);
	    }
	    else {
		mem->xfood = mem->yfood = 0;
	    }
	}
	else {
	    for (i = 1; !move && i < fields[0].NumAnts; i++){
		if (mem[i].xfood || mem[i].yfood){
		    mem[0].xfood = mem[i].xfood;
		    mem[0].yfood = mem[i].yfood;
		    if (mem->xfood != mem->xpos){
			move = (( mem->xpos > mem->xfood) ? 3 : 1);
		    }
		    else {
			move = ((mem->ypos > mem->yfood) ? 2 : 4);
		    }
		}
	    }
	}
	
	if (!move) {
	    int last = mem->random%5;
	    move = ((mem->random = (mem->random*223 + 13))%5) ;
	    while (!(move && (move == last || (move%2 != last%2)))){
		move = ((mem->random = (mem->random*223 + 13))%5) ;
	    }
	}
    }
     
    if (move < 16){
	move = move+8;
    }
    else {
	mem->xpos = mem->ypos = 0;
    }

    switch(move%8){
    case 0: break;
    case 1: mem->xpos++;
	break;
    case 2: mem->ypos--;
	break;
    case 3: mem->xpos--;
	break;
    case 4: mem->ypos++;
	break;
    }
    return move;    
}

DefineAnt(Zero, "ZeroAnt", antfunc, struct Brain)


