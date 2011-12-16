#include "Myre.h"
struct rb {
};
int RB(struct SquareData *f, struct rb *m) {
	int i = 0;
	int max = 0;
	int best = 0;
	for(i = 1; i < 5; i++){
		if(f[i].Base == 1)
			return i;
	}
	for(i = 1; i < 5; i++){
		if(f[i].Team > 0){
			if (f[i].NumAnts > max){
				max = f[i].NumAnts;
				best = i;
			}
		}
	}
	return best;
}

DefineAnt(Rambo, "Rambo#FF0000", RB, struct rb); 
	
