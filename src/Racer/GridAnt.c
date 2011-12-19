/* GridAnt, bme@miba.auc.dk */

#include "Myre.h"

#define X mem->xpos
#define Y mem->ypos
#define XT mem->xtarget
#define YT mem->ytarget
#define F mem->food

#define DIST(x1,y1,x2,y2) (GridAntAbs(x1-x2)+GridAntAbs(y1-y2))

enum { UninitialisedAnt=0, PatrolAnt, ReturnFoodAnt, GuardAnt, 
       GoToXTYTAnt, LineGuardAnt, SquarePatrolAnt, ShootAnt };

struct GridAntMemory {
  unsigned long rnd;
  short xpos,ypos;
  short xtarget,ytarget;
  unsigned char food;
  unsigned char state,dir;
};

#define RAND_INT(m,l,h) (((unsigned short)(GridAntRandom(m) * ((float)(h)-(float)(l)+1))) + (l))

float GridAntRandom (struct GridAntMemory *mem) {
/* The following parameters are recommended settings based on research
   uncomment the one you want. */

  static unsigned int a = 1588635695, m = 4294967291U, q = 2, r = 1117695901;
/* static unsigned int a = 1223106847, m = 4294967291U, q = 3, r = 625646750;*/
/* static unsigned int a = 279470273, m = 4294967291U, q = 15, r = 102913196;*/
/* static unsigned int a = 1583458089, m = 2147483647, q = 1, r = 564025558; */
/* static unsigned int a = 784588716, m = 2147483647, q = 2, r = 578306215;  */
/* static unsigned int a = 16807, m = 2147483647, q = 127773, r = 2836;      */
/* static unsigned int a = 950706376, m = 2147483647, q = 2, r = 246070895;  */

/*   SEED = a*(SEED % q) - r*(SEED / q);*/
   mem->rnd=a*(mem->rnd % q) - r*(mem->rnd / q);
   return ((double)mem->rnd / (double)m);
}

inline int GridAntAbs(int a) {
  if (a>=0)
    return a;
  return -a;
}

inline char GridAntFuzzyWalk(struct GridAntMemory *mem,short x1, short y1, short x2, short y2) {
  if (RAND_INT(mem,-GridAntAbs(x1-x2),GridAntAbs(y1-y2))<0) {
    if (x2>x1) {
      return 1;
    } else if (x1>x2) {
      return 3;
    } else if (y2>y1) {
      return 4;
    } else if (y1>y2) {
      return 2;
    } else {
      return 0;
    }
  } else {
    if (y2>y1) {
      return 4;
    } else if (y1>y2) {
      return 2;
    } else if (x2>x1) {
      return 1;
    } else if (x1>x2) {
      return 3;
    } else {
      return 0;
    }
  }
}

// fields:   NumAnts, Base, Team, NumFood
// egen mem: mem
// andre:    mem[1], mem[2], ..., mem[fields->NumAnts-1]
// return:   0: stay, 1: East, 2: South, 3: West, 4: North
int GridAntBrain(struct SquareData *fields, struct GridAntMemory *mem) {
  int i,max,d,dir;
  const int Xd[]={0, 1,  0, -1, 0};
  const int Yd[]={0, 0, -1,  0, 1};

  dir=0;
  switch(mem->state) {
  case UninitialisedAnt:
    mem->dir=(mem->rnd&3)+1;
    mem->state=PatrolAnt;
    YT=0;
    XT=-((mem->rnd>>7)&1)*42;
    if (RAND_INT(mem,1,4)==1) {
      XT=0;
      F=0;
      //mem->state=LineGuardAnt;
      mem->state=ShootAnt;
    }
    break;
  case PatrolAnt:
    if (fields->NumFood>0 && (X!=0 || Y!=0)) { // found food, but am I necessary?
      max=0;
      for(i=1;i<fields->NumAnts;i++) {
	if(mem[i].state==ReturnFoodAnt) {
	  max++;
	}
      }
      // if more food than ants then help
      if (fields->NumFood>max) {
	XT=X;YT=Y;
	mem->state=ReturnFoodAnt;
	mem->food=fields->NumFood-max;
      }
    } else {
      if (mem->state==PatrolAnt) { // no food located
	if (XT==0 && YT==0) {
	  mem->dir=((mem->rnd>>27)&3)+1;
	}
	if (XT>=YT) {
	  XT=0;
	  if (YT<=3) { YT+=3; } else { YT+=6; }
	  if (mem->rnd&4) {
	    mem->dir++;
	    if (mem->dir>4) mem->dir=1;
	  } else {
	    mem->dir--;
	    if (mem->dir<1) mem->dir=4;
	  }
	}
	XT++;
	dir=mem->dir;
      }
      
      if (X!=0 || Y!=0)
	for(i=1;i<5;i++) // food around me?
	  if(fields[i].NumFood>0 && fields[i].NumAnts<fields[i].NumFood)
	    if (X+Xd[i]!=0 || Y+Yd[i]!=0)
	      dir=i;
    }
    break;
  case ReturnFoodAnt:
    //if (fields->NumFood>0 && (X!=0 || Y!=0)) {
    if (X==0 && Y==0) {
      mem->state=GoToXTYTAnt;
    } else {
      // how many are fetching food right here?
      max=1;
      for(i=1;i<fields->NumAnts;i++)
	if(mem[i].state==ReturnFoodAnt) {
	  max++;
	}
      // too many?
      if (max>fields->NumFood) {
	mem->state=PatrolAnt;
	XT=0;YT=0;F=0;
      } else {
	dir=GridAntFuzzyWalk(mem,X,Y,0,0)+8;
	for(i=1;i<fields->NumAnts;i++) { // tell other where food is
	  if ((mem[i].state==PatrolAnt || mem[i].state==UninitialisedAnt || mem[i].state==GoToXTYTAnt) && mem->food>2) {
	    mem[i].state=GoToXTYTAnt;
	    mem[i].xtarget=XT;
	    mem[i].ytarget=YT;
	    mem[i].food=mem->food-3;
	    mem->food-=3;
	  }
	}
      }
    }
    break;
  case GoToXTYTAnt:
    if (X!=XT || Y!=YT) {
      dir=GridAntFuzzyWalk(mem,X,Y,XT,YT);
      if (X!=0 || Y!=0) {
	for(i=0;i<5;i++) // food around me?
	  if(fields[i].NumFood>0 && fields[i].NumAnts<fields[i].NumFood)
	    if (X+Xd[i]!=0 || Y+Yd[i]!=0) {
	      dir=i;
	      mem->state=ReturnFoodAnt;
	      XT=X+Xd[i];YT=Y+Yd[i];F=fields[i].NumFood;
	    }
      }
    } else {
      mem->state=UninitialisedAnt;
    }
    break;
  case GuardAnt:
    mem->xtarget--; // counter...
    if ((mem->xtarget%20)==0) {
      dir=GridAntFuzzyWalk(mem,X,Y,X*2,Y*2); // walk outwards slowly
    }
    if (mem->xtarget==0) {
      mem->state=PatrolAnt;
      XT=0;
      YT=0;
      dir=0;
    } else if (X!=0 || Y!=0) {
      for(i=0;i<5;i++) // food around me?
        if(fields[i].NumFood>0 && fields[i].NumAnts<fields[i].NumFood)
          if (X+Xd[i]!=0 || Y+Yd[i]!=0) {
            dir=i;
            mem->state=ReturnFoodAnt;
	    XT=X+Xd[i];YT=Y+Yd[i];F=fields[i].NumFood;
          }
    }
    break;
  case LineGuardAnt:
    if (X==0 && Y==0) { // new and fresh
      F=XT=RAND_INT(mem,1,4); // random direction
      dir=F;
      YT=0;
    } else if ((X%24)==0 && (Y%24)==0) {
      if (fields->NumAnts==1) {
	if (YT>=20) {
	  dir=1;
	  mem->state=SquarePatrolAnt;
	  YT=0;
	  F=1;
	} else {
	  YT++;
	}
      } else {
	F=RAND_INT(mem,1,3);
	if (F>=XT) F++;
	dir=F;
      }
    } else {
      dir=F;
    }
    break;
  case SquarePatrolAnt:
    if (X%24==0 && Y%24==0) {
      mem->state=LineGuardAnt;
    } else if (X%12==0 && Y%12==0) {
      if (X!=0 || Y!=0)
	for(i=0;i<5;i++) // food around me?
	  if(fields[i].NumFood>0 && fields[i].NumAnts<fields[i].NumFood)
	    if (X+Xd[i]!=0 || Y+Yd[i]!=0) {
	      dir=i;
	      mem->state=ReturnFoodAnt;
	      XT=X+Xd[i];YT=Y+Yd[i];F=fields[i].NumFood;
	    }
      F++;
      dir=F;
    } else {
      dir=F;
    }
    break;
  case ShootAnt:
#define SL 999
    i=RAND_INT(mem,0,SL*4-1);
    XT=-(SL-1)/2+i;
    YT=(SL+1)/2;
    if (XT>(SL-1)/2) { i-=SL; XT=YT; YT=(SL-1)/2-i; }
    if (YT<-(SL-1)/2) { i-=SL; YT=-XT; XT=(SL-1)/2-i; }
    if (XT<-(SL-1)/2) { i-=SL; XT=YT; YT=-(SL-1)/2+i; }
    mem->state=GoToXTYTAnt;
    XT+=X;
    YT+=Y;
    break;
  }

  // create guards were needed (base ant excluded)
  for(i=1;i<5;i++)
    if(fields[i].Team>0) {
      dir=i;
      mem->state=GuardAnt;
      mem->xtarget=1500; // guard for 1500 turns
    }

  // keep track of where we are (can fail and then ant is lost)
  //   check for overflow and backtrack variables
  d=DIST(X,Y,0,0);
  if ((fields[dir&7].NumAnts>=MaxSquareAnts-d*5 && d<4) ||
      (fields[dir&7].NumAnts>=MaxSquareAnts)) {
    return 0; // wait
  }
  X+=Xd[dir&7];
  Y+=Yd[dir&7];
  return dir;
}

DefineAnt(GridAnt,"GridAnt#ff8000",GridAntBrain,struct GridAntMemory);
