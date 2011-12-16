/*
---------------------------------------------------------------------
"SkyNET" version 1.1 - implementeret af Jørn Holm (gekko@cs.auc.dk).
---------------------------------------------------------------------
---------------------------------------------------------------------
Kommentar: Magien sker på informations motorvejen (ikke internettet).
Koden er relativ rodet og frarådes at forstå.  
---------------------------------------------------------------------
---------------------------------------------------------------------
Ændringer i version 1.1:
-Mad søgning og koordinering let forbedret.
-Har udskiftet det alm. mad warp drive med mad transwarp drive.
-Når 2 informations motorveje mødes, skabes 2 nye.  
---------------------------------------------------------------------
*/

#include "Myre.h"
int SkyNET(struct SquareData *f, u_char *x){
	int a;
	//-----------------------------------------------------------------------------------------
	//Checker for fjender.
	if(f[1].Team){
		x[0] = 255; return 1;
    	}
    	if(f[2].Team){
    		x[0] = 255; return 2;
    	}
    	if(f[3].Team){
    		x[0] = 255; return 3;
    	}
    	if(f[4].Team){
    		x[0] = 255; return 4;
    	}
	//-----------------------------------------------------------------------------------------
	//Basen
	if (f->Base == 1)
		return skynetBase(x);
	//-----------------------------------------------------------------------------------------
	//Vagt
	if(x[0] == 255){
		if (f->NumFood > 0 && f->NumAnts > 1)
			x[0] = x[1];
		else
			return skynetDFense(x,f);
	}
	//-----------------------------------------------------------------------------------------
	//Nye fraktioner
	for (a = 1; a < f->NumAnts; a++) {//Checker om der findes en fraktions leder
		if (x[a] == 0){
			if (f->NumFood > 0){
				if (x[0] > 127){
					x[0] = 136; return 9;
				}
				x[0] = 9; return 11;
			}
			if (x[0] < 128){
				x[0] = 64; return 4;
			}
			else{
				x[0] = 192; return 2;
			}
		}
	}
	if (x[0] < 7 && x[0] > 0){
		for (a = 1; a < f->NumAnts; a++){
			if (x[a] > 127 && x[a] < 134){
				x[0] = 0; return 0;
			}
		}	
	}
	if (x[0] > 127 && x[0] < 134){
		for (a = 1; a < f->NumAnts; a++){
			if (x[a] < 7 && x[a] > 0){
				x[0] = 0; return 0;
			}
		}
	}
	if (x[0] == 0)
		return 0;
			
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 191){
			if (f->NumFood > 0){
				if (x[0] > 127){
					x[0] = 200; return 12;	
				}
				x[0] = 72; return 10;
			}
			if (x[0] < 128){
				x[0] = 1; return 1;
			}
			else{
				x[0] = 128; return 3;
			}
		}
	}
	if (x[0] < 70 && x[0] > 63){
		for (a = 1; a < f->NumAnts; a++){
			if (x[a] > 191 && x[a] < 198){
				x[0] = 191; return 0;
			}
		}	
	}
	if (x[0] > 191 && x[0] < 198){
		for (a = 1; a < f->NumAnts; a++){
			if (x[a] < 70 && x[a] > 63){
				x[0] = 191; return 0;
			}
		}
	}
	if (x[0] == 191){
		return 0;
	}
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------
	//Højre fraktionen
	if (x[0] < 64){
		if (x[0] == 60)//Lige linie søger fra basen
			return skynetRightStraightLiner(x,f);
		if (x[0] < 4 && x[0] > 0)//Arbejdsløse
			return skynetRightUnemployed(x,f);
		if (x[0] == 4)//Knude uden søgere
			return skynetRightNodeNoSeekers(x,f);
		if (x[0] == 5)//return 1 knude med nord søger
			return skynetRightNodeUpper(x,f);
		if (x[0] == 10)//return 1 knude med syd søger
			return skynetRightNodeLower(x,f);
		if (x[0] == 6)
			return skynetRightNodeSeekersPresent(x,f);
		//------------------------------------------------------
		//Højre Nord Søger
		if (x[0] > 18 && x[0] < 21)
			return skynetRightNodeNorthSeekerLaneSwitch(x);
		if (x[0] == 7)
			return skynetRightNorthSeeker(x,f);
		if (x[0] == 11){
			x[0] = 7; return 11;	
		}
		if (x[0] == 12){
			x[0] = 7; return 9;	
		}
		if (x[0] == 23){
			x[0] = 19; return 1;
		}
		//------------------------------------------------------
		//Højre syd søger
		if (x[0] > 15 && x[0] < 18)
			return skynetRightNodeSouthSeekerLaneSwitch(x);
		if (x[0] == 8)
			return skynetRightSouthSeeker(x,f);
		if (x[0] == 14){
			x[0] = 8; return 11;	
		}
		if (x[0] == 15){
			x[0] = 8; return 9;	
		}
		if (x[0] == 22){
			x[0] = 16; return 1;	
		}
		//------------------------------------------------------
		if (x[0] == 9)//Trucker
			return 11;
		if (x[0] == 61){//Straight liner der vender tilbage efter madfund
			x[0] = 60; return 10;
		}
		if (x[0] == 62){//Straight liner der vender tilbage efter madfund
			x[0] = 60; return 12;
		}
	}
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------
	//Venstre fraktionen
	if (x[0] > 127 && x[0] < 191){
		if (x[0] == 180)//Lige linie søger fra basen	
			return skynetLeftStraightLiner(x,f);
		if (x[0] < 131 && x[0] > 127)
			return skynetLeftUnemployed(x,f);
		if (x[0] == 131)
			return skynetLeftNodeNoSeekers(x,f);
		if (x[0] == 132)//Venstre knude med nord søger
			return skynetLeftNodeUpper(x,f);
		if (x[0] == 137)//Venstre knude med syd søger
			return skynetLeftNodeLower(x,f);
		if (x[0] == 133)
			return skynetLeftNodeSeekersPresent(x,f);
		//------------------------------------------------------
		//Venstre nord søger
		if (x[0] > 142 && x[0] < 145)
			return skynetLeftNodeNorthSeekerLaneSwitch(x);
		if (x[0] == 134)
			return skynetLeftNorthSeeker(x,f);
		if (x[0] == 138){
			x[0] = 134; return 11;	
		}
		if (x[0] == 139){
			x[0] = 134; return 9;	
		}
		if (x[0] == 149){
			x[0] = 143; return 3;	
		}
		//------------------------------------------------------
		//Venstre syd ting
		if (x[0] > 145 && x[0] < 148)
			return skynetLeftNodeSouthSeekerLaneSwitch(x);
		if (x[0] == 135)
			return skynetLeftSouthSeeker(x,f);
		if (x[0] == 141){
			x[0] = 135; return 11;	
		}
		if (x[0] == 142){
			x[0] = 135; return 9;	
		}
		if(x[0] == 150){
			x[0] = 146; return 3;	
		}
		//------------------------------------------------------
		if (x[0] == 136)//Trucker
			return 9;
		if (x[0] == 181){//Straight liner der vender tilbage efter madfund
			x[0] = 180; return 12;
		}
		if (x[0] == 182){//Straight liner der vender tilbage efter madfund
			x[0] = 180; return 10;
		}
	}
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------
	//Op fraktionen
	if (x[0] > 63 && x[0] < 128){
		if (x[0] == 120)//Lige linie søger fra basen	
			return skynetUpperStraightLiner(x,f);
		if (x[0] < 67 && x[0] > 63)//Arbejdsløs
			return skynetUpperUnemployed(x,f);
		if (x[0] == 67)//Knude uden søger
			return skynetUpperNodeNoSeekers(x,f);
		if (x[0] == 68)//Op knude med venstre søger
			return skynetUpperNodeLeft(x,f);
		if (x[0] == 73)//Op knude med højre søger
			return skynetUpperNodeRight(x,f);
		if (x[0] == 69)//Up knude med begge søgere
			return skynetUpperNodeSeekersPresent(x,f);
		//------------------------------------------------------
		//Op højre søger
		if (x[0] > 81 && x[0] < 84)
			return skynetUpperNodeRightSeekerLaneSwitch(x);
		if (x[0] == 70)
			return skynetUpperRightSeeker(x,f);
		if (x[0] == 74){
			x[0] = 70; return 12;	
		}
		if (x[0] == 75){
			x[0] = 70; return 10;	
		}
		if (x[0] == 86){
			x[0] = 82; return 4;
		}
		//------------------------------------------------------
		//Op venstre søger
		if (x[0] > 78 && x[0] < 81)
			return skynetUpperNodeLeftSeekerLaneSwitch(x);
		if (x[0] == 71)
			return skynetUpperLeftSeeker(x,f);
		if (x[0] == 77){
			x[0] = 71; return 12;	
		}
		if (x[0] == 78){
			x[0] = 71; return 10;	
		}
		if (x[0] == 85){
			x[0] = 79; return 4;	
		}
		//------------------------------------------------------
		if (x[0] == 72)//Trucker
			return 10;
		if (x[0] == 121){//Straight liner der vender tilbage efter madfund
			x[0] = 120; return 11;
		}
		if (x[0] == 122){//Straight liner der vender tilbage efter madfund
			x[0] = 120; return 9;
		}
	}
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------
	//Op fraktionen
	if (x[0] > 190){
		if (x[0] == 240)//Lige linie søgere fra basen	
			return skynetLowerStraightLiner(x,f);
		if (x[0] < 195 && x[0] > 191)//Arbejdsløs
			return skynetLowerUnemployed(x,f);
		if (x[0] == 195)//Knude uden søgere
			return skynetLowerNodeNoSeekers(x,f);
		if (x[0] == 196)//Ned knude med venstre søger
			return skynetLowerNodeLeft(x,f);
		if (x[0] == 201)//Ned knude med højre søger
			return skynetLowerNodeRight(x,f);
		if (x[0] == 197)//Knude med begge søgere
			return skynetLowerNodeSeekersPresent(x,f);
		//------------------------------------------------------
		//Ned venstre søger
		if (x[0] > 209 && x[0] < 212)
			return skynetLowerNodeLeftSeekerLaneSwitch(x);
		if (x[0] == 198)
			return skynetLowerLeftSeeker(x,f);
		if (x[0] == 202){
			x[0] = 198; return 12;	
		}
		if (x[0] == 203){
			x[0] = 198; return 10;	
		}
		if (x[0] == 214){
			x[0] = 210; return 2;
		}
		//------------------------------------------------------
		//Ned højre søger
		if (x[0] > 206 && x[0] < 209)
			return skynetLowerNodeRightSeekerLaneSwitch(x);		
		if (x[0] == 199)
			return skynetLowerRightSeeker(x,f);
		if (x[0] == 205){
			x[0] = 199; return 12;	
		}
		if (x[0] == 206){
			x[0] = 199; return 10;	
		}
		if (x[0] == 213){
			x[0] = 207; return 2;	
		}
		//------------------------------------------------------
		if (x[0] == 200)//Trucker
			return 12;
		if (x[0] == 241){//Straight liner der vender tilbage efter madfund
			x[0] = 240; return 11;
		}
		if (x[0] == 242){//Straight liner der vender tilbage efter madfund
			x[0] = 240; return 9;
		}
	}
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------
}
inline static int skynetBase(u_char *z){
	if (z[0] == 60){
		z[0] = 60;
		return 1;
	}
	if (z[0] == 240){
		z[0] = 240;
		return 2;
	}
	if (z[0] == 180){
		z[0] = 180;
		return 3;
	}
	if (z[0] == 120){
		z[0] = 120;
		return 4;
	}
	if (z[0] < 44){
		z[0] = 1;
		return 1;
	}
	if (z[0] > 43 && z[0] < 128){
		z[0] = 64;
		return 4;
	}
	if (z[0] > 127 && z[0] < 171){
		z[0] = 128;
		return 3;
	}
	if (z[0] > 170){
		z[0] = 192;
		return 2;
	}
}
inline static int skynetDFense(u_char *x, struct SquareData *f){
	if (f->NumFood > 0 && f[1].NumAnts > 0){
		return 1;
	}
	if (f->NumFood > 0 && f[2].NumAnts > 0){
		return 2;
	}
	if (f->NumFood > 0 && f[3].NumAnts > 0){
		return 3;
	}
	if (f->NumFood > 0 && f[4].NumAnts > 0){
		return 4;
	}
	return 0;	
}
inline static int skynetRightUnemployed(u_char *x, struct SquareData *f){
	int a;
	if (x[0] == 3) {//return 1 arbejdsløs
		if (f->NumFood > 0){//Checker for mad
			for (a = 1; a < f->NumAnts; a++) {
				if (x[a] == 9){//Hvis der befinder sig en trucker på feltet så bytter jeg identitet
					x[a] = x[0];
					x[0] = 9;
					return 11;	
				}	
			}
			x[0] = 9;//Så er det mad uden ejer, så er jeg trucker
			return 11;	
		}
		for (a = 1; a < f->NumAnts; a++) {//Checker om der er en knude på knude punktet
			if (x[a] == 4 || x[a] == 5 || x[a] == 10 || x[a] == 13){
				if (x[a] == 5){//Der er en knude med en nord søger
					x[0]	= 8;
					x[a] = 6;
					return 2;//Så går jeg mod syd
				}
				if (x[a] == 4){//Der er en knude uden søgere
					x[0] = 7;
					x[a] = 5;
					return 4;//Så går jeg mod nord
				}
				if (x[a] == 10){//Der er en knude med en sydsøger
					x[0] = 7;
					x[a] = 6;
					return 4;//Så går jeg mod nord
				}
				if (x[a] == 13){//??
					x[0] = 8;
					x[a] = 6;
					return 2;	
				}
			}
			if (x[a] == 6)//Der er allerede en knude med en nord og en syd søger 
				break;
		}
		if (a == f->NumAnts) {//Ingen knude - så er jeg knude
			x[0] = 4;	
			return 0;
		}
		else  {//Så er der ikke brug for mig her - jeg fortsætter mod næste knudepunkt
			x[0] = 1;//Dvs. jeg er stadig arbejdsløs
			return 1;	
		}
	}
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 9){
				x[a] = x[0];
				x[0] = 9;
				return 11;	
			}	
		}
		x[0] = 9;
		return 11;
	}
	x[0]++;
	return 1;
}
inline static int skynetLeftUnemployed(u_char *x, struct SquareData *f){
	int a;
	if (x[0] == 130) {
		if (f->NumFood > 0){
			for (a = 1; a < f->NumAnts; a++) {
				if (x[a] == 136){
					x[a] = x[0];
					x[0] = 136;
					return 9;	
				}	
			}
			x[0] = 136;
			return 9;	
		}
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 131 || x[a] == 132 || x[a] == 137 || x[a] == 140){
				if (x[a] == 132){
					x[0]	= 135;
					x[a] = 133;
					return 2;
				}
				if (x[a] == 131){
					x[0] = 134;
					x[a] = 132;
					return 4;
				}
				if (x[a] == 137){
					x[0] = 134;
					x[a] = 133;
					return 4;	
				}
				if (x[a] == 140){
					x[0] = 135;
					x[a] = 133;
					return 2;	
				}
			}
			if (x[a] == 133)
				break;
		}
		if (a == f->NumAnts){
			x[0] = 131;	
			return 0;
		}
		else{
			x[0] = 128;
			return 3;	
		}
	}
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 136){
				x[a] = x[0];
				x[0] = 136;
				return 9;	
			}	
		}
		x[0] = 136;
		return 9;	
	}
	x[0]++;
	return 3;	
}
inline static int skynetUpperUnemployed(u_char *x, struct SquareData *f){
	int a;
	if (x[0] == 66) {
		if (f->NumFood > 0){
			for (a = 1; a < f->NumAnts; a++) {
				if (x[a] == 72){
					x[a] = x[0];
					x[0] = 72;
					return 10;	
				}	
			}
			x[0] = 72;
			return 10;
		}
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 67 || x[a] == 68 || x[a] == 73 || x[a] == 76){
				if (x[a] == 68){
					x[0]	= 71;
					x[a] = 69;
					return 1;
				}
				if (x[a] == 67){
					x[0] = 70;
					x[a] = 68;
					return 3;
				}
				if (x[a] == 73){
					x[0] = 70;
					x[a] = 69;
					return 3;	
				}
				if (x[a] == 76){
					x[0] = 71;
					x[a] = 69;
					return 1;	
				}
			}
			if (x[a] == 69)
				break;
		}
		if (a == f->NumAnts) {
			x[0] = 67;	
			return 0;
		}
		else  {
			x[0] = 64;
			return 4;	
		}
	}
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 72){
				x[a] = x[0];
				x[0] = 72;
				return 10;	
			}	
		}
		x[0] = 72;
		return 10;
	}
	x[0]++;
	return 4;
}
inline static int skynetLowerUnemployed(u_char *x, struct SquareData *f){
	int a;
	if (x[0] == 194) {
		if (f->NumFood > 0){
			for (a = 1; a < f->NumAnts; a++) {
				if (x[a] == 200){
					x[a] = x[0];
					x[0] = 200;
					return 12;	
				}	
			}
			x[0] = 200;
			return 12;	
		}
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 195 || x[a] == 196 || x[a] == 201 || x[a] == 204){
				if (x[a] == 196){
					x[0]	= 199;
					x[a] = 197;
					return 1;
				}
				if (x[a] == 195){
					x[0] = 198;
					x[a] = 196;
					return 3;
				}
				if (x[a] == 201){
					x[0] = 198;
					x[a] = 197;
					return 3;	
				}
				if (x[a] == 204){
					x[0] = 199;
					x[a] = 197;
					return 1;	
				}
			}
			if (x[a] == 197)
				break;
		}
		if (a == f->NumAnts) {
			x[0] = 195;	
			return 0;
		}
		else  {
			x[0] = 192;
			return 2;	
		}
	}
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 200){
				x[a] = x[0];
				x[0] = 200;
				return 12;	
			}	
		}
		x[0] = 200;
		return 12;	
	}
	x[0]++;
	return 2;
}

inline static int skynetRightNodeNoSeekers(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 9){
				x[a] = x[0];
				x[0] = 9;
				return 11;	
			}
		}
		x[0] = 9;
		return 11;	
	}
	return 0;
}
inline static int skynetLeftNodeNoSeekers(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
			for (a = 1; a < f->NumAnts; a++) {
				if (x[a] == 136){
					x[a] = x[0];
					x[0] = 136;
					return 9;	
				}	
			}
			x[0] = 136;
			return 9;	
		}
		return 0;	
}
inline static int skynetUpperNodeNoSeekers(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 72){
				x[a] = x[0];
				x[0] = 72;
				return 10;	
			}	
		}
		x[0] = 72;
		return 10;	
	}
	return 0;
}
inline static int skynetLowerNodeNoSeekers(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 200){
				x[a] = x[0];
				x[0] = 200;
				return 12;	
			}	
		}
		x[0] = 200;
		return 12;	
	}
	return 0;	
}
inline static int skynetUpperNodeLeft(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 72){
				x[a] = x[0];
				x[0] = 72;
				return 10;	
			}	
		}
	}
	for (a = 1; a < f->NumAnts; a++) {//Kigger efter arbejdsløse - hvis en findes er den knude og jeg søger
		if (x[a] == 66){
			x[a] = 69;
			x[0]	= 71;
			return 1;
		}
	}
	return 0;
}
inline static int skynetUpperNodeRight(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 72){
				x[a] = x[0];
				x[0] = 72;
				return 10;	
			}	
		}
	}
	return 0;		
}
inline static int skynetLowerNodeLeft(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 200){
				x[a] = x[0];
				x[0] = 200;
				return 12;	
			}	
		}
	}
	for (a = 1; a < f->NumAnts; a++) {//Kigger efter arbejdsløse - hvis en findes er den knude og jeg søger
		if (x[a] == 194){
			x[a] = 197;
			x[0]	= 199;
			return 1;
		}
	}
	return 0;	
}
inline static int skynetLowerNodeRight(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 200){
				x[a] = x[0];
				x[0] = 200;
				return 12;	
			}	
		}
	}
	return 0;	
}
inline static int skynetRightNodeUpper(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 9){
				x[a] = x[0];
				x[0] = 9;
				return 11;	
			}
			
		}
	}
	for (a = 1; a < f->NumAnts; a++) {//Kigger efter arbejdsløse - hvis en findes er den knude og jeg søger
		if (x[a] == 3){
			x[a] = 6;
			x[0] = 8;
			return 2;
		}
	}
	return 0;
}
inline static int skynetRightNodeLower(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 9){
				x[a] = x[0];
				x[0] = 9;
				return 11;	
			}	
		}
	}
	return 0;		
}
inline static int skynetLeftNodeUpper(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 136){
				x[a] = x[0];
				x[0] = 136;
				return 9;	
			}	
		}
	}
	for (a = 1; a < f->NumAnts; a++) {//Kigger efter arbejdsløse - hvis en findes er den knude og jeg søger
		if (x[a] == 130){
			x[a] = 133;
			x[0]	= 135;
			return 2;
		}
	}
	return 0;	
}
inline static int skynetLeftNodeLower(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 136){
				x[a] = x[0];
				x[0] = 136;
				return 9;	
			}	
		}
	}
	return 0;	
}

inline static int skynetRightNodeSeekersPresent(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 9){
				x[a] = x[0];
				x[0] = 9;
				return 11;	
			}
		}
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 7){//Hjemvendt søger detekteret
				x[a] = 10;
				if (f->NumAnts > 2)
					x[a] = 4;
				x[0] = 9;
				return 11;
			}
			if (x[a] == 8){//Hjemvendt søger detekteret
				x[a] = 5;
				if (f->NumAnts > 2)
					x[a] = 4;
				x[0] = 9;
				return 11;
			}
		}
	}
	
	for (a = 1; a < f->NumAnts; a++) {//Kigger efter arbejdsløse jeg kan bytte med
		if (x[a] == 3){
			x[a] = 6;
			x[0]	= 1;
			return 1;
		}
	}
	/*
	if (f->NumFood > 0){
		if (f->NumAnts > 1)
			printf("\n%d\n", x[1]);
		x[0] = 9;
		return 11;	
	}
	*/
	return 0;
}
inline static int skynetLeftNodeSeekersPresent(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 136){
				x[a] = x[0];
				x[0] = 136;
				return 9;	
			}	
		}
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 134){//Hjemvendt søger detekteret
				x[a] = 137;
				if (f->NumAnts > 2)
					x[a] = 131;
				x[0] = 136;
				return 9;
			}
			if (x[a] == 135){//Hjemvendt søger detekteret
				x[a] = 132;
				if (f->NumAnts > 2)
					x[a] = 131;
				x[0] = 136;
				return 9;
			}
		}
	}
	
	for (a = 1; a < f->NumAnts; a++) {//Kigger efter arbejdsløse jeg kan bytte med
		if (x[a] == 130){
			x[a] = 133;
			x[0]	= 128;
			return 3;
		}
	}
	
	return 0;	
}
inline static int skynetUpperNodeSeekersPresent(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 72){
				x[a] = x[0];
				x[0] = 72;
				return 10;	
			}	
		}
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 70){//Hjemvendt søger detekteret
				x[a] = 73;
				if (f->NumAnts > 2)
					x[a] = 67;
				x[0] = 72;
				return 10;
			}
			if (x[a] == 71){//Hjemvendt søger detekteret
				x[a] = 68;
				if (f->NumAnts > 2)
					x[a] = 67;
				x[0] = 72;
				return 10;
			}
		}
	}
	
	for (a = 1; a < f->NumAnts; a++) {//Kigger efter arbejdsløse jeg kan bytte med
		if (x[a] == 66){
			x[a] = 69;
			x[0] = 64;
			return 4;
		}
	}
	
	return 0;
}
inline static int skynetLowerNodeSeekersPresent(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 200){
				x[a] = x[0];
				x[0] = 200;
				return 12;	
			}	
		}
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 198){//Hjemvendt søger detekteret
				x[a] = 201;
				if (f->NumAnts > 2)
					x[a] = 195;
				x[0] = 200;
				return 12;
			}
			if (x[a] == 199){//Hjemvendt søger detekteret
				x[a] = 196;
				if (f->NumAnts > 2)
					x[a] = 195;
				x[0] = 200;
				return 12;
			}
		}
	}
	
	for (a = 1; a < f->NumAnts; a++) {//Kigger efter arbejdsløse jeg kan bytte med
		if (x[a] == 194){
			x[a] = 197;
			x[0]	= 192;
			return 2;
		}
	}
	
	return 0;
}
inline static int skynetRightNodeNorthSeekerLaneSwitch(u_char *x){
	x[0]++;
	if (x[0] == 21){
		x[0] = 7;
	}
	return 1;
}
inline static int skynetRightNodeSouthSeekerLaneSwitch(u_char *x){
	x[0]++;
	if (x[0] == 18){
		x[0] = 8;
	}
	return 1;	
}
inline static int skynetLeftNodeNorthSeekerLaneSwitch(u_char *x){
	x[0]++;
	if (x[0] == 145){
		x[0] = 134;
	}
	return 3;	
}
inline static int skynetLeftNodeSouthSeekerLaneSwitch(u_char *x){
	x[0]++;
	if (x[0] == 148){
		x[0] = 135;
	}
	return 3;	
}
inline static int skynetUpperNodeRightSeekerLaneSwitch(u_char *x){
	x[0]++;
	if (x[0] == 84){
		x[0] = 70;
	}
	return 4;
}
inline static int skynetUpperNodeLeftSeekerLaneSwitch(u_char *x){
	x[0]++;
	if (x[0] == 81){
		x[0] = 71;
	}
	return 4;	
}
inline static int skynetLowerNodeRightSeekerLaneSwitch(u_char *x){
	x[0]++;
	if (x[0] == 209){
		x[0] = 199;
	}
	return 2;	
}
inline static int skynetLowerNodeLeftSeekerLaneSwitch(u_char *x){
	x[0]++;
	if (x[0] == 212){
		x[0] = 198;
	}
	return 2;
}
inline static int skynetRightNorthSeeker(u_char *x, struct SquareData *f){
	int a;	
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 4 || x[a] == 5 || x[a] == 6 || x[a] == 10){
				if (x[a] == 6){
					x[a] = 10;
					if (f->NumAnts > 2)
						x[a] = 4;
					x[0] = 9;
					return 11;
				}
				x[0] = 9;
				x[a] = 4;
				return 11;
			}	
		}
		if (f->NumAnts <= f->NumFood)
			return 10;	
	}
	if (f[1].NumFood > 0 && f[1].NumAnts < f[1].NumFood){
		x[0] = 11;
		return 1;	
	}
	if (f[3].NumFood > 0 && f[3].NumAnts < f[3].NumFood){
		x[0] = 12;
		return 3;	
	}
	/*
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 8 && f->NumAnts == 2 && f->NumFood == 0){
			x[a] = 23;
			x[0] = 16;
			return 1;
		}
	}
	*/
	return 4;
}
inline static int skynetRightSouthSeeker(u_char *x, struct SquareData *f){
	int a;	
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 4 || x[a] == 5 || x[a] == 6 || x[a] == 10){
				if (x[a] == 6){
					x[a] = 5;
					if (f->NumAnts > 2)
						x[a] = 4;
					x[0] = 9;
					return 11;
				}
				x[0] = 9;
				x[a] = 4;
				return 11;
			}	
		}
		if (f->NumAnts <= f->NumFood)
		return 12;	
	}
	if (f[1].NumFood > 0 && f[1].NumAnts < f[1].NumFood){
		x[0] = 14;
		return 1;	
	}
	if (f[3].NumFood > 0 && f[3].NumAnts < f[3].NumFood){
		x[0] = 15;
		return 3;	
	}
	/*
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 7 && f->NumAnts == 2 && f->NumFood == 0){
			x[a] = 22;
			x[0] = 19;
			return 1;
		}
	}
	*/
	return 2;	
}
inline static int skynetLeftNorthSeeker(u_char *x, struct SquareData *f){
	int a;	
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 131 || x[a] == 132 || x[a] == 133 || x[a] == 137){
				if (x[a] == 133){
					x[a] = 137;
					if (f->NumAnts > 2)
						x[a] = 131;
					x[0] = 136;
					return 9;
				}
				x[0] = 136;
				x[a] = 131;
				return 9;
			}	
		}
		if (f->NumAnts <= f->NumFood)
			return 10;	
	}
	if (f[1].NumFood > 0 && f[1].NumAnts < f[1].NumFood){
		x[0] = 138;
		return 1;	
	}
	if (f[3].NumFood > 0 && f[3].NumAnts < f[3].NumFood){
		x[0] = 139;
		return 3;	
	}
	/*
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 135 && f->NumAnts == 2 && f->NumFood == 0){
			//x[a] = 149;
			x[0] = 146;
			return 3;
		}
	}
	*/
	return 4;
}
inline static int skynetLeftSouthSeeker(u_char *x, struct SquareData *f){
	int a;	
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 131 || x[a] == 132 || x[a] == 133 || x[a] == 137){
				if (x[a] == 133){
					x[a] = 132;
					if (f->NumAnts > 2)
						x[a] = 131;
					x[0] = 136;
					return 9;
				}
				x[0] = 136;
				x[a] = 131;
				return 9;
			}	
		}
		if (f->NumAnts <= f->NumFood)
		return 12;	
	}
	if (f[1].NumFood > 0 && f[1].NumAnts < f[1].NumFood){
		x[0] = 141;
		return 1;	
	}
	if (f[3].NumFood > 0 && f[3].NumAnts < f[3].NumFood){
		x[0] = 142;
		return 3;	
	}
	/*
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 134 && f->NumAnts == 2 && f->NumFood == 0){
			x[a] = 150;
			x[0] = 143;
			return 3;
		}
	}
	*/
	return 2;	
}
inline static int skynetUpperRightSeeker(u_char *x, struct SquareData *f){
	int a;	
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 67 || x[a] == 68 || x[a] == 69 || x[a] == 73){
				if (x[a] == 69){
					x[a] = 73;
					if (f->NumAnts > 2)
						x[a] = 67;
					x[0] = 72;	
					return 10;
				}
				x[0] = 72;
				x[a] = 67;
				return 10;
			}	
		}
		if (f->NumAnts <= f->NumFood)
			return 11;	
	}
	if (f[2].NumFood > 0 && f[2].NumAnts < f[2].NumFood){
		x[0] = 74;
		return 2;	
	}
	if (f[4].NumFood > 0 && f[4].NumAnts < f[4].NumFood){
		x[0] = 75;
		return 4;	
	}
	/*
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 8+63 && f->NumAnts == 2 && f->NumFood == 0){
			x[a] = 86;
			x[0] = 79;
			return 4;
		}
	}
	*/
	return 1;
}
inline static int skynetUpperLeftSeeker(u_char *x, struct SquareData *f){
	int a;	
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 67 || x[a] == 68 || x[a] == 69 || x[a] == 73){
				if (x[a] == 69){
					x[a] = 68;
					if (f->NumAnts > 2)
						x[a] = 67;
					x[0] = 72;
					return 10;
				}
				x[0] = 72;
				x[a] = 67;
				return 10;
			}	
		}
		if (f->NumAnts <= f->NumFood)
			return 9;	
	}
	if (f[2].NumFood > 0 && f[2].NumAnts < f[2].NumFood){
		x[0] = 77;
		return 2;	
	}
	if (f[4].NumFood > 0 && f[4].NumAnts < f[4].NumFood){
		x[0] = 78;
		return 4;	
	}
	/*
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 70 && f->NumAnts == 2 && f->NumFood == 0){
			x[a] = 85;
			x[0] = 82;
			return 4;
		}
	}
	*/
	return 3;
}
inline static int skynetLowerRightSeeker(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 195 || x[a] == 196 || x[a] == 197 || x[a] == 201){
				if (x[a] == 197){
					x[a] = 196;
					if (f->NumAnts > 2)
						x[a] = 195;
					x[0] = 200;
					return 12;
				}
				x[0] = 200;
				x[a] = 195;
				return 12;
			}	
		}
		if (f->NumAnts <= f->NumFood)
			return 11;	
	}
	if (f[2].NumFood > 0 && f[2].NumAnts < f[2].NumFood){
		x[0] = 205;
		return 2;	
	}
	if (f[4].NumFood > 0 && f[4].NumAnts < f[4].NumFood){
		x[0] = 206;
		return 4;	
	}
	/*
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 198 && f->NumAnts == 2 && f->NumFood == 0){
			x[a] = 213;
			x[0] = 210;
			return 2;
		}
	}
	*/
	return 1;
}
inline static int skynetLowerLeftSeeker(u_char *x, struct SquareData *f){
	int a;	
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 195 || x[a] == 196 || x[a] == 197 || x[a] == 201){
				if (x[a] == 197){
					x[a] = 201;
					if (f->NumAnts > 2)
						x[a] = 195;
					x[0] = 200;	
					return 12;
				}
				x[0] = 200;
				x[a] = 195;
				return 12;
			}	
		}
		if (f->NumAnts <= f->NumFood)
			return 9;	
	}
	if (f[2].NumFood > 0 && f[2].NumAnts < f[2].NumFood){
		x[0] = 202;
		return 2;	
	}
	if (f[4].NumFood > 0 && f[4].NumAnts < f[4].NumFood){
		x[0] = 203;
		return 4;	
	}
	/*
	for (a = 1; a < f->NumAnts; a++) {
		if (x[a] == 199 && f->NumAnts == 2 && f->NumFood == 0){
			x[a] = 214;
			x[0] = 207;
			return 2;
		}
	}
	*/
	return 3;
}
inline static int skynetRightStraightLiner(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0 && f->NumAnts <= f->NumFood)
		return 11;
	if (f[2].NumFood > 0 && f[2].NumFood > f->NumAnts){
		x[0] = 62;
		return 2;
	}
	if (f[4].NumFood > 0 && f[4].NumFood > f->NumAnts){
		x[0] = 61;
		return 4;
	}
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 9){
				x[a] = x[0];
				x[0] = 9;
				return 11;	
			}	
		}
	}
	return 1;		
}
inline static int skynetLeftStraightLiner(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0 && f->NumAnts <= f->NumFood)
		return 9;
	if (f[2].NumFood > 0 && f[2].NumFood > f->NumAnts){
		x[0] = 181;
		return 2;
	}
	if (f[4].NumFood > 0 && f[4].NumFood > f->NumAnts){
		x[0] = 182;
		return 4;
	}
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 136){
				x[a] = x[0];
				x[0] = 136;
				return 9;	
			}	
		}
	}
	return 3;
}
inline static int skynetUpperStraightLiner(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0 && f->NumAnts <= f->NumFood)
		return 10;
	if (f[1].NumFood > 0 && f[1].NumFood > f->NumAnts){
		x[0] = 121;
		return 1;
	}
	if (f[3].NumFood > 0 && f[3].NumFood > f->NumAnts){
		x[0] = 122;
		return 3;
	}
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 72){
				x[a] = x[0];
				x[0] = 72;
				return 10;	
			}	
		}
	}
	return 4;
}
inline static int skynetLowerStraightLiner(u_char *x, struct SquareData *f){
	int a;
	if (f->NumFood > 0 && f->NumAnts <= f->NumFood)
		return 12;
	if (f[1].NumFood > 0 && f[1].NumFood > f->NumAnts){
		x[0] = 241;
		return 1;
	}
	if (f[3].NumFood > 0 && f[3].NumFood > f->NumAnts){
		x[0] = 242;
		return 3;
	}
	if (f->NumFood > 0){
		for (a = 1; a < f->NumAnts; a++) {
			if (x[a] == 200){
				x[a] = x[0];
				x[0] = 200;
				return 12;	
			}	
		}
	}
	return 2;
}
DefineAnt(SkyNET,"SkyNET#6666FF",SkyNET,u_char);
