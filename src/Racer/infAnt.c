
/***********************
 *    infAnt v1.0      *
 *     (C) 2002        *
 *  Lars Ole Simonsen  *
 *   lo@daimi.au.dk    *
 ***********************/

#include "Myre.h"
#define abs(a) (a<0 ? -a : a)
#define SETTLERS_PR_BASE 250 //skal helst være mindre end ANTS_BEFORE_BASE
#define ANTS_BEFORE_BASE 800//antal myrer der skal være produceret før der kan udvides
#define PROD_SPEED_FOR_BASE 1.0//hvor lille (tid/myre) skal være før der bygges base. For at undgå at der bygges base, når der er meget lidt mad
#define ROUNDS_BEFORE_BASE -1 //bruges ikke for tiden
#define SEARCH_RADIUS 250//start radius
#define START_QUEEN_SETTLE 1// hvis 1 så starter start-basen nye baser
#define SETTLE_QUEEN_SETTLE 1 //hvis 1 så starter spawnede baser nye baser
#define BASE_FORMATION 2 //2 for 2 new bases, 1 for 4 new bases
#define BASE_DISTANCE 110//afstand mellem baserne
#define INC_RADIUS_INTERVAL 300 //antal myrer der laves mellem hver forøgelse
#define RADIUS_INC 17//radius øges med 17 pr. inc
#define GUARD_TIME 1000//hvor længe skal guards blive stående
#define PERCENTAGE_SIDESTEP 15//povirker søgemønsteret. 
#define exploreDir 1+((mem->dir + 2)&3)

typedef enum {UNINITIALIZED=0, QUEEN, EXPLORER, CARRIER, GUARD,SETTLER} Job; 

typedef struct {
  //long random;
  unsigned char random : 7;
  unsigned char lastDir : 1;
  unsigned char dir : 2;
  unsigned char side : 2;
  unsigned char baseDir : 2;
  short locx:16;
  short locy:16;
  short foodx:16;
  short foody:16;
  unsigned short radius : 9;
  unsigned char job: 3;
 } LOBrain;
// QUEEN bruger foodx til at tælle de myrer hun har lavet 
// siden sidste nye base blev startet
// og foody til at tælle antal runder hun har været dronning
// og locx og -y til at huske hvor hun er ved at sætte en ny base op
// og lastDir til at huske at hun er ved at sætte base op
// og side til at huske om hun overhovedet har lov at lave baser
// GUARD bruger foodx til at tælle antal runder han mangler at guarde
// EXPLORER bruger foodx og -y til at huske hvor maden er
// CARRIER ditto
// SETTLER bruger foodx og -y til at huske hvor den nye base skal være
char LO2_updateAnt(char,LOBrain*);
char LO2_goto(short,short,LOBrain*);

//genererer et "tilfældigt" tal mellem 0 og 128 
unsigned char LO2_getRand(LOBrain *mem){
  mem->random = mem->random*245 + 123;
  return (((mem->random)*4213+421) & 255);
}

int LO2_Func(struct SquareData *sqr, LOBrain *mem)
{
  
  int i = 1;
  LOBrain* queen = 0;
  unsigned char dir = 1;
  if(mem->job == UNINITIALIZED) {
    //hvad skal myren så lave?
    //skal den vaere dronning?
    mem->locx = mem->foodx = 0;
    if(sqr->NumAnts > 1) {
      queen = &mem[1];
      if(queen->job != QUEEN) {
	queen->job = QUEEN;
	queen->side = START_QUEEN_SETTLE;
	queen->foodx = 1;
	queen->foody = queen->lastDir = 0;
	queen->dir = queen->random;
	queen->radius = SEARCH_RADIUS;
      }
    } else {
      queen = mem;
      queen->side = START_QUEEN_SETTLE;
      queen->job = QUEEN;
      queen->foodx = 1;
      queen->foody = queen->lastDir = 0;
      queen->dir = queen->random;
      queen->radius = SEARCH_RADIUS;
    }
    //nu peger queen på basens dronning
    if(queen->foodx++ % INC_RADIUS_INTERVAL == 0) {
      queen->radius+=RADIUS_INC;
    }

    //////kode til at finde ud af om der skal laves ny base og hvor/////
    if(queen && queen->side && queen->foodx % ANTS_BEFORE_BASE == 0&&
       queen->foody/queen->foodx < PROD_SPEED_FOR_BASE) {
      //queen sættes i 'lav-ny-base-mode'
      queen->lastDir = 1;
      queen->dir +=BASE_FORMATION;
      //dir +=2 betyder dronningen vender 180 inden næste base placeres
      //dir +=1 ditto 90 grader
      queen->foodx = queen->foody = 1;
      mem->job = SETTLER;
      mem->baseDir = queen->dir;
      switch(queen->dir) {
      case 0:
	mem->foodx = queen->locx = BASE_DISTANCE;
	mem->foody = queen->locy = 0;
	break;
      case 1:
	mem->foodx = queen->locx = 0;
	mem->foody = queen->locy = -BASE_DISTANCE;
	break;
      case 2:
	mem->foodx = queen->locx = -BASE_DISTANCE;
	mem->foody = queen->locy = 0;
	break;
      case 3:
	mem->foodx = queen->locx = 0;
	mem->foody = queen->locy = BASE_DISTANCE;
      }
      return queen->dir + 1;
      //her angiver foodx antallet af myrer sendt til den nye base
    } else if(queen && queen->side && queen->lastDir && 
	      queen->foodx <= SETTLERS_PR_BASE) {
      mem->job = SETTLER;
      mem->baseDir = queen->dir;
      mem->foodx = queen->locx;
      mem->foody = queen->locy;
      if(queen->foodx == SETTLERS_PR_BASE) {
	//tag queen ud af 'lav-ny-base-mode'
	queen->lastDir = 0;
	queen->foodx = queen->foody = 1;
      } else
	queen->foodx++;
      return queen->dir + 1;
    }
    /////////slut på ny-base-koden/////////////////

    //kan vist ikke ske, men hvad..
    if(!sqr->Base && sqr->NumAnts < NewBaseAnts) {
      mem->job = SETTLER;
    } else {
      mem->job = EXPLORER;
      mem->radius = queen->radius;
    }     
  }
  for(; i < 5; i++) {
    //kig efter fjender og tramp på en!
    if(sqr[i].Team > 0) {
      mem->job = GUARD;
      mem->foodx = GUARD_TIME;
      return LO2_updateAnt(i,mem);
    }
  }
  if(mem->job == GUARD && mem->foodx == 0) {
      mem->job = CARRIER;
      mem->foody = 0;
  }

  switch(mem->job) {
    //////////////////////EXPLORER//////////////////// 
 case EXPLORER:
    i = 1;
    dir = 1;
    //hvis vi går ind i en anden base, saa bliver den vores hjembase
    if(sqr->Base || (sqr->NumAnts > 1 && mem[1].job == QUEEN)) {
      mem->foodx -=mem->locx;
      mem->foody -=mem->locy;
      mem->locx = mem->locy = 0;
    }

    //kig efter mad på de synlige felter
    if(abs(mem->locx)+abs(mem->locy) > 1) {
      if(sqr->NumFood >= sqr->NumAnts) {
	mem->job = CARRIER;
	mem->foodx = mem->locx;
	mem->foody = mem->locy;
	return LO2_updateAnt(mem->dir+9,mem);
      }  else 
	for(; dir < 5; dir++)
	  if(sqr[dir].NumFood > sqr[dir].NumAnts) {
	    return LO2_updateAnt(dir,mem);
	  }
    }
    //spørg andre myrer hvor der er mad (med visse undtagelser)
    for(; i < sqr->NumAnts; i++) {
      if(mem->radius < mem[i].radius)
	mem->radius = mem[i].radius;
      if(!(mem->foodx) && !(mem->foody)
	 && mem[i].job != GUARD
	 && mem[i].job != QUEEN
	 && (abs(mem[i].foodx)+abs(mem[i].foody) > 1)
	 && (mem[i].foodx!=0 || mem[i].foody!=0)) {
	mem->foodx = mem[i].foodx;
	mem->foody = mem[i].foody;
	
	break;
      }
    }
    if(abs(mem->locx)+abs(mem->locy) == BASE_DISTANCE)
      if(sqr->Base) {
	mem->locx = mem->locy = 0;
	mem->job = EXPLORER;
	return LO2_updateAnt(exploreDir,mem);
      }
    //ved vi hvor der er mad?
    if(mem->foodx != 0 || mem->foody != 0) {
      //hvis vi ikke er kommet frem til madstedet så går vi et skridt nærmere
      if(mem->foodx != mem->locx || mem->foody != mem->locy) {
	return LO2_goto(mem->foodx,mem->foody,mem);
      }
      else {
 	mem->foodx = mem->foody = 0;
      }
    }
    //myren ved ikke hvor der er mad

    //hvis myren er kommet for langt hjemmefra, så  vendes den 180 
    //grader og sendes et skridt hjemad
    if(abs(mem->locx)+abs(mem->locy) >= mem->radius ) {
      mem->dir+=2;
      mem->side+=1;
      return LO2_goto(0,0,mem);
    } else {
      ///////////////søgemønster///////////////
      if(LO2_getRand(mem)%101 <= PERCENTAGE_SIDESTEP*mem->side)
	return LO2_updateAnt(1 + ((mem->dir+(mem->side?1:3))&3),mem);
      else 
	return LO2_updateAnt(1 + mem->dir,mem);
    }
    //////////////søgemønster slut :-)/////////

    /////////////CARRIER//////////////////////
  case CARRIER:
    if(sqr->Base) {
      //hvis vi kommer til en base, så gør vi den til vores hjembase
      //uanset om den var det i forvejen
      queen = &mem[1];
      mem->radius = queen->radius;
      mem->foodx -= mem->locx;
      mem->foody -= mem->locy;
      mem->locx = mem->locy = 0;
      mem->job = EXPLORER;
      return LO2_updateAnt(exploreDir,mem);
    } else if(mem->locx == 0 && mem->locy == 0) {
      mem->job = SETTLER;
      return mem->foodx = mem->foody = 0;
    } else 
      return LO2_goto(0,0,mem);
    
  case GUARD:
    mem->foodx --;
    return 0;
    ///////////////////////SETTLER////////////////////////////
  case SETTLER:
    // Are we there yet?
    if(mem->foodx == mem->locx && mem->foody == mem->locy) {
      mem->foodx = mem->foody = mem->locx = mem->locy = 0;
      if(sqr->Base) {
	mem->job = EXPLORER;
	return LO2_updateAnt(exploreDir,mem);
      } else {
	//skal lige sikres at der er en dronning
	if(sqr->NumAnts == 1) {
	  mem->job = QUEEN;
	  mem->foodx = 1;
	  mem->side = SETTLE_QUEEN_SETTLE;
	  mem->foody = mem->lastDir = 0;
	  mem->radius = SEARCH_RADIUS;
	  mem->dir = mem->baseDir+1;
	  return 16;
	  //nok ikke sandsynligt at en base bliver bygget her, men hvad..
	} else if(mem[1].job!=QUEEN) {
	  mem[1].job = QUEEN;
	  mem[1].side = SETTLE_QUEEN_SETTLE;
	  mem[1].foodx = 1;
	  mem[1].foody = mem[1].lastDir = 0;
	  mem[1].radius = SEARCH_RADIUS;
	  mem[1].dir = mem[1].baseDir+1;
	}
	if(sqr->NumAnts <= NewBaseAnts) {
	  return 0;
	} else {
	  mem->job = EXPLORER;
	  return LO2_updateAnt(exploreDir,mem);
	}
      }
    } else {
      return LO2_goto(mem->foodx,mem->foody,mem);
    }

  case QUEEN:
    mem->foody++;
    return 16;
  }
  return 0;
}

char LO2_goto(short x,short y, LOBrain* mem)
{
  char i = 0;
  x -= mem->locx;
  y -= mem->locy;
  //vi tillader settlers at stjæle mad på vejen ud hvis de kan
  if(mem->job == CARRIER || mem->job == SETTLER) {
      if(y == 0)
	i = x < 0 ? 11:9;
      else if(x == 0)
	  i = y<0?10:12;
      else if(mem->lastDir++)
	  i = y<0?10:12;
	else
	  i = x<0?11:9;
    } else {
      if(y == 0)
	i = x < 0 ? 3:1;
      else if(x == 0)
	  i = y<0?2:4;
      else if(mem->lastDir++)
	  i = y<0?2:4;
	else
	  i = x<0?3:1;
    }
  switch(i) {
  case 1 :
  case 9 :
    mem->locx +=1;
    break;
  case 2 :
  case 10 :
    mem->locy -=1;
    break;
  case 3 :
  case 11 :
    mem->locx -=1;
    break;
  case 4 :
  case 12 :
    mem->locy +=1;
  }
  return i;
}

char LO2_updateAnt(char action, LOBrain* mem)
{
  if(!(action%8 == 0)) {
    switch(action) {
    case 1 :
    case 9 :
      mem->locx +=1;
      break;
    case 2 :
    case 10 :
      mem->locy -=1;
      break;
    case 3 :
    case 11 :
      mem->locx -=1;
      break;
    case 4 :
    case 12 :
      mem->locy +=1;
    }
  }
  return action;
}

DefineAnt(infAnt,"infAnt#9053a2",LO2_Func,LOBrain) 
