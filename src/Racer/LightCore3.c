#include "Myre.h"

/// Defines...
#define LCFoodLimit 3
#define LCInitRange 20
#define LCMarginPercent 50
#define LCMarginExtra 20
#define LCRangeAdapt 10
#define LCDanger 200
#define LCDangerStepInterval 20
#define LCDangerStepFactor 102
#define LCSafety 10
#define LCMaxAnts (MaxSquareAnts-10)

#define absv(a) ((a) >= 0 ? (a) : -(a))
#define val(x,y) (absv(x)+absv(y))
#define xdir(d) (((d)==1)-((d)==3))
#define ydir(d) (((d)==2)-((d)==4))

#define LC_Status_Base 0
#define LC_Status_Searching 1
#define LC_Status_FoundNothing 2
#define LC_Status_FoundFood 3
#define LC_Status_RememberFood 4
#define LC_Status_Queen 5

#define xrange brain1.queen.xrange_
#define yrange brain1.queen.yrange_
#define xpos brain1.worker.xpos_
#define ypos brain1.worker.ypos_
#define xdest brain1.worker.xdest_
#define ydest brain1.worker.ydest_
#define route brain2.route_
#define num brain2.num_
//|

/// Strukturer...
struct LC3Brain {
   union {
      struct {
	long xrange_,yrange_;
      } queen;
      struct {
         short xdest_,ydest_, xpos_,ypos_;
      } worker;
   } brain1;
   union {
     short route_;
     short num_;
   } brain2;
   short status;
};
//|
/// Prototyper...
int LightCore3(struct SquareData *felt, struct LC3Brain *mem);
int LC3Main(struct SquareData *felt, struct LC3Brain *mem);
int LC3Queen(struct SquareData *felt, struct LC3Brain *mem);
//|

/// void LC3FoodLook(struct SquareData *felt, struct LC3Brain *mem) 
void LC3FoodLook(struct SquareData *felt, struct LC3Brain *mem) {
   int f;

   /* Kig efter mad */
   if (felt[1].NumFood+felt[2].NumFood+felt[3].NumFood+felt[4].NumFood) {
     for (f = 1 ; f <= 4 ; f++) {
       if (felt[f].NumFood > felt[f].NumAnts) {
	 mem->xdest = mem->xpos+xdir(f);
	 mem->ydest = mem->ypos+ydir(f);
	 break;
       }
     }
   }
   if (felt->NumFood > felt->NumAnts) {
     mem->xdest = mem->xpos;
     mem->ydest = mem->ypos;
     mem->num = felt->NumFood-felt->NumAnts;
     mem->status = LC_Status_FoundFood;
   }
}
//|
/// void LC3EnemyLook(struct SquareData *felt, struct LC3Brain *mem)
void LC3EnemyLook(struct SquareData *felt, struct LC3Brain *mem) {
   int f;

   /* Bask fjender !! */
   if (felt[1].Team+felt[2].Team+felt[3].Team+felt[4].Team) {
     for (f = 1 ; f <= 4 ; f++) {
       if (felt[f].Team) {
	   mem->xdest = mem->xpos+xdir(f);
	   mem->ydest = mem->ypos+ydir(f);
	   mem->status = LC_Status_Searching;
	   mem->num = LCDanger;
	   break;
       }
     }
   }
}
//|
/// int LC3FindWayOut(struct LC3Brain *mem)
int LC3FindWayOut(struct LC3Brain *mem) {
  int x = mem->xpos, y = mem->ypos;
  int tox = mem->xdest, toy = mem->ydest;
  if (x == tox) return y < toy ? 2 : 4;
  if (y == toy) return x < tox ? 1 : 3;
  if (x*toy > y*tox) {
    /* Til venstre for */
    return x < tox ? y < toy ? 2 : 1 : y < toy ? 3 : 4;
  } else {
    /* Til højre for */
    return x < tox ? y < toy ? 1 : 4 : y < toy ? 2 : 3;
  }
}
//|
/// int LC3FindWayBack(struct LC3Brain *mem)
int LC3FindWayBack(struct LC3Brain *mem) {
  int x = mem->xpos, y = mem->ypos;
  int fromx = mem->xdest, fromy = mem->ydest;
  if (x == 0) return y < 0 ? 2 : 4;
  if (y == 0) return x < 0 ? 1 : 3;
  if (x*fromy < y*fromx) {
    /* Til venstre for */
    return x < 0 ? y < 0 ? 2 : 1 : y < 0 ? 3 : 4;
  } else {
    /* Til højre for */
    return x < 0 ? y < 0 ? 1 : 4 : y < 0 ? 2 : 3;
  }
}
//|
/// int LightCore3(struct SquareData *felt, struct LC3Brain *mem)
int LightCore3(struct SquareData *felt, struct LC3Brain *mem) {
  int retval, dir;

  retval = LC3Main(felt, mem);

  if((dir = retval&7)) {
    if(felt[dir].NumAnts >= MaxSquareAnts) {
      //printf("(%d,%d)_%d ",mem->xpos,mem->ypos,dir);
      mem->status = LC_Status_Searching;
    } else {
      switch (dir) {
      case 1: mem->xpos++; break;
      case 2: mem->ypos++; break;
      case 3: mem->xpos--; break;
      case 4: mem->ypos--; break;
      }
    }
  }

  return retval;
}
//|
/// int LC3Main(struct SquareData *felt, struct LC2Brain *mem)
int LC3Main(struct SquareData *felt, struct LC3Brain *mem) {
   int m;
   switch(mem->status) {
   case LC_Status_Base:
     {
       /* Jeg er på basen. Se om her er en dronning. */
       for(m = 1 ; m < felt->NumAnts ; m++) {
	 if(mem[m].status == LC_Status_Queen) {
	   return 0;
	 }
       }
       /* Jeg skal være dronning!! */
       mem->status = LC_Status_Queen;
       mem->xrange = LCInitRange;
       mem->yrange = LCInitRange;
       return LC3Queen(felt, mem);
     }
   case LC_Status_Searching:
     LC3FoodLook(felt, mem);
     LC3EnemyLook(felt, mem);
     /* Er jeg nået frem? */
     if(mem->xpos == mem->xdest && mem->ypos == mem->ydest) {
       /* Er her farligt? */
       if (mem->num-- > 0) {
	 if (mem->num % LCDangerStepInterval == 0) {
	   mem->xdest = (mem->xdest*LCDangerStepFactor)/100;
	   mem->ydest = (mem->ydest*LCDangerStepFactor)/100;
	 }
	 return 0;
       }
       /* Er vi nået frem? */
       if (mem->status == LC_Status_Searching) mem->status = LC_Status_FoundNothing;
       /* Jeg er på vej tilbage! */
       return LC3FindWayBack(mem)+8;
     } else {
       assert(absv(mem->xpos)+absv(mem->ypos) <= absv(mem->xdest)+absv(mem->ydest)+1);
       /* Jeg er på vej ud! */
       return LC3FindWayOut(mem);
     }
   case LC_Status_FoundNothing:
      LC3EnemyLook(felt, mem);
   case LC_Status_FoundFood:
      LC3FoodLook(felt, mem);
      /* Er jeg på basen? */
      if(mem->xpos == 0 && mem->ypos == 0) return 0;
      {
	/* Jeg er på vej tilbage! */
	return LC3FindWayBack(mem)+8;
      }
   case LC_Status_RememberFood:
     /*
     if (felt->NumAnts >= LCMaxAnts) {
       //printf("!");
       mem->status = LC_Status_Base;
     }
     */
     if (mem->num <= 0) {
       mem->num--;
       if (mem->num < -(val(mem->xdest,mem->ydest)*2+LCSafety)) {
	 mem->status = LC_Status_Base;
       }
     }
     return 0;
   case LC_Status_Queen:
     return LC3Queen(felt, mem);
   }
   return 0; /* Never reached */
}
//|
/// int LC3Queen(struct SquareData *felt, struct LC3Brain *mem)
int LC3Queen(struct SquareData *felt, struct LC3Brain *mem) {
   int m,fm,fn,bfm,fval,mfval, x,y;
   int costab[10] = { -0x8000, 0x0000, 0x5a82, 0x7642, 0x7d8a, 0x7f62, 0x7fd9, 0x7ff6, 0x7ffe, 0x7fff };
   int sintab[10] = {  0x0000, 0x8000, 0x5a82, 0x30fc, 0x18f9, 0x0c8c, 0x0648, 0x0324, 0x0192, 0x00c9 };

   ///  Indsaml information fra myrerne.
   for(m = 1 ; m < felt->NumAnts ; m++) {
      x = mem[m].xdest; y = mem[m].ydest;
      switch(mem[m].status) {
      case LC_Status_Searching:
      case LC_Status_Base:
      case LC_Status_RememberFood:
         break;
      case LC_Status_FoundFood:
         if (mem[m].num >= LCFoodLimit) {
	   /* Kender vi maden i forvejen? */
	   for (fm = 1 ; fm < felt->NumAnts ; fm++) {
	     if (mem[fm].status == LC_Status_RememberFood) {
	       if (mem[fm].xdest == x && mem[fm].ydest == y) {
		 mem[m].status = LC_Status_Base;
		 break;
	       }
	     }
	   }
	 }
	 if (mem[m].status == LC_Status_FoundFood) {
	   mem[m].status = LC_Status_RememberFood;
	 }
	 break;
      default:
	mem[m].status = LC_Status_Base;
	{
	  int ax = absv(x);
	  int ay = absv(y);
	  mem->xrange += ax*(ax*100-mem->xrange)/(ax+ay)*LCRangeAdapt/100;
	  mem->yrange += ay*(ay*100-mem->yrange)/(ax+ay)*LCRangeAdapt/100;
	}
      }
   }
   //|

   ///  Giv myrerne ordrer!
   for (m = 1 ; m < felt->NumAnts ; m++) {
     if (mem[m].status == LC_Status_Base) {
       /* Myren er til rådighed. */

       /* Find den nærmeste mad */
       x = 1000000;
       y = 1000000;
       fn = 0;
       fval = val(x,y);
       bfm = 0;
       for (fm = 1 ; fm < felt->NumAnts ; fm++) {
	 if (mem[fm].status == LC_Status_RememberFood && mem[fm].num > 0) {
	   mfval = val(mem[fm].xdest,mem[fm].ydest);
	   if (mfval < fval) {
	     x = mem[fm].xdest;
	     y = mem[fm].ydest;
	     fn = mem[fm].num;
	     fval = mfval;
	     bfm = fm;
	   }
	 }
       }
       
       if (fn == 0) {
	 /* Find en retning at sende myren i. */
	 int routetemp = mem->route;
	 int bit;
	 x = 0x8000; y = 0x0064;
	 for (bit = 0 ; bit < 10 ; bit++) {
	   if (routetemp&1) {
	     int nx = x*costab[bit]-y*sintab[bit];
	     int ny = y*costab[bit]+x*sintab[bit];
	     x = nx >> 15; y = ny >> 15;
	   }
	   routetemp >>= 1;
	 }
	 x = x*(mem->xrange*(100+LCMarginPercent)/10000+LCMarginExtra)/0x8000;
	 y = y*(mem->yrange*(100+LCMarginPercent)/10000+LCMarginExtra)/0x8000;
	 mem->route++;
       } else {
	 /* Bed myren om at hente noget mad! */
	 x = mem[bfm].xdest;
	 y = mem[bfm].ydest;
	 mem[bfm].num--;
       }
       /* Giv myren ordren! */
       mem[m].xdest = x;
       mem[m].ydest = y;
       mem[m].status = LC_Status_Searching;
       mem[m].num = 0;
     }
   
   }
   //|

   return 16;
}
//|


DefineAnt(LightCore3,"LightCore3",LightCore3,struct LC3Brain)
