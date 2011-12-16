/* Smiley, bme@miba.auc.dk */

#include "Myre.h"

#define BASERANGE 96
#define FONTX 20
#define FONTY 5

#define X mem->n.xpos
#define Y mem->n.ypos
#define XT mem->n.xtarget
#define YT mem->n.ytarget
#define F mem->n.food
#define R mem->b.radius
#define F1 mem->b.f1
#define F2 mem->b.f2
#define F3 mem->b.f3
#define F4 mem->b.f4
#define F5 mem->b.f5
#define F6 mem->b.f6

#define MIN(a,b) (a)<(b)?(a):(b)
#define DIST(x1,y1,x2,y2) (SmileyAbs(x1-x2)+SmileyAbs(y1-y2))
#define ROT(d) ((((d&7)%4)+1)|(d&24))

enum { UninitialisedAnt=0, SearchAndDestroyAnt, ReturnFoodAnt, GuardAnt, 
       BaseAnt, NewBaseAnt, PreDrawAnt, DrawAnt };

union SmileyMemory {
  struct {
    short xpos;
    short ypos;
    short food;
    short xtarget:12;
    unsigned short rnd:3;
    unsigned short dir:1;
    short ytarget:12;
    unsigned short state:4;
  } n;
  struct {
    unsigned char f1,f2,f3,f4;
    unsigned char f5,f6;//,firstbase,food;
    unsigned short radius;
    unsigned short count:11;
    unsigned short firstbase:1;
    unsigned short state:4;
  } b;
  struct {
    unsigned short r1,r2,r3,r4,r5;
  } r;
};


#define RAND_INT(m,f,l,h) (((unsigned short)(SmileyRandom(m,f) * ((float)(h)-(float)(l)+1))) + (l))
/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */

inline float SmileyRandom (union SmileyMemory *mem, struct SquareData *fields) {
  unsigned int r;
  //r=(mem->r.r1|(mem->r.r2<<16))^
  //  (mem->r.r3|(mem->r.r4<<16))^
  r=(*((unsigned int *)mem))^
    (*((unsigned int *)mem+1))^
      (mem->r.r5|((fields->NumAnts<<8)|fields->NumFood));
  mem->n.rnd=mem->n.rnd*3+1; // cycles "randomly" from 0 to 7
  r=1588635695*(r % 2) - 1117695901*(r / 2);
  return (float)(r)/(float)4294967291U;
}

inline int SmileyAbs(int a) {
  if (a>=0)
    return a;
  return -a;
}

inline int SmileyMax(int a, int b) {
  if (a>b)
    return a;
  return b;
}

inline short SmileySqrt(int n) {
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

inline char SmileyWalk(short x1, short y1, short x2, short y2) {
  if (DIST(x1,y1,x2,y2)==0)
    return 0;
  if (SmileyAbs(x1-x2)>SmileyAbs(y1-y2)) {
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

inline char SmileyFuzzyWalk(union SmileyMemory *mem, struct SquareData *fields,short x1, short y1, short x2, short y2) {
  if (RAND_INT(mem,fields,-SmileyAbs(x1-x2),SmileyAbs(y1-y2))<0) {
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
int SmileyBrain(struct SquareData *fields, union SmileyMemory *mem) {
  const int Xd[]={0, 1,  0, -1, 0};
  const int Yd[]={0, 0, -1,  0, 1};
  
  const u_char bit2[11][9]={{4,10,17,17,17,17,17,10,4},   // 0
			  {4,12,20,4,4,4,4,4,31},       // 1
			  {14,17,17,1,2,4,8,16,31},     // 2
			  {31,1,2,4,14,1,1,17,14},      // 3
			  {2,2,6,10,10,18,31,2,2},      // 4
			  {31,16,16,22,25,1,1,17,14},   // 5
			  {14,17,16,16,30,17,17,17,14}, // 6
			  {31,1,2,2,4,4,8,8,8},         // 7
			  {14,17,17,17,14,17,17,17,14}, // 8
			  {14,17,17,17,15,1,1,17,14},   // 9
			  {99,99,8,8,65,34,28,0,0}};    // smiley
  int i,max,d,dir,x,y;

  dir=0;
  switch(mem->n.state) {
  case UninitialisedAnt:
    mem->n.rnd=mem->n.xpos;
    mem->n.xpos=0; // part of the initial random long, used for something else now.
    mem->n.ypos=0;
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
      F1=0;F2=0;F3=0;F4=0;F5=0;F6=0;R=0;
      mem->b.count=0;
      mem->b.firstbase=1;
    }
    break;
  case SearchAndDestroyAnt:
    if (fields->NumFood>0 && (X!=0 || Y!=0)) { // found food, but am I necessary?
      max=0;
      for(i=1;i<fields->NumAnts;i++) {
	if(mem[i].n.state==ReturnFoodAnt) {
	  max++;
	} else if (mem[i].n.state==BaseAnt) { // pick new home
	  XT-=X;
	  YT-=Y;
	  X=0;
	  Y=0;
	  break;
	}
      }
      // if more food than ants then help
      if (fields->NumFood>max) {
	XT=X;YT=Y;
	mem->n.state=ReturnFoodAnt;
	mem->n.food=fields->NumFood-max;
      }
    } else {
      if (X==XT && Y==YT) { // search along another path
	if (X==0 && Y==0) {
	  XT=RAND_INT(mem,fields,-16,16);
	  YT=RAND_INT(mem,fields,-16,16);
	  mem->n.dir=RAND_INT(mem,fields,0,1);
	} else {
	  if (mem->n.food>0) { // there was supposed to be food, but its all gone
	    XT=RAND_INT(mem,fields,XT,XT*1.10);
	    YT=RAND_INT(mem,fields,YT,YT*1.10);
	    F=0;
	  } else if (mem->n.dir) {
	    XT=X+Y;
	    YT=Y-X;
	  } else {
	    XT=X-Y;
	    YT=Y+X;
	  }
	  i=SmileySqrt(X*X+Y*Y); // not exactly a cheap operation
	  if (RAND_INT(mem,fields,0,1)==1) { // choose between 26.6 and 45 deg
	    XT=(XT+X)/2; // 1.118=sqrt(10*10+5*5)/10
	    YT=(YT+Y)/2;
	    XT=(XT*10*(i+10))/(11*i); // +10/11
	    YT=(YT*10*(i+10))/(11*i);
	  } else {
	    XT=(XT*10*(i+20))/(14*i); // +20,/14
	    YT=(YT*10*(i+20))/(14*i);
	  }
	}
      } else {
	dir=SmileyFuzzyWalk(mem,fields,X,Y,XT,YT);
	if (X!=0 || Y!=0)
	  for(i=1;i<5;i++) // food around me?
	    if(fields[i].NumFood>0)
	      if (fields[i].NumAnts<fields[i].NumFood)
		if (X+Xd[i]!=0 && Y+Yd[i]!=0)
		  dir=i;  // take it
      }
    }
    break;
  case ReturnFoodAnt:
    // check ants to see which home is closer
    for(i=1;i<fields->NumAnts;i++)
      if (DIST(mem[i].n.xpos,mem[i].n.ypos,0,0)<DIST(X,Y,0,0) && mem[i].n.state!=BaseAnt) {
	// fix target
	XT=XT-(X-mem[i].n.xpos);
	YT=YT-(Y-mem[i].n.ypos);
	// fix home
	X=mem[i].n.xpos;
	Y=mem[i].n.ypos;
      }
    if (fields->NumFood>0 && (X!=0 || Y!=0)) {
      // how many are fetching food right here?
      d=DIST(XT,YT,0,0);
      max=0;
      for(i=0;i<fields->NumAnts;i++)
	if(mem[i].n.state==ReturnFoodAnt) {
	  max++;
	  if (d>DIST(mem[i].n.xtarget,mem[i].n.ytarget,0,0) && mem[i].n.food>3) {
	    XT=mem[i].n.xtarget+(X-mem[i].n.xpos); // pick closer food target
	    YT=mem[i].n.ytarget+(Y-mem[i].n.ypos);
	    d=DIST(XT,YT,0,0);
	    mem->n.food=mem[i].n.food-3;
	  }
	}
      // more ants than food?
      if (max>fields->NumFood)
	mem->n.state=SearchAndDestroyAnt;
      else {
	dir=SmileyFuzzyWalk(mem,fields,X,Y,0,0)+8;
	for(i=1;i<fields->NumAnts;i++) { // tell other where food is
	  if (mem[i].n.state==SearchAndDestroyAnt && mem->n.food>2) {
	    mem[i].n.xtarget=XT-(X-mem[i].n.xpos);
	    mem[i].n.ytarget=YT-(Y-mem[i].n.ypos);
	    mem[i].n.state=SearchAndDestroyAnt;
	    mem[i].n.food=mem->n.food-3;
	    mem->n.food-=3;
	  }
	}
      }
    } else {
      mem->n.state=SearchAndDestroyAnt;
    }
    break;
  case GuardAnt: // guard forever
    mem->n.xtarget--; // counter...
    if (mem->n.xtarget==0) {
      mem->n.state=SearchAndDestroyAnt;
      XT=X;
      YT=Y;
    } else if (X!=0 || Y!=0) {
      for(i=0;i<5;i++) // food around me?
        if(fields[i].NumFood>0 && fields[i].NumAnts<fields[i].NumFood)
          if (X+Xd[i]!=0 || Y+Yd[i]!=0) {
            dir=i;
            mem->n.state=ReturnFoodAnt;
	    XT=X+Xd[i];YT=Y+Yd[i];F=fields[i].NumFood;
          }
    }

    break;
  case BaseAnt: // base ant, never moves, specielt use of X,Y,XT,YT
    // adjust offset of new ants to weak areas
    mem->b.count++;
    // how many new ants?
    max=0;
    d=0;
    for(i=1;i<fields->NumAnts;i++) {
      if (mem[i].n.state==UninitialisedAnt) max++;
      if (mem[i].n.state==BaseAnt) d++;
    }
    if (d>0) { // someone else is base ant
      mem->n.state=SearchAndDestroyAnt;
      X=0;Y=0;XT=0;YT=0;F=0;
    } else if (fields->Base==1) {
      if (mem->b.count>550) {
	F1=0;F2=0;F3=0;F4=0;F5=0;F6=0;R=0;
	mem->b.count=0;
      } else if (mem->b.count>400) { // create base
	switch(F1) {
	case 1: x=BASERANGE*0.5; y=BASERANGE*0.866; break;
	case 2: x=BASERANGE; y=0; break;
	case 3: x=BASERANGE*0.5; y=-BASERANGE*0.866; break;
	case 4: x=-BASERANGE*0.5; y=-BASERANGE*0.866; break;
	case 5: x=-BASERANGE; y=0; break;
	case 6: x=-BASERANGE*0.5; y=BASERANGE*0.866; break;
	default: x=0;y=0; break; // keep compiler from complaining.
	}
	for(i=1;i<fields->NumAnts;i++)
	  if (mem[i].n.state==UninitialisedAnt) {
	    mem[i].n.state=NewBaseAnt;
	    mem[i].n.xpos=-x;
	    mem[i].n.ypos=-y;
	  }
      } else if (mem->b.count==100) { // decide after 100 turns what to do
	R/=100;
#define FR BASERANGE+10
	if ((R>BASERANGE*0.7) &&
	    (F1>FR || F2>FR || F3>FR || F4>FR || F5>FR || F6>FR)) {
	  max=SmileyMax(SmileyMax(SmileyMax(F1,F2),SmileyMax(F3,F4)),SmileyMax(F5,F6));
	  if (F1==max) { 
	    F1=1; 
	  } else if (F2==max) { 
	    F1=2; 
	  } else if (F3==max) { 
	    F1=3; 
	  } else if (F4==max) { 
	    F1=4;
	  } else if (F5==max) { 
	    F1=5; 
	  } else { // only F6 possible now
	    F1=6;
	  }
	  mem->b.count=400;
	} else { // reset counters, i.e. check again after 100 turns
	  F1=0;F2=0;F3=0;F4=0;F5=0;F6=0;R=0;
	  mem->b.count=0;
	}
      } else { // mem->b.count is less than 100
	// count food arriving from 8 directions, if base exist
	for(i=1;i<fields->NumAnts;i++)
	  if (mem[i].n.state==ReturnFoodAnt) {
	    x=mem[i].n.xtarget;
	    y=mem[i].n.ytarget;
	    d=SmileySqrt(x*x+y*y);
	    if (x>0) {
	      if (x*100>SmileyAbs(y)*173) {
		if (d>F2 && F2<240) F2=(F2*7+d)/8;
	      } else if (y>0) {
		if (d>F1 && F1<240) F1=(F1*7+d)/8;
	      } else {
		if (d>F3 && F3<240) F3=(F3*7+d)/8;
	      }
	    } else {
	      if (x*100<-SmileyAbs(y)*173) {
		if (d>F5 && F5<240) F5=(F5*7+d)/8;
	      } else if (y>0) {
		if (d>F6 && F6<240) F6=(F6*7+d)/8;
	      } else {
		if (d>F4 && F4<240) F4=(F4*7+d)/8;
	      }
	    }
	    R+=d;
	    //if (R>65000) R=65000;
	  }
	d=mem->b.count%20;//RAND_INT(&mem[1],&fields[1],0,19);
	for(i=1;i<fields->NumAnts;i++)
	  if (mem[i].n.state==UninitialisedAnt) {
	    mem[i].n.state=SearchAndDestroyAnt;
	    mem[i].n.xpos=0;
	    mem[i].n.ypos=0;
	    if (d<5) { x=3; y=2-d; }
	    else if (d<10) { x=2-(d-5); y=-3; }
	    else if (d<15) { x=-3; y=-2+(d-10); }
	    else { x=-2+(d-15); y=3; }
	    d=(d+7)%20;
	    mem[i].n.xtarget=x;
	    mem[i].n.ytarget=y;
	    mem[i].n.dir=(mem->b.count+i)&1;
	    if (mem->b.firstbase==1)
	      if (RAND_INT(&mem[i],fields,1,250)==1) {
		mem[i].n.state=PreDrawAnt;
		mem[i].n.food=10;  // do the smiley thing :-)
	      }
	  }
      } // not more mem->b.count checks
    } else { // no base, special case
      if (fields->NumFood>=NewBaseFood) { // we have enough food, so gather enough ants to create a base
	for(i=1;i<fields->NumAnts;i++)
	  if (mem[i].n.state!=BaseAnt) {
	    mem[i].n.state=NewBaseAnt;
	    mem[i].n.xpos=0;
	    mem[i].n.ypos=0;
	    mem[i].n.xtarget=0;
	    mem[i].n.ytarget=0;
	    mem[i].n.food=1;
	  }
      } else { // not enough food, send out searchers
	for(i=1;i<fields->NumAnts;i++)
	  if (mem[i].n.state==NewBaseAnt) {
	    mem[i].n.state=SearchAndDestroyAnt;
	    mem[i].n.xtarget-=mem[i].n.xpos;
	    mem[i].n.ytarget-=mem[i].n.ypos;
	    mem[i].n.xpos=0;
	    mem[i].n.ypos=0;
	    mem[i].n.food=0;
	  } else if (mem[i].n.state==UninitialisedAnt) {
	    mem[i].n.state=SearchAndDestroyAnt;
	    mem[i].n.xpos=0;
	    mem[i].n.ypos=0;
	  }	    
      }
      mem->b.count=0;
    }
    break;
  case NewBaseAnt: // gather at new base, bring food along
    if (X==0 && Y==0) {
      if (fields->Base==0) {
	dir=16;
	if (fields->NumAnts>=NewBaseAnts) {
	  max=1;
	  for(i=1;i<fields->NumAnts;i++) {
	    if (mem[i].n.state==NewBaseAnt)
	      max++;
	  }
	  if (max>NewBaseAnts) {
	    XT=0;YT=0;F=0;dir=0;
	    mem->n.state=SearchAndDestroyAnt;
	  }
	}
      } else {
	mem->n.state=SearchAndDestroyAnt;
      }
    } else {
      dir=SmileyFuzzyWalk(mem,fields,X,Y,XT,YT)|8; // take food, attracts search and food ants
    }
    break;
  case PreDrawAnt: // draw ant, used to draw images
    // food contains image nr to be drawn
    if (mem->n.food>=0 && mem->n.food<=10) {
      XT=FONTX;
      YT=FONTY;
      mem->n.state=DrawAnt;
    } else {
      mem->n.state=SearchAndDestroyAnt;
    }
    break;
  case DrawAnt: // walk to designated spot
    // mem->food bits 12-15=xpos, bits 8-11=ypos
    x=mem->n.food>>12;      // 0-15
    y=(mem->n.food>>8)&15;  // 0-15
    if (X==(XT-x) && Y==(YT-y)) {
      // should bit be set?
      d=bit2[mem->n.food&255][y];
      for(i=0;i<x;i++)
	d=d>>1;
      max=0;
      for(i=0;i<fields->NumAnts;i++)
	if (mem[i].n.state==GuardAnt) {
	  max++;
	  mem[i].n.xtarget=5000; // stay! (delay 5000)
	  i=fields->NumAnts; // terminate early, 1 or more guards, it doesn't matter.
	}
      if (max>0 || (d&1)==0) { // bit already set, so calc new address
	do {
	  x++;
	  d=d>>1;
	  if (x>7) {
	    x-=8;
	    y++;
	    if (y<9)
	      d=bit2[mem->n.food&255][y];
	  }
	} while (((d&1)==0) && (y<9));
	if (y>=9) {
	  mem->n.state=SearchAndDestroyAnt; // no more ants needed
	  F=0;XT=0;YT=0;
	}
      } else {
	mem->n.state=GuardAnt;
	mem->n.xtarget=5000;
      }
      mem->n.food=(mem->n.food&255)|(x<<12)|(y<<8);
    } else {
      dir=SmileyWalk(X,Y,XT-x,YT-y);
    }
    break;
  default:
    break;
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
      F1=0;F2=0;F3=0;F4=0;F5=0;F6=0;R=0;
      mem->b.count=0;
      mem->b.firstbase=0;
      // of-by-one errors are hard to avoid (I do try)
      for(i=1;i<5;i++)
	if (fields[i].Base) {
	  dir=i;
	}
    }
  }

  // create guards were needed
  for(i=1;i<5;i++)
    if(fields[i].Team>0) {
      dir=i;
      mem->n.state=GuardAnt;
      mem->n.xtarget=1500; // guard for 1500 turns
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

DefineAnt(Smiley,"Smiley#ff8000",SmileyBrain,union SmileyMemory);
