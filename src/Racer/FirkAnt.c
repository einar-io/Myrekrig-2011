/* FirkAnt, bme@miba.auc.dk */
#include "Myre.h"

#define BASERANGE 96

#define X mem->n.xpos
#define Y mem->n.ypos
#define XT mem->n.xtarget
#define YT mem->n.ytarget
//#define F mem->n.food
#define W mem->b.wait1
#define FD mem->b.fooddistance
#define PD mem->b.patroldistance

#define MIN(a,b) (a)<(b)?(a):(b)
#define DIST(x1,y1,x2,y2) (FirkAntAbs(x1-x2)+FirkAntAbs(y1-y2))
#define ROT(d) ((((d&7)%4)+1)|(d&24))

#define SETFOOD(mem,value) tmp=value;(mem)->n.food=(tmp>15?15:(tmp<0?0:tmp));
#define GETFOOD(mem) ((mem)->n.food)

#define SETCOUNT(mem,value) tmp=value;(mem)->b.count1=(tmp)&255;(mem)->b.count2=((tmp)>>8)&255;
#define GETCOUNT(mem) ((mem)->b.count1|((mem)->b.count2<<8))

enum { UninitialisedAnt=0, SearchAndDestroyAnt, ReturnFoodAnt, GuardAnt, 
       BaseAnt, NewBaseAnt, PatrolAnt, PreventBaseAnt };

union FirkAntMemory {
  struct {
    // 1. short
    char xpos;
    char ypos;
    // 2. short
    char xtarget;
    char ytarget;
    // 5. byte
    unsigned char food:4;
    unsigned char dir:1;
    unsigned char state:3;
  } n;
  struct {
    unsigned char fooddistance,patroldistance;
    unsigned char count1,count2;
    unsigned char wait1:4;
    unsigned char firstbase:1;
    unsigned char state:3;
  } b;  //BaseAnt
  struct {
    unsigned char r1,r2,r3,r4;
    unsigned char r5;
  } r;
};

#define RAND_INT(m,f,l,h) (((signed int)(FirkAntRandom(m,f) * ((float)(h)-(float)(l)+1))) + (l))
/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */

inline float FirkAntRandom (union FirkAntMemory *mem, struct SquareData *fields) {
  unsigned int r;
  r=((mem->r.r4<<24)|(mem->r.r3<<16)|(mem->r.r2<<8)|mem->r.r1)^
    (mem->r.r5)^
    //r=(*((unsigned int *)mem))^   // (r2<<16)|r1
    //(*((unsigned short *)mem+2))^ // r3
    (((fields->NumAnts*76531)^(fields->NumFood*1001)));
  r=1588635695*(r % 2) - 1117695901*(r / 2);
  return (float)(r)/(float)4294967291U;
}

inline int FirkAntAbs(int a) {
  if (a>=0)
    return a;
  return -a;
}

inline int FirkAntMax(int a, int b) {
  if (a>b)
    return a;
  return b;
}

short FirkAntSqrt(int n) {
  int r,t,tmp;

  if (n<65536)
    r=0x100; // 8 loops
  else 
    r=0x10000; // 16 loops

  t=0;
  while (r>0) {
    tmp=t|(r>>=1);
    if (tmp*tmp<=n)
      t=tmp;
  }
  return t;
}

#define RANGE (BASERANGE+25)
void FirkAntSwitchBase(union FirkAntMemory *mem) {
  short x,y,nx,ny,d;
  x=XT;
  y=YT;
  d=FirkAntMax(FirkAntAbs(x),FirkAntAbs(y));
  if (d>RANGE) {
    nx=0;ny=0;
    if (x>0 && x>=FirkAntAbs(y)) { nx=BASERANGE; ny=0; }
    if (x<0 && x<=-FirkAntAbs(y)) { nx=-BASERANGE; ny=0; }
    if (y>0 && y>FirkAntAbs(x)) { nx=0; ny=BASERANGE; }
    if (y<0 && y<-FirkAntAbs(x)) { nx=0; ny=-BASERANGE; }
    X-=nx;
    Y-=ny;
    XT-=nx;
    YT-=ny;
  }
}

inline char FirkAntWalk(short x1, short y1, short x2, short y2) {
  if (DIST(x1,y1,x2,y2)==0)
    return 0;
  if (FirkAntAbs(x1-x2)>FirkAntAbs(y1-y2)) {
    if (x2>x1)
      return 1;
    else 
      return 3;
  } else if (y2>y1) {
    return 4;
  } else {
    return 2;
  }
}

char FirkAntCrossWalk(short x1, short y1, short x2, short y2) {
  if (FirkAntAbs(x2-x1)>FirkAntAbs(y2-y1)) {
    if (y2>y1)
      return 4;
    else if (y2<y1)
      return 2;
    else if (x2>x1)
      return 1;
    else if (x2<x1)
      return 3;
    else
      return 0;
  } else {
    if (x2>x1)
      return 1;
    else if (x2<x1)
      return 3;
    else if (y2>y1)
      return 4;
    else if (y2<y1)
      return 2;
    else
      return 0;
  }
}

inline char FirkAntFuzzyWalk(union FirkAntMemory *mem, struct SquareData *fields,short x1, short y1, short x2, short y2) {
  if (RAND_INT(mem,fields,-FirkAntAbs(x1-x2),FirkAntAbs(y1-y2))<0) {
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
int FirkAntBrain(struct SquareData *fields, union FirkAntMemory *mem) {
  const int Xd[]={0, 1,  0, -1, 0};
  const int Yd[]={0, 0, -1,  0, 1};

  int i,max,d,dir=0,x,y,tmp,prev_state=-1; // tmp used by SETXT

  while(mem->n.state!=prev_state) {
  prev_state=mem->n.state;

  dir=0;
  switch(mem->n.state) {
  case UninitialisedAnt:
    mem->n.xpos=0; // part of the initial random long, used for something else now.
    mem->n.ypos=0;
    mem->n.xtarget=0;
    mem->n.ytarget=0;
    SETFOOD(mem,0);
    mem->n.dir=0;
    max=0;
    for(i=1;i<fields->NumAnts;i++)
      if (mem[i].n.state!=UninitialisedAnt)
	max++;
    for(i=1;i<5;i++)
      if(fields[i].NumAnts>0)
	max++;
    if (max==0) {
      mem->n.state=BaseAnt; // base ant elected
      FD=0;W=0;
      SETCOUNT(mem,0);
      mem->b.firstbase=1;
    }
    break;
  case SearchAndDestroyAnt:
    if (fields->NumFood>0 && (X!=0 || Y!=0)) { // found food, but am I necessary?
      max=0;
      d=DIST(X,Y,0,0);
      for(i=1;i<fields->NumAnts;i++) {
	switch(mem[i].n.state) {
	case ReturnFoodAnt:
	  max++;
	  break;
	case BaseAnt: // pick new home
	  XT-=X;
	  YT-=Y;
	  X=0;
	  Y=0;
	  max=200;i=fields->NumAnts;// early termination
	  break;
	case SearchAndDestroyAnt:
	case PatrolAnt:
	case GuardAnt:
	  if (DIST(mem[i].n.xpos,mem[i].n.ypos,0,0)<d) {
	    d=DIST(mem[i].n.xpos,mem[i].n.ypos,0,0);
	    // fix target
	    XT=XT-(X-mem[i].n.xpos);
	    YT=YT-(Y-mem[i].n.ypos);
	    // fix home
	    X=mem[i].n.xpos;
	    Y=mem[i].n.ypos;
	  }
	  break;
	}
      }
      // if more food than ants then help
      if (fields->NumFood>max) {
	XT=X;
	YT=Y;
	mem->n.state=ReturnFoodAnt;
	SETFOOD(mem,(fields->NumFood-max)/3);
	FirkAntSwitchBase(mem);
      }
    } else {
      if (X==XT && Y==YT) { // search along another path
	if (X==0 && Y==0) {
	  XT=RAND_INT(mem,fields,-16,16);
	  YT=RAND_INT(mem,fields,-16,16);
	  mem->n.dir=RAND_INT(mem,fields,0,1);
	} else {
	  if (GETFOOD(mem)>0) { // there was supposed to be food, but its all gone
	    x=RAND_INT(mem,fields,XT,XT*1.10);
	    y=RAND_INT(mem,fields,YT,YT*1.10);
	    SETFOOD(mem,0);
	  } else if (mem->n.dir) {
	    x=X+Y; // rotate 45 degrees clockwise
	    y=Y-X;
	  } else {
	    x=X-Y; // rotate 45 degrees counterclockwise
	    y=Y+X;
	  }
	  i=FirkAntSqrt(X*X+Y*Y); // not exactly a cheap operation
	  x=(x*10*(i+20))/(14*i); // +20,/14
	  y=(y*10*(i+20))/(14*i);
	  if (x>126) x=126;
	  if (x<-126) x=-126;
	  if (y>126) y=126;
	  if (y<-126) y=-126;
	  XT=x;YT=y;
	  mem->n.dir=RAND_INT(mem,fields,0,1);
	}
      } else {
	dir=FirkAntFuzzyWalk(mem,fields,X,Y,XT,YT);
	if (X!=0 || Y!=0)
	  for(i=1;i<5;i++) // food around me?
	    if(fields[i].NumFood>0 && fields[i].NumAnts<fields[i].NumFood)
	      if (X+Xd[i]!=0 && Y+Yd[i]!=0)
		dir=i;  // take it
      }
    }
    break;
  case ReturnFoodAnt:
    // check ants to see which home is closer
    for(i=1;i<fields->NumAnts;i++)
      switch(mem[i].n.state) {
      case BaseAnt:
	XT-=X;
	YT-=Y;
	X=0;
	Y=0;
	break;
      case UninitialisedAnt:
	break;
      case PatrolAnt:
	//break;
      default:
	if (DIST(mem[i].n.xpos,mem[i].n.ypos,0,0)<DIST(X,Y,0,0)) {
	  // fix target
	  XT=XT-(X-mem[i].n.xpos);
	  YT=YT-(Y-mem[i].n.ypos);
	  // fix home
	  X=mem[i].n.xpos;
	  Y=mem[i].n.ypos;
	}
	break;
      }
    if (fields->NumFood>0 && (X!=0 || Y!=0)) {
      // how many are fetching food right here?
      d=DIST(XT,YT,0,0);
      max=0;
      for(i=0;i<fields->NumAnts;i++)
	if(mem[i].n.state==ReturnFoodAnt) {
	  max++;
	  if (d>DIST(mem[i].n.xtarget,mem[i].n.ytarget,0,0) && GETFOOD(&mem[i])>1) {
	    XT=mem[i].n.xtarget+(X-mem[i].n.xpos); // pick closer food target
	    YT=mem[i].n.ytarget+(Y-mem[i].n.ypos);
	    d=DIST(XT,YT,0,0);
	    SETFOOD(mem,GETFOOD(&mem[i])-1);
	  }
	}
      // more ants than food?
      if (max>fields->NumFood) {
	mem->n.state=SearchAndDestroyAnt;
      } else {
	dir=FirkAntCrossWalk(X,Y,0,0)+8;
	for(i=1;i<fields->NumAnts;i++) { // tell other where food is
	  if (((mem[i].n.state==SearchAndDestroyAnt)
	       || (mem[i].n.state==PatrolAnt && mem[i].n.ytarget!=3))
	      && GETFOOD(mem)>1 ) {
	    mem[i].n.xtarget=XT-(X-mem[i].n.xpos);
	    mem[i].n.ytarget=YT-(Y-mem[i].n.ypos);
	    mem[i].n.state=SearchAndDestroyAnt;
	    SETFOOD(&mem[i],GETFOOD(mem)-1);
	    SETFOOD(mem,GETFOOD(mem)-1);
	  }
	}
      }
    } else {
      mem->n.state=SearchAndDestroyAnt;
    }
    break;
  case GuardAnt: // guard forever
    SETCOUNT(mem,GETCOUNT(mem)-1); // counter
    if (GETCOUNT(mem)==0) { // timeout, continue searching
      mem->n.state=SearchAndDestroyAnt;
      XT=X;
      YT=Y;
    } else if (X!=0 || Y!=0) { // am i anywhere but home?
      for(i=0;i<5;i++) // food around me?
        if(fields[i].NumFood>0)
	  if (fields[i].NumAnts<fields[i].NumFood)
	    if (X+Xd[i]!=0 || Y+Yd[i]!=0) {
	      dir=i;
	      mem->n.state=ReturnFoodAnt;
	      XT=X+Xd[i];YT=Y+Yd[i];
	      SETFOOD(mem,(fields[i].NumFood/3));
	    }
    } else { // Guard at home, very unlikely
      mem->n.state=SearchAndDestroyAnt;
      XT=X;
      YT=Y;
    }
    
    break;
  case BaseAnt: // base ant, never moves, specielt use of X,Y,XT,YT
    // count base ants, one is enough
    d=0;
    for(i=1;i<fields->NumAnts;i++)
      if (mem[i].n.state==BaseAnt)
	d++;
    if (d>0) { // someone else is base ant
      mem->n.state=UninitialisedAnt;
      X=0;Y=0;XT=0;YT=0;SETFOOD(mem,0);mem->n.dir=0;
    } else {
      if ((fields->Base==1) || 
	  (fields->Base==0 && fields->NumFood<NewBaseFood)) {
	for(i=1;i<fields->NumAnts;i++) {
	  switch(mem[i].n.state) {
	  case ReturnFoodAnt:
	    x=mem[i].n.xtarget;
	    y=mem[i].n.ytarget;
	    d=FirkAntSqrt(x*x+y*y);
	    FD=(FD*20+d)/21;
	    break;
	  case PatrolAnt:
	    x=mem[i].n.xtarget;
	    if (x>PD) PD=x;
	    break;
	  case UninitialisedAnt:
	    mem[i].n.xpos=0;
	    mem[i].n.ypos=0;
	    //max=(FD<(BASERANGE-10)?(int)((BASERANGE)/3):((int)(FD/3)));
	    max=126/3;
	    //if (max>126/3) max=126/3;
	    mem[i].n.xtarget=((GETCOUNT(mem)>>3)%max)*3+3;
	    mem[i].n.ytarget=0;
	    SETFOOD(&mem[i],((GETCOUNT(mem)&6)>>1)+1);
	    mem[i].n.dir=GETCOUNT(mem)&1;
	    mem[i].n.state=PatrolAnt;
	    SETCOUNT(mem,GETCOUNT(mem)+1);
	    break;
	  default:
	    break;
	  }
	}
      } else {
	for(i=1;i<fields->NumAnts;i++)
	  if (mem[i].n.state!=PreventBaseAnt) {
	    mem[i].n.state=NewBaseAnt;
	    mem[i].n.xpos=0;
	    mem[i].n.ypos=0;
	    mem[i].n.xtarget=0;
	    mem[i].n.ytarget=0;
	    SETFOOD(&mem[i],0);
	  }
      }
    }
    break;
  case NewBaseAnt: // gather at new base, bring food along
    if (fields->Base==0) {
      dir=16;
      if (fields->NumAnts>NewBaseAnts) {
	max=1;
	for(i=1;i<fields->NumAnts;i++) {
	  if (mem[i].n.state==NewBaseAnt)
	    max++;
	}
	if (max>NewBaseAnts) {
	  XT=0;YT=0;SETFOOD(mem,0);dir=0;
	  //mem->n.state=SearchAndDestroyAnt;
	  mem->n.state=UninitialisedAnt;
	}
      }
    } else {
      mem->n.state=SearchAndDestroyAnt;
    }
    break;
  case PatrolAnt:
    if (fields->NumFood>0) {
      if (X==0 && FirkAntAbs(Y)!=BASERANGE) {
	dir=FirkAntWalk(X,Y,0,0)+8;
	break;
      }
      if (Y==0 && FirkAntAbs(X)!=BASERANGE) {
	dir=FirkAntWalk(X,Y,0,0)+8;
	break;
      }
    }
    if (fields->NumFood>0 && (X!=0 || Y!=0)) { // found food, but am I necessary?
      max=0;
      for(i=1;i<fields->NumAnts;i++) {
	if(mem[i].n.state==ReturnFoodAnt) {
	  max++;
	}
      }
      // if more food than ants then help
      if (fields->NumFood>max) {
	XT=X;YT=Y;
	mem->n.state=ReturnFoodAnt;
	SETFOOD(mem,(fields->NumFood-max)/3);
	FirkAntSwitchBase(mem);
      }
    } else {
      if (mem->n.state==PatrolAnt) { // no food located
	// XT=range, YT=state
	y=GETFOOD(mem); // holds direction
	if (mem->n.dir) {
	  x=y+1;if (x>4) x-=4;  // next direction (clockwise)
	} else {
	  x=y-1;if (x<1) x+=4;  // next direction (counter-clockwise)
	}
	switch(YT) {
	case 0: // going away from home
	  dir=FirkAntWalk(X,Y,Xd[y]*XT,Yd[y]*XT);
	  if (dir==0) YT++;
	  break;
	case 1: // going away, first turn
	  dir=FirkAntWalk(X,Y,
			  (Xd[y]+Xd[x])*XT,
			  (Yd[y]+Yd[x])*XT);
	  if (dir==0) YT++;
	  break;
	case 2: // going home, second turn
	  dir=FirkAntWalk(X,Y,Xd[x]*XT,Yd[x]*XT);
	  if (dir==0) YT++;
	  break;
	case 3: // going home, last turn
	  dir=FirkAntWalk(X,Y,0,0);
	  if (dir==0) YT++;
	  break;
	case 4:
	  mem->n.state=UninitialisedAnt; // reuses as patrolant
	  XT=0;
	  YT=0;
	  SETFOOD(mem,0);
	  if (X!=0 || Y!=0) {
	    mem->n.state=SearchAndDestroyAnt; // will go to XT,YT
	  }
	default:
	  break;
	}
      }
      
      if (X!=0 || Y!=0)
	for(i=1;i<5;i++) // food around me?
	  if(fields[i].NumFood>0)
	    if (fields[i].NumAnts<fields[i].NumFood)
	      if (X+Xd[i]!=0 || Y+Yd[i]!=0)
		dir=i;
      
    }
    // prevent unnecessary base at all cost
    if (fields->Base==0)
      if (FirkAntAbs(X)==32 || FirkAntAbs(Y)==32) {
	for(i=1;i<fields->NumAnts;i++)
	  if (mem[i].n.state==BaseAnt) {
	    mem->n.state=PreventBaseAnt;
	    dir=0;
	  }
      }
    break;
  case PreventBaseAnt:
    for(i=1;i<fields->NumAnts;i++)
      switch(mem[i].n.state) {
      case SearchAndDestroyAnt:
      case PatrolAnt:
	if (mem[i].n.xpos==0 && mem[i].n.ypos==0) {
	  mem[i].n.xpos=X;
	  mem[i].n.ypos=Y;
	}
	break;
      case ReturnFoodAnt:
	if (mem[i].n.xpos==0 && mem[i].n.ypos==0) {
	  mem[i].n.xtarget=XT-X;
	  mem[i].n.ytarget=YT-Y;
	  mem[i].n.xpos=X;
	  mem[i].n.ypos=Y;
	}
	break;
      case BaseAnt:
	mem[i].n.state=SearchAndDestroyAnt;
	mem[i].n.xpos=X;
	mem[i].n.ypos=Y;
	mem[i].n.xtarget=0;
	mem[i].n.ytarget=0;
	mem[i].n.dir=0;
	SETFOOD(&mem[i],0);
	break;
      case PreventBaseAnt:
	mem->n.state=SearchAndDestroyAnt;
	i=fields->NumAnts; // terminate early
      default:
	break;
      }
    break;
  default:
    break;
  }

}
  
  // make sure there is a base ant, very important
  if (fields->Base==1 || (fields->Base==0 && X==0 && Y==0)) {
    max=0;
    for(i=0;i<fields->NumAnts;i++)
      if (mem[i].n.state==BaseAnt) {
	max++;
	i=fields->NumAnts; // terminate early (only happens in base, but thats also where most ants are in one spot
      }
    if (max==0) { // no base ant, so create one.
      mem->n.state=BaseAnt;
      dir=0;
      FD=0;W=0;
      SETCOUNT(mem,0);
      mem->b.firstbase=0;
      // of-by-one errors are hard to avoid
      for(i=1;i<5;i++)
	if (fields[i].Base) {
	  dir=i;
	  mem->n.state=UninitialisedAnt;
	}
    }
  }

  // create guards were needed (base ant excluded)
  for(i=1;i<5;i++)
    if(fields[i].Team>0) {
      dir=i;
      if (mem->n.state==BaseAnt) {
	X=0;Y=0; // X,Y in BaseAnt has no meaning (random values)
      }
      mem->n.state=GuardAnt;
      SETCOUNT(mem,1500); // guard for 1500 turns
    }

  // keep track of where we are (can fail and then ant is lost)
  //   check for overflow and backtrack variables
  d=DIST(X,Y,0,0);
  if ((fields[dir&7].NumAnts>=MaxSquareAnts-d*5 && d<4) ||
      (fields[dir&7].NumAnts>=MaxSquareAnts)) {
    return 0;
  }
  // the only ants that can cause overflow are the SearchAndDestroyAnts.
  if (mem[i].n.state==SearchAndDestroyAnt) 
    //if (DIST(X+Xd[dir&7],Y+Yd[dir&7],0,0)>254) {
    if (FirkAntAbs(X+Xd[dir&7])>126 || FirkAntAbs(Y+Yd[dir&7])>126) {
      XT=0;
      YT=0;
      SETFOOD(mem,0);
      dir=dir&24;
    }

  X+=Xd[dir&7];
  Y+=Yd[dir&7];
  return dir;
}

DefineAnt(FirkAnt,"FirkAnt#8080ff",FirkAntBrain,union FirkAntMemory);
