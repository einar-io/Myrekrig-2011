#include "Myre.h"

#define signpost walker_.signpost_
#define dir walker_.dir_
#define pos walker_.pos_
#define found walker_.found_

#define orient signpost_.orient_
#define foodleft signpost_.foodleft_
#define foodright signpost_.foodright_

#define FOUND_FOOD_AMOUNT 3
#define POSITION_WAR 3
#define DEFENCE_SHIFT 3

#define min(a,b) ((a)<(b)?(a):(b))

union FishBrain {
    struct {
		u_char signpost_ : 1;
		u_char dir_ : 2;
		u_char pos_ : 2;
		u_char found_ : 1;
	} walker_;
	struct {
		u_char signpost_ : 1;
		u_char orient_ : 1;
		u_char foodleft_ : 2;
		u_char foodright_ : 2;
	} signpost_;
	u_char brain;
};

int Fish(struct SquareData *felt, union FishBrain *mem) {
	int i, d, n;
	
	d = n = 0;
	for(i = 1; i < 5; i++) if(felt[i].Team && felt[i].NumAnts > n) n = felt[d = i].NumAnts;
	if(d) {
		mem[0].brain = 0;
		mem[0].pos = POSITION_WAR;
		return d;
	}

	if(felt[0].Base) {
		if(felt[0].NumAnts == 1) return 0;
		if(mem[1].brain & ~1) mem[1].brain = mem[0].brain & 1;
		mem[0].brain = 0;
		d = min(min(felt[0].NumAnts,felt[2].NumAnts),felt[4].NumAnts)>>DEFENCE_SHIFT;
		if(felt[1].NumAnts < felt[3].NumAnts) {
			if(felt[1].NumAnts < d) {
				mem[0].pos = POSITION_WAR;
				return 1;
			}
		} else {
			if(felt[3].NumAnts < d) {
				mem[0].pos = POSITION_WAR;
				return 3;
			}
		}
		mem[1].brain = (mem[1].brain + 1) & 1;
		mem[0].dir = (mem[1].brain << 1) + 1;
		mem[0].pos = 1;
		return ((mem[0].dir + 2) & 3) + 1;
	}

	if(mem[0].signpost) return 0;

	if(mem[0].pos == POSITION_WAR) {
		if(mem[0].found) {
			if(felt[0].NumAnts > 1 && mem[1].signpost) {
				if(!mem[1].foodleft) mem[1].foodleft = 1;
				if(!mem[1].foodright) mem[1].foodright = 1;
			}
			return mem[0].dir + 9;
		}
		return 0;
	}

	if((mem[0].dir & 1) && !mem[0].pos) {
		if(felt[0].NumAnts > 1) {
			if(!mem[1].signpost) {
				mem[1].signpost = 1;
				mem[1].orient = (mem[0].dir >> 1);
				mem[1].foodright = mem[1].foodleft = 1;
			}
		} else {
			mem[0].signpost = 1;
			mem[0].orient = (mem[0].dir >> 1);
			mem[0].foodright = mem[0].foodleft = 1;
			return 0;
		}
	}

	if(felt[0].NumAnts > 1 && mem[1].signpost) {
		if(mem[0].dir == ((!mem[1].orient) << 1) + 1) {
			mem[0].pos = POSITION_WAR;
			mem[0].found = 1;
			return 9 + mem[0].dir;
		}
		mem[0].pos = 0;
		if(mem[0].found) {
			switch(mem[0].dir) {
			case 0:
				mem[1].foodleft = FOUND_FOOD_AMOUNT;
				break;
			case 2:
				mem[1].foodright = FOUND_FOOD_AMOUNT;
				break;
			}
		}
		mem[0].brain = 0;
		mem[0].dir = (mem[1].orient << 1) + 1;
		if(felt[0].NumFood) return mem[0].dir + 9;
		if(felt[1].NumFood > felt[1].NumAnts) {
			mem[0].dir = 2;
			return 1;
		}
		if(felt[3].NumFood > felt[3].NumAnts) {
			mem[0].dir = 0;
			return 3;
		}
		if(mem[1].foodleft || mem[1].foodright) {
			if(mem[1].foodleft > mem[1].foodright) {
				mem[1].foodleft--;
				mem[0].dir = 0;
			} else {
				mem[1].foodright--;
				mem[0].dir = 2;
			}
		}
		return ((mem[0].dir + 2) & 3) + 1;
	} else {
		mem[0].found = 0;
		if(felt[0].NumFood) {
			mem[0].found = 1;
			if(mem[0].dir & 1) return mem[0].dir + 9;
			switch(mem[0].pos) {
			case 1: return 10;
			case 2: return 12;
			}
			return mem[0].dir + 9;
		} else {
			if(!(mem[0].dir & 1)) {
				if(felt[2].NumFood > felt[2].NumAnts) return 2;
				if(felt[4].NumFood > felt[4].NumAnts) return 4;
			}
			return ((mem[0].dir + 2) & 3) + 1;
		}
	}
}

int FishBone(struct SquareData *felt, union FishBrain *mem) {
	int temp, d, retval = Fish(felt, mem);
	d = retval & 7;
	if((mem[0].pos == POSITION_WAR) && !mem[0].signpost) return d;
	if(d) {
		if(felt[d].NumAnts >= MaxSquareAnts) {
			retval = d = ((d+1)&3)+1;
			temp = mem[0].dir;
			mem[0].brain = 0;
			mem[0].dir = temp;
		}
		switch (d) {
		case 2: mem[0].pos = (mem[0].pos + 2) % 3; break;
		case 4: mem[0].pos = (mem[0].pos + 1) % 3; break;
		}
	}
	return retval;;
}

DefineAnt(FishBone, "FishBone#DDDDDD", FishBone, union FishBrain)
