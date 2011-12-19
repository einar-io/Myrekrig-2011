/*
---------------------------------------------------------------------
"Legions" version 1.2 - implementeret af Jørn Holm (gekko@cs.auc.dk).
---------------------------------------------------------------------

---------------------------------------------------------------------
Ændringer i version 1.2:
-Performance optimeret (via simplere 'path planning' algoritmer).
-Forsvars system væsentligt forstærket.
---------------------------------------------------------------------
Ændringer i version 1.1:
-Performance optimeret.
-Filstørrelse reduceret.
-Forsvars system ændret.
-'Booster' start optimeret.
---------------------------------------------------------------------
*/
#include "Myre.h"
#define abs(x) ((x) >= 0 ? (x) : -(x))
struct LegionsBrain {
	signed char x,y,z,w;
	unsigned char v;
};
int Legions(struct SquareData *f, struct LegionsBrain *m) {
	unsigned int a;
	if (f->Base == 1){
		if (m->v != 255){
    			for (a = 1; a < f->NumAnts; a++){
    				if (m[a].v == 255){
    					if (m->v == 5 || m->v == 14){
    						m[a].x = m->z;
    						m[a].y = m->w;
    						if (m->v == 14)
    							m[a].z = 6;
    						if (m->v == 5)
    							m[a].z = 15;
					}
    					else if (m->v == 0 && (m[a].x != 0 || m[a].y != 0)){
    						m->z = m[a].x;
    						m->w = m[a].y;
    						m->x = m->y = 0;
    						m->v = m[a].z;
					}
					else if (m[a].x == 0 && m[a].y == 0){
    						if (f->NumAnts > 2){
    								unsigned int b;
    								for (b = 1; b < f->NumAnts; b++){
    									if (m[b].v == 5 || m[b].v == 14){
    										m[0] = m[b];
    									}
    								}
    						}
    					}
    					a = 1024;
				}
			}
    			if (a != 1025){
    				unsigned int xs[50];
    				unsigned int b = 3;
    				m->x = m->y = m->z = m->w = 0;
    				m->v = 255;
    				xs[0] = 3;xs[1] = 6;xs[2] = 9;xs[3] = 12;xs[4] = 15;xs[5] = 18;xs[6] = 21;xs[7] = 24;xs[8] = 27;xs[9] = 30;xs[10] = 33;xs[11] = 36;xs[12] = 39;xs[13] = 42;xs[14] = 45;xs[15] = 48;xs[16] = 51;xs[17] = 54;xs[18] = 57;xs[19] = 60;xs[20] = 63;xs[21] = 66;xs[22] = 69;xs[23] = 72;xs[24] = 75;xs[25] = 78;xs[26] = 81;xs[27] = 84;xs[28] = 87;xs[29] = 90;xs[30] = 93;xs[31] = 96;xs[32] = 99;xs[33] = 102;xs[34] = 105;xs[35] = 108;xs[36] = 111;xs[37] = 114;xs[38] = 117;xs[39] = 120;xs[40] = 123;xs[41] = 126;xs[42] = 129;xs[43] = 132;xs[44] = 135;xs[45] = 138;xs[46] = 141;xs[47] = 144;xs[48] = 147;
				for (a = 1; a < f->NumAnts; a++){
					m[a].v = 30;
					m[a].x = m[a].y = 0;
					if ((b&3) == 0){
						m[a].z = m[a].w = xs[b/4];
					}
					else if ((b&3) == 1){
						m[a].z = m[a].w = -xs[b/4];
					}
					else if ((b&3) == 2){
						m[a].w = xs[b/4];
						m[a].z = -xs[b/4];
					}
					else {
						m[a].w = -xs[b/4];
						m[a].z = xs[b/4];
					}
					b++;
				}
    				return 16;
    			}
		}
	}
	if (m->v == 30){
		if (m->x > 111 || m->x < -111)
			m->x = 0;
		if (m->y > 111 || m->y < -111)
			m->y = 0;
		a = lwc(f);
    		if (a != 0){
    			m->w = 12;
    			m->z = 0;
    			m->v = 17;
    			return lnpt(m,a);
    		}
		a = lfc(f);
		if (a != 5){
			if (m->z%6 == 0)
				m->v = 1;
			else
				m->v = 10;
		}
		else {
	   		if (m->y != m->w || m->x != m->z){
	   			return lnpt(m,lgtnipo(m));
	   		}
	   		m->z = 0;
	   		if (m->x < 0 && m->y < 0)
	   			m->w = 3;
	   		else if (m->x < 0 && m->y > 0)
	   			m->w = 4;
	   		else if (m->x > 0 && m->y < 0)
	   			m->w = 2;
	   		else
	   			m->w = 1;
	   		m->v = 31;
	   	}
	}
	if (m->v == 31){
		if (m->x > 111 || m->x < -111)
			m->x = 0;
		if (m->y > 111 || m->y < -111)
			m->y = 0;
		a = lwc(f);
    		if (a != 0){
    			m->w = 8;
    			m->z = 0;
    			m->v = 17;
    			return lnpt(m,a);
    		}
		a = lfc(f);
		if (a == 5){
			if (abs(m->x) == abs(m->y)){
				if (m->z == 1){
					if (m->x > 0 && m->y > 0){
						m->z = m->x + 9;
						m->w = m->y + 9;
					}
					else if (m->x > 0 && m->y < 0){
						m->z = m->x + 9;
						m->w = m->y - 9;
					}
					else if (m->x < 0 && m->y > 0){
						m->z = m->x - 9;
						m->w = m->y + 9;
					}
					else {
						m->z = m->x - 9;
						m->w = m->y - 9;
					}
					m->v = 30;
					return lnpt(m,lgtnipo(m));
				}
				if ((abs(m->x) == abs(m->x)) && (m->z == 0))
					m->z = 1;
				m->w++;
				if (m->w > 4)
					m->w = 1;
			}
			return lnpt(m,m->w);
		}
		else {
			m->v = 10;
			if (m->z%6 == 0)
				m->v = 1;
		}
	}
    	if (m->v == 0){
    		if (m->x < 0)
    			m->v = 9;
    		m->w = 0;
	}
    	if (m->v < 9){
    		if (m->x > 111 || m->x < -111)
			m->x = 0;
		if (m->y > 111 || m->y < -111)
			m->y = 0;
    		a = lwc(f);
    		if (a != 0){
    			m->w = 0;
    			m->z = 0;
    			m->v = 8;
    			return lnpt(m,a);
		}
    		if (f->NumFood > 49)
    			return 16;
    		if (m->v == 8){
    			if (f->NumFood > 0 && f->NumAnts == 1)
    				return lnpt(m,lsu(m));
    			if (m->z > 100){
    				m->z = 0;
    				m->w++;	
    			}
    			m->z++;
    			if (m->w > 30){
    				m->w = 7;
    				m->z = 3;
    				m->v = abs(m->x%4+10);
    			}   			
    			return 0;
	    	}
    		if (f->NumAnts > 1 && m->v < 5 && m->v != 0)
    			m->z = m->z + f->NumAnts;
		if (((m->x == 0 && m->y == 0) || f->Base == 1) && (m->v == 0 || m->v == 5)){
			if (m->v == 5)
				m->v = 6;
			else{
    				m->w = abs(m->z%40+(m->z%20+3));
    				m->v = abs(m->z%5);
    				m->z = abs(m->z%20+3);
    				m->x = m->y = 0;
			}
		}
		if (m->v == 5){
			for (a = 1; a < f->NumAnts; a++){
				if ((m[a].v < 5) || ((m[a].v < 14 && m[a].v > 9))){
					m[a].x = m->x;
					m[a].y = m->y;
					m[a].z = m->z;
					m[a].w = m->w;
					m[a].v = 15;
				}
			}
			return lnpt(m,lgtb(m))+8;
		}
		if (f->NumFood > 0 && f->NumAnts <= f->NumFood && m->v != 5 && (m->x != 0 || m->y != 0)){
	    		m->z = m->x;
	    		m->w = m->y;
	    		if (f->NumFood == 1)
	    			m->z = m->w = 0;
    			m->v = 5;
    			for (a = 1; a < f->NumAnts; a++){
				if ((m[a].v < 5) || ((m[a].v < 14 && m[a].v > 9))){
					m[a].x = m->x;
					m[a].y = m->y;
					m[a].z = m->z;
					m[a].w = m->w;
					m[a].v = 15;
				}
			}
			return lnpt(m,lgtb(m))+8;
		}
		if (m->v == 6){
			if (m->y != m->w || m->x != m->z){
				for (a = 1; a < f->NumAnts; a++){
					if ((m[a].v < 5) || ((m[a].v < 14 && m[a].v > 9))){
						m[a].x= m->x;
						m[a].y = m->y;
						m[a].z = m->z;
						m[a].w = m->w;
						m[a].v = 15;
					}
				}
				return lnpt(m,lgtnp(m));
			}
			m->w = 21;
			m->v = abs(m->x%4+1);
			m->z = 3;
			return lnpt(m,lds1(m));
		}
		a = lfc(f);
		if (a != 5  && a != 0)
			return lnpt(m,a);
		if (f->NumAnts > 1 || f[1].NumAnts > 0 || f[2].NumAnts > 0 || f[3].NumAnts > 0 || f[4].NumAnts > 0){
			m->z = m->z + f->NumAnts + 3;
		}
		return lnpt(m,lds1(m));
	}
	else {
		if (m->x > 111 || m->x < -111)
			m->x = 0;
		if (m->y > 111 || m->y < -111)
			m->y = 0;
    		a = lwc(f);
    		if (a != 0){
    			m->w = 0;
    			m->z = 0;
    			m->v = 17;
    			if (f->NumAnts > 1 && f->NumFood == 0){
    				int b;
    				for (b = 1; b < f->NumAnts; b++){
    					m[b] = m[0];
    					m[b].w = 10;
    				}
    			}
    			return lnpt(m,a);
		}
    		if (f->NumFood > 49)
    			return 16;
    		if (m->v == 17){
    			if (m->z > 100){
    				m->z = 0;
    				m->w++;	
    			}
    			m->z++;
    			if (m->w > 16){
    				m->w = 7;
    				m->z = 3;
    				m->v = abs(m->x%4+1);
    			}
    			if (f->NumFood > 0 && f->NumAnts == 1)
    				return lnpt(m,lsu(m));
    			return 0;
		}
	    	if (f->NumAnts > 1 && m->v < 14 && m->v > 9)
	    		m->z = m->z + f->NumAnts;
		if (((m->x == 0 && m->y == 0) || f->Base == 1) && (m->v == 9 || m->v == 14)){
			if (m->v == 14)
				m->v = 15;
			else{
					m->w = abs(m->z%40+(m->z%20+3));
					m->v = abs(m->z%4+10);
					m->z = abs(m->z%20+3);
					m->y = m->x = 0;
			}
		}
		if (m->v == 255){
			return 16;
	    	}
		if (m->v == 14){
			for (a = 1; a < f->NumAnts; a++){
				if (m[a].v < 14 && m[a].v > 9){
					m[a].x = m->x;
					m[a].y = m->y;
					m[a].z = m->z;
					m[a].w = m->w;
					m[a].v = 6;
				}
			}
	    		return lnpt(m,lgtb(m))+8;
		}
		if (f->NumFood > 0 && f->NumAnts <= f->NumFood && m->v != 14 && (m->x != 0 || m->y != 0)){
				m->z = m->x;
				m->w = m->y;
	    			if (f->NumFood == 1)
	    				m->z = m->w = 0;
	    			m->v = 14;
	    			for (a = 1; a < f->NumAnts; a++){
					if (m[a].v < 14 && m[a].v > 9){
						m[a].x = m->x;
						m[a].y = m->y;
						m[a].z = m->z;
						m[a].w = m->w;
						m[a].v = 6;
					}
				}
		    		return lnpt(m,lgtb(m))+8;
		}
		if (m->v == 15){
			if (m->y != m->w || m->x != m->z) {
				for (a = 1; a < f->NumAnts; a++){
					if (m[a].v < 14 && m[a].v > 9){
						m[a].x= m->x;
						m[a].y = m->y;
						m[a].z = m->z;
						m[a].w = m->w;
						m[a].v = 6;
					}
				}
				return lnpt(m,lgtnp(m));
			}
			m->w = 21;
			m->v = abs(m->y%4+10);
			m->z = 3;
			return lnpt(m,lds2(m));
		}
		a = lfc(f);
		if (a != 5  && a != 0)
			return lnpt(m,a);
		if (f->NumAnts > 1 || f[1].NumAnts > 0 || f[2].NumAnts > 0 || f[3].NumAnts > 0 || f[4].NumAnts > 0){
				m->z = m->z + f->NumAnts;
		}
		return lnpt(m,lds2(m));
	}
}
inline static int lfc(struct SquareData *felt){
    	if (felt[0].NumFood)
    		return 0;
    	if (felt[1].NumFood && felt[1].NumAnts <= felt[1].NumFood)
    		return 1;
    	if (felt[2].NumFood && felt[2].NumAnts <= felt[2].NumFood)
    		return 2;
    	if (felt[3].NumFood && felt[3].NumAnts <= felt[3].NumFood)
    		return 3;
    	if (felt[4].NumFood && felt[4].NumAnts <= felt[4].NumFood)
    		return 4;
    	return 5;
}
inline static int lwc(struct SquareData *felt){
    	if(felt[1].Team)
    		return 1;
    	if(felt[2].Team)
    		return 2;
    	if(felt[3].Team)
    		return 3;
    	if(felt[4].Team)
    		return 4;
    	return 0;
}
inline static int lnpt(struct LegionsBrain *mem, int direction){
    	switch (direction) {
    		case 1: mem->x++;break;
    		case 2: mem->y--;break;
    		case 3: mem->x--;break;
    		case 4: mem->y++;break;
  	}
     return direction;
}
inline static int lds2(struct LegionsBrain *mem) {
    	if (mem->v < 10){
    		mem->v = 10;
    	}
    	if (mem->z > mem->w){
    		mem->v++;
    		mem->w = mem->w + 17;
    		mem->z = 1;
	}
    	if (mem->v > 13){
    		mem->z++;
    		mem->v = 9;
    		return 0;
    	}
    	mem->z++;
    	return mem->v-9;
}
inline static int lds1(struct LegionsBrain *mem) {
    	if (mem->v < 1){
    		mem->v = 4;
    	}
    	if (mem->z > mem->w){
    		mem->v--;
    		mem->w = mem->w + 20;
    		mem->z = 1;
	}
    	if (mem->v == 0)
    		mem->v = 4;
    	mem->z++;
    	return mem->v;
}
inline static int lgtnipo(struct LegionsBrain *mem){
	if (mem->z > 0 && mem->w > 0){
		if (mem->y < mem->w)
			return 4;
		if (mem->x < mem->z)	
			return 1;
	}
	if (mem->z > 0 && mem->w < 0){
		if (mem->x < mem->z)
			return 1;
		if (mem->y > mem->w)	
			return 2;
	}
	if (mem->z < 0 && mem->w > 0){
		if (mem->x > mem->z)	
			return 3;
		if (mem->y < mem->w)
			return 4;
	}
	if (mem->z < 0 && mem->w < 0){
		if (mem->y > mem->w)
			return 2;
		if (mem->x > mem->z)	
			return 3;
	}	
}
inline static int lgtnp(struct LegionsBrain *mem){
	if (mem->x < mem->z)
		return 1;
	if (mem->x > mem->z)
		return 3;
	if (mem->y < mem->w)
		return 4;
	if (mem->y > mem->w)
		return 2;
}
inline static int lgtb(struct LegionsBrain *mem){
	if (abs(mem->x) > abs(mem->y)){
		if (mem->x < 0)
			return 1;
		else
			return 3;
	}
	if (mem->y < 0)
		return 4;
	else
		return 2;
}
inline static int lsu(struct LegionsBrain *mem) {
    	unsigned int d = abs(mem->z)%3;
    	if (mem->y >= 0 && mem->x >= 0){
    		if (d == 0)
    			return 4;
    		else if (d == 1)
    			return 1;
		else {
    			if (mem->w == 0){
    				mem->w = 1;
    				return 4;
			}
			else{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
    				mem->w = 0;
    				return 1;
			}
		}
	}
    	else if (mem->y < 1 && mem->x >= 0){
    		if (d == 0)
    			return 2;
    		else if (d == 1)
    			return 1;
		else {
    			if (mem->w == 0){
    				mem->w = 1;
    				return 1;
			}
			else{
    				mem->w = 0;
    				return 2;
			}
		}
	}
    	else if (mem->y >= 0 && mem->x <= 1){
    		if (d == 0)
    			return 3;
    		else if (d == 1)
    			return 4;
		else {
    			if (mem->w == 0){
    				mem->w = 1;
    				return 3;
			}
			else{
    				mem->w = 0;
    				return 4;
			}
		}
	}
    	else if (mem->y <= 1 && mem->x <= 1){
    		if (d == 0)
    			return 2;
    		else if (d == 1)
    			return 3;
		else {
    			if (mem->w == 0){
    				mem->w = 1;
    				return 2;
			}
			else{
    				mem->w = 0;
    				return 3;
			}
		}
	}
}
DefineAnt(Legions,"Legions#393EF0", Legions, struct LegionsBrain);
