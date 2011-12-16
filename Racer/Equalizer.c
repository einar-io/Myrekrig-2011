/*
-----------------------------------------------------------------------
"Equalizer" version 1.1 - implementeret af Jørn Holm (gekko@cs.auc.dk).
-----------------------------------------------------------------------

-----------------------------------------------------------------------
Ændringer i version 1.1:
-Performance let forbedret.
-Bedre udnyttelse af hukommelsen.
-Bedre distribuering af mad information.
-Øget robusthed, vha. bedre forsvarssystem (specielt på basen).
---------------------------------------------------------------------
*/
#include "Myre.h"
int equalizer(struct SquareData *f, u_char *x){
	int y;
	y = equalizerWarCheck(f);
	if (y){
		if (x[0] == 255 || x[0] == 252)
			x[0] = 10+y;
		return y;
	}
	if (x[0] > 10 && x[0] < 15){
		int temp = x[0]-10;
		x[0] = 255;
		if (temp == 1)
			return 3;
		if (temp == 2)
			return 4;
		if (temp == 3)
			return 1;
		if (temp == 4)
			return 2;
	}
	if (f->Base && x[0] != 252){
		x[0] = 1;
		return 1;
	}
	
	if (x[0] < 2 && x[0]){
		x[0]--;
		return 0;
	}
	if (f->NumAnts > 2 && x[0] == 0)
		return 1;
	if (f->NumAnts == 2 && x[0] == 0 && x[1] == 0){
		x[0] = 246;
		x[1] = 255;
		return 4;
	}
	if (x[0] == 70){
		y = equalizerFoodCheck(f);
		if (y == 0 && f->NumAnts == 1)
			return 12;
		if (y < 5 && y && f[y].NumAnts == 0)
			return y;
		return 2;
	}
	if (x[0] < 247 && x[0] > 79){
		y = equalizerFoodCheck(f);
		if (y == 0 && f->NumAnts == 1){
			x[0] = 246;
			return 10;
		}
		if (y == 0 && f->NumAnts == 3 && f[4].NumAnts == 0){
			if (x[1] != 255)
				x[1] = 246;
			else
				x[2] = 246;
		}
		
		if (y < 5 && y && f[y].NumAnts == 0){
			x[0] = 246;
			return y;
		}
		if (x[0] == 80){
			x[0] = 70;
			return 1;
		}
		x[0]--;
		return 4;
	}
	if (x[0] == 255){
		if (f->NumFood){
			x[0] = 252;
			return 11;
		}
		return 0;
	}
	if (x[0] == 252){
			x[0] = 255;
			return 1;
	}
	return 0;
}
inline static int equalizerWarCheck(struct SquareData *z){
    	if(z[1].Team)
    		return 1;
    	if(z[2].Team)
    		return 2;
    	if(z[3].Team)
    		return 3;
    	if(z[4].Team)
    		return 4;
    	return 0;
}
inline static int equalizerFoodCheck(struct SquareData *z){
	if (z[0].NumFood)
    		return 0;
    	if (z[1].NumFood)
    		return 1;
    	if (z[3].NumFood)
    		return 3;
    	return 5;
}
DefineAnt(Equalizer,"Equalizer",equalizer,u_char);