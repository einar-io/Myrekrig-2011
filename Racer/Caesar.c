/* Caesar, bernhard@ege.cc */
#include "Myre.h"

extern int abs(int);

#define X mem->n.xpos
#define Y mem->n.ypos
#define XT mem->n.xtarget
#define YT mem->n.ytarget
#define PD mem->b.patroldistance

#define DIST(x1,y1,x2,y2) (abs(x1-x2)+abs(y1-y2))

#define SETFOOD(mem,value) tmp=value;(mem)->n.food=(tmp>255?255:(tmp<0?0:tmp));
#define GETFOOD(mem) ((mem)->n.food)

#define SETCOUNT(mem,value) (mem)->b.count=value;
#define GETCOUNT(mem) ((mem)->b.count)

enum { UninitialisedAnt=0, ScoutAnt, ReturnFoodAnt, GuardAnt, 
       BaseAnt, NewBaseAnt, PatrolAnt };

union CaesarMemory {
  struct {
    signed short xpos;
    signed short ypos;
    signed short xtarget;
    signed short ytarget;
    unsigned char food;
    unsigned char unused:4;
    unsigned char dir:1;
    unsigned char state:3;
  } n;
  struct {
    unsigned short patroldistance,unused1;
    unsigned short count,unused2;
    unsigned char unused3;
    unsigned char unused4:4;
    unsigned char firstbase:1;
    unsigned char state:3;
  } b;  //BaseAnt
  struct {
    unsigned short r1,r2,r3,r4;
    unsigned char unused;
    unsigned char r5;
  } r;
};

#define RAND_INT(m,f,l,h) (((signed int)(CaesarRandom(m,f) * ((float)(h)-(float)(l)+1))) + (l))
/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */

inline float CaesarRandom (union CaesarMemory *mem, struct SquareData *fields) {
  unsigned int r;
  r=((mem->r.r4<<16)|(mem->r.r3)|(mem->r.r2<<16)|mem->r.r1)^
    (mem->r.r5)^
    (((fields->NumAnts*76531)^(fields->NumFood*1001)));
  r=1588635695*(r % 2) - 1117695901*(r / 2);
  return (float)(r)/(float)4294967291U;
}

short CaesarSqrt(int n) {
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

inline char CaesarWalk(short x1, short y1, short x2, short y2) {
  if (DIST(x1,y1,x2,y2)==0)
    return 0;
  if (abs(x1-x2)>abs(y1-y2)) {
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

char CaesarCrossWalk(short x1, short y1, short x2, short y2) {
  if (abs(x2-x1)>abs(y2-y1)) {
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

// fields:   NumAnts, Base, Team, NumFood
// egen mem: mem
// andre:    mem[1], mem[2], ..., mem[fields->NumAnts-1]
// return:   0: stay, 1: East, 2: South, 3: West, 4: North
int CaesarBrain(struct SquareData *fields, union CaesarMemory *mem) {
  const int Xd[]={0, 1,  0, -1, 0};
  const int Yd[]={0, 0, -1,  0, 1};

  int i,max,d,dir=0,x,y,tmp,prev_state=-1; // tmp used by macros

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
      PD=0;
      SETCOUNT(mem,0);
      mem->b.firstbase=1;
    }
    break;
  case ScoutAnt:
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
	case ScoutAnt:
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
	SETFOOD(mem,(fields->NumFood-max)/2);
      }
    } else {
      if (X==XT && Y==YT) { // search along another path
	if (X==0 && Y==0) {
	  mem->n.state=UninitialisedAnt;
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
	  i=CaesarSqrt(X*X+Y*Y); // not exactly a cheap operation
	  x=(x*10*(i+20))/(14*i); // +20,/14
	  y=(y*10*(i+20))/(14*i);
	  XT=x;YT=y;
	  mem->n.dir=RAND_INT(mem,fields,0,1);
	}
      } else {
	dir=CaesarWalk(X,Y,XT,YT);
	if (XT==0 && YT==0) {
	  dir|=8;
	} else {
	  if (X!=0 || Y!=0)
	    for(i=1;i<5;i++) // food around me?
	      if(fields[i].NumFood>0 && fields[i].NumAnts<fields[i].NumFood)
		if (X+Xd[i]!=0 && Y+Yd[i]!=0)
		  dir=i;  // take it
	}
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
	}
      // more ants than food?
      if (max>fields->NumFood) {
	mem->n.state=ScoutAnt;
      } else {
	dir=CaesarCrossWalk(X,Y,0,0)+8;
	for(i=1;i<fields->NumAnts;i++) { // tell other where food is
	  if (GETFOOD(mem)>1 ) {
	    mem[i].n.xtarget=XT-(X-mem[i].n.xpos);
	    mem[i].n.ytarget=YT-(Y-mem[i].n.ypos);
	    mem[i].n.xpos=X;mem[i].n.ypos=Y;
	    mem[i].n.state=ScoutAnt;
	    d=GETFOOD(mem)-3;
	    SETFOOD(&mem[i],d);
	    SETFOOD(mem,d);
	  }
	}
      }
    } else {
      mem->n.state=ScoutAnt;

      // don't leave base if we need more ants to (re)create it
      if (fields->Base==0 && fields->NumFood>=NewBaseFood) {
	mem->n.state=UninitialisedAnt;
      }
    }
    break;
  case GuardAnt: // guard forever
    SETCOUNT(mem,GETCOUNT(mem)-1); // counter
    if (GETCOUNT(mem)==0) { // timeout, continue searching
      mem->n.state=ScoutAnt;
      XT=X;
      YT=Y;
    } else if (X!=0 || Y!=0) { // am i anywhere but home?
      // draft more guards if necessary
      d=0;
      max=1;
      for(i=1;i<fields->NumAnts;i++)
	if(mem[i].n.state!=GuardAnt && mem[i].n.state!=ReturnFoodAnt) {
	  d=i;
	} else if (mem[i].n.state==GuardAnt) {
	  max++;
	}
      if(max<3 && d!=0 && GETCOUNT(mem)>500) {
	mem[d].n.state=GuardAnt;
	SETCOUNT(mem,GETCOUNT(mem)/2);
	SETCOUNT(&mem[d],GETCOUNT(mem));
      }

      // distribute guards a bit
      if (max==2) {
	if (X>0 && Y>0)
	  dir=((RAND_INT(mem,fields,0,1)+3)%4)+1;
	if (X>0 && Y<0)
	  dir=RAND_INT(mem,fields,1,2);
	if (X<0 && Y<0)
	  dir=RAND_INT(mem,fields,2,3);
	if (X<0 && Y>0)
	  dir=RAND_INT(mem,fields,3,4);
      }
      
      // might as well check for food, but let single pieces be as bait
      for(i=0;i<5;i++)
        if(fields[i].NumFood>0)
	  if (fields[i].NumAnts<fields[i].NumFood && fields[i].NumFood>1)
	    if (X+Xd[i]!=0 || Y+Yd[i]!=0) {
	      dir=i;
	      mem->n.state=ReturnFoodAnt;
	      XT=X+Xd[i];YT=Y+Yd[i];
	      SETFOOD(mem,(fields[i].NumFood));
            }

    } else { // Guard at home, very unlikely and not wanted
      mem->n.state=UninitialisedAnt;
    }
    
    break;
  case BaseAnt: // base ant, never moves, specielt use of X,Y,XT,YT
    // count base ants, one is enough
    d=0;
    for(i=1;i<fields->NumAnts;i++) {
      if (mem[i].n.state==BaseAnt)
	d++;
      }
    if (d>0) { // someone else is base ant
      mem->n.state=UninitialisedAnt;
      X=0;Y=0;XT=0;YT=0;SETFOOD(mem,0);mem->n.dir=0;
    } else {
      if ((fields->Base==1) || 
	  (fields->Base==0 && fields->NumFood<NewBaseFood)) {
	for(i=1;i<fields->NumAnts;i++) {
	  switch(mem[i].n.state) {
	  case PatrolAnt:
	    if (mem[i].n.ytarget==4) {
	      x=mem[i].n.xtarget;
	      if (x>PD) {
		PD=x;
	      }
	    }
	    break;
	  case UninitialisedAnt:
	    d=GETCOUNT(mem);
	    mem[i].n.xpos=0;
	    mem[i].n.ypos=0;
	    max=(PD*2)/3;
	    if (max<90/3) max=90/3;

	    // (range, x-bits)(compas, 2 bits)(clockwise, 1 bit)
	    mem[i].n.xtarget=((d>>3)%(max))*3+3;
	    mem[i].n.dir=d&1;
	    SETFOOD(&mem[i],((d&6)>>1)+1);
	    mem[i].n.ytarget=0;
	    mem[i].n.state=PatrolAnt;

	    SETCOUNT(mem,GETCOUNT(mem)+1);
	    if (d>max*8) {
	      SETCOUNT(mem,0);
	    }
	    break;
	  default:
	    break;
	  }
	}
      } else {
	for(i=1;i<fields->NumAnts;i++) {
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
	  mem->n.state=UninitialisedAnt;
	}
      }
    } else {
      mem->n.state=UninitialisedAnt;
    }
    break;
  case PatrolAnt:
    // move food towards home (if on-axis)
    if (fields->NumFood>0) {
      max=0;
      if (X==0) {
	d=abs(Y);
	max=0;
	for(i=1;i<fields->NumAnts;i++)
	  if(abs(mem[i].n.ypos)<d) {
	    i=fields->NumAnts;
	    max=1;
	  }
	if (max==0) {
	  dir=4+8;
	  if (Y>0) dir=2+8;
	  break; // early termination
	}
      }
      if (Y==0) {
	d=abs(X);
	max=0;
	for(i=1;i<fields->NumAnts;i++)
	  if(abs(mem[i].n.xpos)<d) {
	    i=fields->NumAnts;
	    max=1;
	  }
	if (max==0) {
	  dir=1+8;
	  if (X>0) dir=3+8;
	  break; // early termination
	}
      }
    }
    // found food on off-axis
    d=0;
    if (fields->NumFood>0 && (X!=0 || Y!=0)) { // found food, but am I necessary?
      max=0;
      for(i=1;i<fields->NumAnts;i++) {
	if(mem[i].n.state==ReturnFoodAnt) {
	  max++;
	  d=i;
	}
      }
      // if more food than ants then help
      if (fields->NumFood>max) {
	XT=X;YT=Y;
	mem->n.state=ReturnFoodAnt;
	SETFOOD(mem,(fields->NumFood-max));
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
	  dir=CaesarWalk(X,Y,Xd[y]*XT,Yd[y]*XT);
	  if (dir==0) YT++;
	  break;
	case 1: // going away, first turn
	  dir=CaesarWalk(X,Y,
			  (Xd[y]+Xd[x])*XT,
			  (Yd[y]+Yd[x])*XT);
	  if (dir==0) YT++;
	  break;
	case 2: // going home, second turn
	  dir=CaesarWalk(X,Y,Xd[x]*XT,Yd[x]*XT);
	  if (dir==0) YT++;
	  break;
	case 3: // going home, last turn
	  dir=CaesarWalk(X,Y,0,0);
	  if (dir==0) YT++;
	  break;
	case 4:
	  mem->n.state=UninitialisedAnt; // reuses as patrolant
	  if (X!=0 || Y!=0) {
	    mem->n.state=ScoutAnt; // will go to XT,YT
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
      PD=0;
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

  X+=Xd[dir&7];
  Y+=Yd[dir&7];
  return dir;
}

DefineAnt(Caesar,"Caesar#808080",CaesarBrain,union CaesarMemory);
