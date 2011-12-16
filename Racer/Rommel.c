/*
    Rommel en general basseret myre opkaldt efter en næsten ukendt tysk myre.
    Denne har endnu ikke lært at bygge baser, eller det vil sige den har fået
    ordre til ikke at bygge nogen. Dette er Rommel i midlertid ligeglad med 
    og kan godt finde på at bygge en. Myren er generelt defekt og har størst 
    anvendelse som skydeskive. Dvs. medmindre man ynder dens let pussige
    bevægelses mønster. Om performance kan man kun sige ringe, da den bruger
    en sinus tabel, da det ikke er lovligt at bruge globale variable, bliver 
    denne oprettet på stacken hver gang, og da myre ikke er hurtige til at
    lave memcpy tjaaa der røg den performance...
    Åh ja lige en sidste ting, så frygter denne myre kun en ting, dens 
    afløser Patton.
*/
#include "Myre.h"

enum {unknown=0, rommel, idle, searcher, goingHome, goTo, defender};

#define here 0
#define left 3
#define right 1
#define up 4
#define down 2
#define dragFood 8
#define buildBase 16

#define reserveNumber 10
#define foodGatherOverkill 0
#define returnCountForExpansion 10
#define maxSearchLength 1500

#define antTunnelSize 10
#define antOutPrioity antTunnelSize-2

struct antBasicBrain{
    u_long rnd;
    short baseX ,baseY;
    unsigned char type;
    short x,y;
};

struct antBrain{
    struct antBasicBrain brain;
    short  data[5];
};

struct searcherBrain{
    struct antBasicBrain brain;
    short delay;
    short direction;
    short xDirCount;
    short yDirCount;
    short steps;
};

struct goingHomeBrain{
    struct antBasicBrain brain;
    short state;
    short formerType;
    short messageA;
    short messageB;
    short messageC;
};

struct goToBrain{
    struct antBasicBrain brain;
    short state;
    short xPos;
    short yPos;
};

struct defenderBrain{
    struct antBasicBrain brain;
};

struct rommelBrain{
    struct antBasicBrain brain;
    short searchDir;
    short searchLength;
    short searcherDelay;
    short returnCount;
};

struct baseBuilderBrain{
    struct antBasicBrain brain;
};


int searcherAnt(struct SquareData *fields, struct searcherBrain *ants);



short cosinus(short x);
short sinus(short x);
int moveAnt(int dir,struct SquareData *fields, struct antBasicBrain *brain, int priority);
void setNewBornType(struct SquareData *fields, struct antBrain *ants);
short findType(struct SquareData *fields, struct antBrain *ants, short type);

/*
struct SquareData {
   u_char NumAnts;
   u_char Base;
   u_char Team;
   u_char NumFood;
};
*/



int collectorAction(struct SquareData *fields)
{
    int i;
    for (i=0; i<5; i++){
	if (fields[i].NumFood>0) return i;
    }
    return 0;
}

int defendAction(struct SquareData *fields)
{
    int i;
    int result=0;
    for (i=0; i<5; i++){
	if (fields[i].Team!=0) result = i;
    }
    return result;
}




int goingHomeAnt(struct SquareData *fields, struct goingHomeBrain *ants)
{

    int allowDrag=0;
    //base construct help
    if (fields[here].NumFood>=NewBaseFood) {ants->brain.type=unknown; return here;}


    if (fields[here].NumFood>0 && findType(fields, (struct antBrain*)ants, rommel)==0) allowDrag=dragFood;

    if (ants->brain.x == ants->brain.baseX && ants->brain.y == ants->brain.baseY) {
    
	if (ants->formerType==goTo) ants->brain.type=unknown;
	return here;
    }
    if (ants->brain.x == ants->brain.baseX) ants->state = 0;
    if (ants->brain.y == ants->brain.baseY) ants->state = 1;
    
    
    if (ants->state){
	ants->state = 0;
	if (ants->brain.x<ants->brain.baseX) return moveAnt(right, fields, (struct antBasicBrain*) ants, 0)+allowDrag;
	else return moveAnt(left, fields, (struct antBasicBrain*) ants, 0)+allowDrag;
    }
   if (!ants->state){
	ants->state = 1;
  	if (ants->brain.y<ants->brain.baseY) return moveAnt(down, fields, (struct antBasicBrain*) ants, 0)+allowDrag;
	else return moveAnt(up, fields, (struct antBasicBrain*) ants, 0)+allowDrag;
    }
    return here;
}


int goToAnt(struct SquareData *fields, struct goToBrain *ants)
{

    int food;
    
    //base construct help
    if (fields[here].NumFood>=NewBaseFood) {ants->brain.type=unknown; return here;}


    food = defendAction(fields);
    if (food) {
	ants->brain.type = defender;
	return moveAnt(food, fields, (struct antBasicBrain*) ants, antOutPrioity);
    }

    if (ants->brain.x == ants->xPos && ants->brain.y == ants->yPos){
	if (fields[here].NumFood>0){
	    ants->brain.type = goingHome;
	    ((struct goingHomeBrain*)ants)->formerType = goTo;

	    return goingHomeAnt(fields, (struct goingHomeBrain *)ants);
	} 
	else {
	    ants->brain.type = searcher;
	    ((struct searcherBrain*)ants)->direction = ants->brain.rnd  & 0xff;
	    ((struct searcherBrain*)ants)->xDirCount=0;
	    ((struct searcherBrain*)ants)->yDirCount=0;
	    ((struct searcherBrain*)ants)->steps=20;
	    ((struct searcherBrain*)ants)->delay=0;
	    return searcherAnt(fields, (struct searcherBrain*) ants);
	}
    }
    if (ants->brain.x == ants->xPos) ants->state = 0;
    if (ants->brain.y == ants->yPos) ants->state = 1;
    
    
    if (ants->state){
	ants->state = 0;
	if (ants->brain.x<ants->xPos) return moveAnt(right, fields, (struct antBasicBrain*) ants, antOutPrioity)+dragFood;
	else return moveAnt(left, fields, (struct antBasicBrain*) ants, antOutPrioity);
    }
    if (!ants->state){
	ants->state = 1;
  	if (ants->brain.y<ants->yPos) return moveAnt(down, fields, (struct antBasicBrain*) ants, antOutPrioity)+dragFood;
	else return moveAnt(up, fields, (struct antBasicBrain*) ants, antOutPrioity);
    }
    return here;
}


int searcherAnt(struct SquareData *fields, struct searcherBrain *ants)
{
    int food;
    short dir = ants->direction;
    
    if (ants->delay) {
	ants->delay--;
	return here;
    }
    
    //base construct help
    if (fields[here].NumFood>=NewBaseFood) {ants->brain.type=unknown; return here;}

    food = defendAction(fields);
    if (food) {
	ants->brain.type = defender;
	return moveAnt(food, fields, (struct antBasicBrain*) ants, antOutPrioity);
    }

    food = collectorAction(fields);
    if (food) return moveAnt(food, fields, (struct antBasicBrain*) ants, antOutPrioity);
    
    if (ants->steps==0 || (fields[here].NumFood!=0 && findType(fields, (struct antBrain*)ants, rommel)==0)){
	ants->brain.type = goingHome;
	((struct goingHomeBrain*)ants)->state = 0;
	((struct goingHomeBrain*)ants)->formerType = searcher;
	if (fields[here].NumFood==0){
	    ((struct goingHomeBrain*)ants)->messageA = 0;
	    ((struct goingHomeBrain*)ants)->messageB = 0;
	    ((struct goingHomeBrain*)ants)->messageC = 0;
	} else {
	    ((struct goingHomeBrain*)ants)->messageA = ants->brain.x;
	    ((struct goingHomeBrain*)ants)->messageB = ants->brain.y;
	    ((struct goingHomeBrain*)ants)->messageC = (fields[here].NumFood>>0)-1+foodGatherOverkill;
	}
	return goingHomeAnt(fields, (struct goingHomeBrain*)ants);
    }
    ants->xDirCount += cosinus(dir);
    ants->yDirCount += sinus(dir);

    ants->steps--;

    if (abs(ants->xDirCount)>=127){
	if (ants->xDirCount>0) {
	    ants->xDirCount-=127;
	    return  moveAnt(right, fields, (struct antBasicBrain*) ants, antOutPrioity);
	}
	else {
	    ants->xDirCount+=127;
	    return  moveAnt(left, fields, (struct antBasicBrain*) ants, antOutPrioity);
	}
    }
    if (abs(ants->yDirCount)>=127 && !food){
	if (ants->yDirCount>0) {
	    ants->yDirCount-=127;
	    return  moveAnt(down, fields, (struct antBasicBrain*) ants, antOutPrioity);
	}
	else {
	    ants->yDirCount+=127;
	    return moveAnt(up, fields, (struct antBasicBrain*) ants, antOutPrioity);
	}
    }

    return here;
}


int defenderAnt(struct SquareData *fields, struct defenderBrain *ants)
{
    int where;
    where = defendAction(fields);
    return where;
}



short findType(struct SquareData *fields, struct antBrain *ants, short type)
{
    short i;
    for (i=1; i<fields[here].NumAnts; i++){
	if (ants[i].brain.type == type) return i;
    }
    return 0;
}

short findIdle(struct SquareData *fields, struct antBrain *ants)
{
    short i;
    for (i=1; i<fields[here].NumAnts; i++){
	if (ants[i].brain.type == idle) return i;
    }
    return 0;
}


int rommelAnt (struct SquareData *fields, struct rommelBrain *ants)
{
    short i, j;
    short idleAnt;
    struct antBrain *ant;
    struct antBrain *freeAnt;
    short antCount;
    short reserveAnts = 1;

    //long foodLeft=1;


    //antCount = fields[left].NumAnts + fields[rigth].NumAnts + fields[up].NumAnts + fields[down].NumAnts + fields[here].NumAnts;

    //make goingHome empty handed ants idle
    for (i = 1; i < fields[here].NumAnts; i++) {
	ant = ((struct antBrain *) ants) + i;

	if (ant->brain.type == goingHome &&
	    !(((struct goingHomeBrain *) ant)->formerType == searcher
	      && ((struct goingHomeBrain *) ant)->messageC > 0)) {
	    ant->brain.type = idle;
	    ants->returnCount++;
	}

    }

    if (ants->returnCount > ants->searchLength * ants->searchLength / 200) {
	ants->returnCount = 0;
	ants->searchLength++;
	if (ants->searchLength > maxSearchLength)
	    ants->searchLength--;
    }

    antCount = 0;
    //find an ant with a food message
    for (i = 1; i < fields[here].NumAnts; i++) {
	ant = ((struct antBrain *) ants) + i;
	if (ant->brain.type == goingHome
	    && ((struct goingHomeBrain *) ant)->formerType == searcher) {
	    antCount++;
	    for (j = 0; j < 1 && ((struct goingHomeBrain *) ant)->messageC;
		 j++) {
		idleAnt = findType (fields, (struct antBrain *) ants, idle);
		if (idleAnt) {
		    freeAnt = ((struct antBrain *) ants) + idleAnt;
		    freeAnt->brain.type = goTo;
		    ((struct goToBrain *) freeAnt)->state = 0;
		    ((struct goToBrain *) freeAnt)->xPos = ((struct goingHomeBrain *) ant)->messageA;
		    ((struct goToBrain *) freeAnt)->yPos = ((struct goingHomeBrain *) ant)->messageB;
		    ((struct goingHomeBrain *) ant)->messageC--;
		} else
		    break;
	    }
	    if (antCount > MaxSquareAnts - 20)
		ant->brain.type = idle;

	    //foodLeft += ((struct goingHomeBrain*)ant)->messageC;
	}
    }


    if (fields[here].NumFood >= NewBaseFood)
	reserveAnts = NewBaseAnts;
    else
	reserveAnts = 1;

    //sendout the remaining ants to search for food
    for (i = reserveAnts; i < fields[here].NumAnts; i++) {
	ant = ((struct antBrain *) ants) + i;
	if (ant->brain.type == idle) {
/*	    if (fields[here].Base && (ants->searchLength) > 75 && (ant->brain.rnd & 1)==1){
		ant->brain.baseX += 50;
		((struct goingHomeBrain *) ant)->formerType = idle;
		ant->brain.type = goingHome;
	    } else {
*/
		ant->brain.type = searcher;
		((struct searcherBrain *) ant)->steps = ants->searchLength;
		((struct searcherBrain *) ant)->delay = ants->searcherDelay;
		((struct searcherBrain *) ant)->xDirCount = 0;
		((struct searcherBrain *) ant)->yDirCount = 0;
		((struct searcherBrain *) ant)->direction = ants->searchDir;
		ants->searcherDelay++;
		if (ants->searcherDelay == 1) {
		    ants->searcherDelay = 0;
		    ants->searchDir += 1;
		    ants->searchDir &= 0xff;
		}
//	    }
	}
    }

    return buildBase;

}

int myre(struct SquareData *fields, struct antBrain *ants)
{
    if (ants[0].brain.type==unknown) setNewBornType(fields, ants);

    switch (ants[0].brain.type){
	case rommel:	return rommelAnt(fields, (struct rommelBrain*)ants); break;
	case searcher:  return searcherAnt(fields, (struct searcherBrain*)ants); break;
	case goingHome:	return goingHomeAnt(fields, (struct goingHomeBrain*)ants); break;
	case goTo:	return goToAnt(fields, (struct goToBrain*)ants); break;
	case defender:	return defenderAnt(fields, (struct defenderBrain*)ants); break;
    }

    return here;
}

short cosinus(short x)
{
    return sinus((x+64)&0xff);
}

short sinus(short x)
{
    short data[256]={0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 59, 62, 65, 67, 70, 73, 75, 78, 80, 82, 85, 87, 89, 91, 94, 96, 98, 100, 102, 103, 105, 107, 108, 110, 112, 113, 114, 116, 117, 118, 119, 120, 121, 122, 123, 123, 124, 125, 125, 126, 126, 126, 126, 126, 127, 126, 126, 126, 126, 126, 125, 125, 124, 123, 123, 122, 121, 120, 119, 118, 117, 116, 114, 113, 112, 110, 108, 107, 105, 103, 102, 100, 98, 96, 94, 91, 89, 87, 85, 82, 80, 78, 75, 73, 70, 67, 65, 62, 59, 57, 54, 51, 48, 45, 42, 39, 36, 33, 30, 27, 24, 21, 18, 15, 12, 9, 6, 3, 0, -3, -6, -9, -12, -15, -18, -21, -24, -27, -30, -33, -36, -39, -42, -45, -48, -51, -54, -57, -59, -62, -65, -67, -70, -73, -75, -78, -80, -82, -85, -87, -89, -91, -94, -96, -98, -100, -102, -103, -105, -107, -108, -110, -112, -113, -114, -116, -117, -118, -119, -120, -121, -122, -123, -123, -124, -125, -125, -126, -126, -126, -126, -126, -127, -126, -126, -126, -126, -126, -125, -125, -124, -123, -123, -122, -121, -120, -119, -118, -117, -116, -114, -113, -112, -110, -108, -107, -105, -103, -102, -100, -98, -96, -94, -91, -89, -87, -85, -82, -80, -78, -75, -73, -70, -67, -65, -62, -59, -57, -54, -51, -48, -45, -42, -39, -36, -33, -30, -27, -24, -21, -18, -15, -12, -9, -6, -3};
    return data[x];
}

int moveAnt(int dir,struct SquareData *fields, struct antBasicBrain *brain, int priority)
{
    if (fields[dir].NumAnts>(MaxSquareAnts-antTunnelSize+priority)) return here;
    switch (dir){
	case left: brain->x--; break;
	case right: brain->x++; break;
	case up: brain->y--; break;
	case down: brain->y++; break;
    }
    return dir;
}

void setNewBornType(struct SquareData *fields, struct antBrain *ants)
{
    short i;
    for (i=1; i<fields[here].NumAnts; i++){
	if (ants[i].brain.type == rommel){
	    ants->brain.baseX = ants[i].brain.x;
	    ants->brain.baseY = ants[i].brain.y;
	    ants->brain.x = ants->brain.baseX;
	    ants->brain.y = ants->brain.baseY;
	    ants->brain.type = idle;
	    return;	    
	}
    }  
    ants[0].brain.type = rommel;
    ((struct rommelBrain*)ants)->searchDir = 0;
    ((struct rommelBrain*)ants)->searchLength = 50;
    ((struct rommelBrain*)ants)->searcherDelay = 0;
    ((struct rommelBrain*)ants)->returnCount = 0;
}


DefineAnt(Rommel, "Rommel", myre, struct antBrain)
