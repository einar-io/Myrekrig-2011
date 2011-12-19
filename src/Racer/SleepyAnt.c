/*
 Denne myre er lavet en dag, hvor jeg ikke sov natten forinden. Deraf navnet og kvaliteten.

  Taktikken er:
    Alle myre leder efter mad. Naar de finder det 

*/

#include "Myre.h"


struct SData {
  u_short random[1];
  short X,Y,Dist;

  // The last 6 bit in direction is a target offset in the X direction.
  short TargetX,TargetY;
  char Misc;
  unsigned char Direction,Task;

};

u_long Sleepy_Random(int num, u_short *rvals );
int Sleep_GotoHandlerSearch(struct SData *M,short X,short Y);
int Sleep_GotoHandlerDirect(struct SData *M,short X,short Y);
int Sleep_GotoHandlerHomeWithFood(struct SData *M);
int Sleep_GotoHandlerLines(struct SData *M);
int Sleep_ExistsFood(struct SquareData *S,struct SData *M);
int Sleep_ExistsEnemy(struct SquareData *S,struct SData *M);
int Sleep_PerformTask(struct SquareData *S,struct SData *M); 
void Sleep_IAmHome(struct SquareData *S,struct SData *M);
void HomeWithFood(struct SData *M);
void SetTask_GoOut(struct SData *M);

#define Random(num) Sleepy_Random(num, M->random)

#define Min_Dist_Add 30
#define Start_Dist 20

#define StartAttack 10
#define MaxRadius 300
#define Attack_Rate 5

enum { Task_NewAnt=0,Task_GoOut=20,Task_GoHome,Task_GetFood,Task_HomeFoundFood=40,Task_HomeWithFood,
       Task_TellOtherAboutFood,Task_Guard,Task_Direct_Attack,Task_RealDirect_Attack,Task_Attack };

void SetTask_GoOut(struct SData *M) {
  M->Task=Task_GoOut;
  M->Misc=0;
  switch( (M->Direction&3)+1) {
  case 1:
    M->TargetX=M->Dist;
    M->TargetY=-Random(M->Dist) - Random(M->Dist/2);
    break;
  case 2:
    M->TargetX=Random(M->Dist) + Random(M->Dist/2);
    M->TargetY=M->Dist;
    break;
  case 3:
    M->TargetX=-M->Dist;
    M->TargetY=Random(M->Dist) + Random(M->Dist/2);
    break;
  case 4:
    M->TargetX=-Random(M->Dist) -Random(M->Dist/2);
    M->TargetY=-M->Dist;
    break; 
  }
  //  printf("Calling settask with dist %d and target %d,%d while standing at %d,%d\n",M->Dist,M->TargetX,M->TargetY,M->X,M->Y);
}

// Get task based on what OTHER ants here do.
void Sleep_GetTask(struct SquareData *S,struct SData *M) {
  int i,Dist,TmpDist,CurrentPos=0;
  int WarIndex=-1;

  if(M->Task==Task_NewAnt) {
    M->Dist=Start_Dist;
    M->Direction=Random(4);
    M->X=M->Y=0;
  }
  M->Dist=M->Dist > MaxRadius ? MaxRadius: M->Dist;

  Dist=9999;   // Distance to food

  // First find out if anyone know where there is food.
  for(i=1;i!=S[0].NumAnts;i++) {
    if(M[i].Task==Task_TellOtherAboutFood && M[i].Misc>0) {
      TmpDist=abs(M[i].X) + abs(M[i].Y);
      if(TmpDist < Dist) {
	Dist=TmpDist;
	CurrentPos=i;
      }
    }
    else if(WarIndex==-1 && M[i].Task==Task_RealDirect_Attack)
      WarIndex=i;
  }

  if(CurrentPos) {
    M->Task=Task_GetFood;
    M[CurrentPos].Misc--;
    M->TargetX=M[CurrentPos].TargetX;
    M->TargetY=M[CurrentPos].TargetY;
    return;
  }

  // Do we want to start a war?
  if(WarIndex >=0 && Random(Attack_Rate)==0) {
    M->TargetX=M[WarIndex].TargetX;
    M->TargetY=M[WarIndex].TargetY;
    M->Misc=M[WarIndex].Misc;
    M->Task=Task_Attack;
    //    printf("ATTACK!!! at %d,%d\n",M->TargetX,M->TargetY);
  }
  else {
    int Dist=M->Dist/3;
    SetTask_GoOut(M);

    if(Dist<Min_Dist_Add)
      Dist=Min_Dist_Add;

    M->Dist+=Dist;
  }
}

int Sleep_PerformTask(struct SquareData *S,struct SData *M) {
  int i,Result,Dist;

  if(M->Task!=Task_HomeFoundFood) {
    Result=Sleep_ExistsEnemy(S,M);
    if(Result>0) {
      if(M->Task==Task_Guard) {
	if(++M->Misc==StartAttack) {
	  //	  printf("Attack start at %d,%d. Direction %d\n",M->X,M->Y,M->Misc);
	  M->Task=Task_Direct_Attack;
	  M->TargetX=M->X;
	  M->TargetY=M->Y;
	  if(abs(M->X) > abs(M->Y)) {
	    if(M->X>0)
	      M->Misc=1;
	    else
	      M->Misc=3;
	  }
	  else if(M->Y>0)
	    M->Misc=2;
	  else
	    M->Misc=4;
	}
      }
      else {
	M->Task=Task_Guard;
	M->Misc=0;
      }
      return(Result);
    }
  }

  if(M->Task >= Task_GoOut && M->Task < Task_GetFood) {
    // Search for food
    Result=Sleep_ExistsFood(S,M);
    if(Result==0) {
      // Jeg staar oven paa noget mad.
      M->Task=Task_HomeFoundFood;
      M->TargetX=M->X;
      M->TargetY=M->Y;
      M->Misc=S[0].NumFood;
      Result=Sleep_PerformTask(S,M);
      return(Result);
    }
    else if(Result>0) {
      return(Result);
    }
  }

  //  Dir=(M->Direction&3)+1;

  switch(M->Task) {
  case Task_GoOut:
    Result=Sleep_GotoHandlerSearch(M,M->TargetX,M->TargetY);
    if(Result==0) {
      if(M->Dist==Start_Dist) {
	M->Task=Task_GoHome;
      }
      else {
	M->Task=Task_GoHome;
	break;
      }
      Result=Sleep_PerformTask(S,M);
    }
    break;
 
  case Task_GoHome:
    Result=Sleep_GotoHandlerSearch(M,0,0);
    if(Result==0) {
      Sleep_GetTask(S,M);
      Result=Sleep_PerformTask(S,M);
    }
    break;
  case Task_HomeWithFood:
    Result=Sleep_GotoHandlerLines(M)+8;
    if(Result==8) {
      Sleep_IAmHome(S,M);
      Result=Sleep_PerformTask(S,M);
    }
    break;

  case Task_HomeFoundFood:
    for(i=1;i!=S[0].NumAnts;i++) {
      if(M[i].Task >= Task_GoOut && M[i].Task < Task_GetFood) {
	// printf("Tell this ant about food at %d,%d\n",M[0].TargetX,M[0].TargetY);
	// Tell it where there is food.
	M[i].Task=Task_GetFood;
	M[i].TargetX=M[0].TargetX;
	M[i].TargetY=M[0].TargetY;
	if(--(M[0].Misc) <= 0) {
	  SetTask_GoOut(M);
	  Result=Sleep_PerformTask(S,M);
	  return(Result);
	}
      }
    }

    Result=Sleep_GotoHandlerLines(M)+8;
    if(Result==8) {
      Sleep_IAmHome(S,M);
      Result=Sleep_PerformTask(S,M);
    }
    break;
    
  case Task_TellOtherAboutFood:
    if(M->Misc>0) {
      Result=0;
    }
    else if(M->Misc==0) {
      M->Misc=-2;
      Result=0;
    }
    else if(M->Misc-- < -100) {
      Sleep_GetTask(S,M);
      M->Misc=0;
      Result=Sleep_PerformTask(S,M);
    }
    else
      Result=0;
    break;

  case Task_GetFood:
    Result=Sleep_GotoHandlerDirect(M,M->TargetX,M->TargetY);
    if(Result==0) {
      if(S[0].NumFood>0) {
	M->Task=Task_HomeWithFood;
	Result=Sleep_PerformTask(S,M);
      }
      else {
	SetTask_GoOut(M);
	Result=Sleep_PerformTask(S,M);
      }
    }
    break;

  case Task_Guard:
    Dist=abs(M->X) + abs(M->Y); // Just a temp var.
    Result=0;

    for(i=1;i!=S[0].NumAnts;i++)
      if(M[i].Dist > Dist) {
	M->Dist=-M->Dist;
	M->Task=Task_GoOut;
	Result=Sleep_PerformTask(S,M);
	//	Result=1;
	break;
      }
    break;

  case Task_Direct_Attack:
    Result=Sleep_GotoHandlerDirect(M,0,0);
    if(Result==0) {
      for(i=0;i!=S[0].NumAnts;i++) {
	if(M[i].Task==Task_RealDirect_Attack) {
	  //	  printf("An other ant is attacking\n");
	  SetTask_GoOut(M);
	  Result=Sleep_PerformTask(S,M);
	  return(Result);
	}
      }
      M->Task=Task_RealDirect_Attack;
      //      printf("Directing attack ok\n");
    }
    break;

  case Task_RealDirect_Attack:
    Result=0;
    break;

  case Task_Attack:
    Result=Sleep_GotoHandlerDirect(M,M->TargetX,M->TargetY);
    if(Result)
      return(Result);

    // We made it. Let's start the attack
#define Gridx 1
#define Gridy 2

    if(S[0].NumAnts>1) {
      // Move further
      switch(M->Misc) {
      case 0:
	M->TargetX+=Gridx;
	M->TargetY-=Gridy;
	break;
      case 1:
	M->TargetX+=Gridx;
	M->TargetY-=Gridy;
	break;
      case 2:
	M->TargetX-=Gridx;
	M->TargetY+=Gridy;
	break;
      case 3:
	M->TargetX-=Gridx;
	M->TargetY-=Gridy;
	break;
      default:
	Result=1;
	break;
      }
      Result=Sleep_GotoHandlerDirect(M,M->TargetX,M->TargetY);
    }
    else
      Result=0;
    break;


  default:
    //    printf("Unknown task %d\n",M->Task);
    Result=0;
  }
  return(Result);
}

void Sleep_IAmHome(struct SquareData *S,struct SData *M) {
  int i;

  if(M->Task==Task_HomeFoundFood) {
    // Find out if someone else already have found this food.
    for(i=1;i!=S[0].NumAnts;i++) {
      if(M[i].TargetX==M[0].TargetX && M[i].TargetY==M[i].TargetY) {
	Sleep_GetTask(S,M);
	return;
      }      
    }
    M->Task=Task_TellOtherAboutFood;
    //    M->Misc=S[0].NumFood;
  }
  else {
    Sleep_GetTask(S,M);
  }
}

int Sleepy(struct SquareData *S, struct SData *M) {
  int Result;
  int Weight=20;

  if(M->Task==Task_NewAnt) {
    Sleep_GetTask(S,M);
  }

  Result=Sleep_PerformTask(S,M);
  if(M->Task==Task_GoHome)
    Weight+=10;
  else if(M->Task==Task_HomeWithFood || M->Task==Task_HomeFoundFood)
    Weight-=10;
  else if(M->Task==Task_GoOut || M->Task==Task_GetFood)
    Weight-=20;

  switch(Result&7) {
  case 0: break;
  case 1:
    if(S[1].NumAnts>=MaxSquareAnts-Weight) {
      Result=0;
    }
    else
      M->X++;
    break;
  case 2:
    if(S[2].NumAnts>=MaxSquareAnts-Weight) {
      Result=0;
    }
    else
      M->Y++;
    break;
  case 3:
    if(S[3].NumAnts>=MaxSquareAnts-Weight) {
      Result=0;
    }
    else
      M->X--;
    break;
  case 4:
    if(S[4].NumAnts>=MaxSquareAnts-Weight) {
      Result=0;
    }
    else
      M->Y--;
    break;
    
  default:
    //    printf("Unknown direction %d at %d,%d\n",Result,M->X,M->Y);Result=0;
    break;
  }
 
  return(Result);
}
/// Rnd fra MyreKrig.c
u_long Sleepy_Random(int num, u_short *rvals ) 
{
  u_short *Tmp=(u_short *)rvals;
  u_short BattleSeed=*Tmp;
  u_long a = (BattleSeed = BattleSeed*(42*42-42/42+42)+7);

  if(num==0) {
    return(0);
  }
   *Tmp=BattleSeed;
   return ((a<<19)+(a>>13))%num;
}

//|
/// GotoHandler
int Sleep_GotoHandlerSearch(struct SData *M,short X,short Y)
{
  int Result=0;                    // =0 for debug only.

  int AbsX=abs((M->X-X));
  int AbsY=abs(M->Y-Y);
  
  if(AbsX==0 && AbsY==0)
    return(0);

  if(AbsX && AbsY)
  {
    if(Random(AbsY)/2 > Random(AbsX))
    {
      if(M->Y > Y)
        Result=4;
      else if(M->Y < Y)
        Result=2;
      //else
	//printf("Panic\n");
    }
    else		// Move in Y direction
    {
      if(M->X > X)
        Result=3;
      else if(M->X < X)
        Result=1;
      //else
        //printf("Panic\n");
    }
  }
  else
  {
    if(M->X != X)
      if(M->X > X)
        Result=3;
      else
        Result=1;
    else
      if(M->Y >Y)
        Result=4;
      else if(M->Y == Y)
        Result=0;
      else
        Result=2;
  }
  return(Result);
}


int Sleep_GotoHandlerDirect(struct SData *M,short X,short Y) {
   int Result=0;                    // =0 for debug only.

  int AbsX=abs(M->X-X);
  int AbsY=abs(M->Y-Y);
  
  if(AbsX==0 && AbsY==0)
    return(0);

  if(AbsX && AbsY)
  {
    if(Random(AbsY) > Random(AbsX))
    {
      if(M->Y > Y)
        Result=4;
      else if(M->Y < Y)
        Result=2;
      //else
	//printf("Panic\n");
    }
    else		// Move in Y direction
    {
      if(M->X > X)
        Result=3;
      else if(M->X < X)
        Result=1;
      //else
        //printf("Panic\n");
    }
  }
  else
  {
    if(M->X != X)
      if(M->X > X)
        Result=3;
      else
        Result=1;
    else
      if(M->Y >Y)
        Result=4;
      else if(M->Y == Y)
        Result=0;
      else
        Result=2;
  }
  return(Result);
}

int Sleep_GotoHandlerHomeWithFood(struct SData *M) {
  int Result=0;                    // =0 for debug only.

  if(M->X >0) {
    Result=3;
  }
  else if(M->X<0) {
    Result=1;
  }
  else {
    if(M->Y>0)
      Result=4;
    else if(M->Y<0)
      Result=2;
    else
      Result=0;
  }

  return(Result+8);
}

int Sleep_GotoHandlerLines(struct SData *M) {
  int Result=0;                    // =0 for debug only.

  if(abs(M->X) > abs(M->Y)) {
    // Flyt paa x aksen.

    if(M->X > 0)
      Result=3;
    else
      Result=1;
  }
  else {
    // Flyt paa y aksen(Eller vi er hvor vi skal vaere)
    if(M->Y > 0)
      Result=4;
    else if(M->Y < 0)
      Result=2;
    else
      Result=0;
  }
  return(Result);
}

//|
/// ExistsFood
int Sleep_ExistsFood(struct SquareData *S,struct SData *M)
{
   int i,FoodCount=0;

   if(M->X==0 && M->Y==0) {
     return(-1);
   }

   if(S[0].NumFood) {
     // Standing on the food. Find out if other are doing the same thing.
     for(i=1;i!=S[0].NumAnts;i++) {
       if(M[i].Task==Task_HomeFoundFood || M[i].Task==Task_HomeWithFood)
	 FoodCount++;
     }
     if(FoodCount >= S[0].NumAnts)
       return(-1);
     else
       return(0);
   }


   for(i=1;i!=5;i++)
     if(S[i].NumFood > S[i].NumAnts) // S[i].NumAnts)
         return(i);

   return(-1);
}
//|
/// ExistsEnemy

int Sleep_ExistsEnemy(struct SquareData *S,struct SData *M)
{
   int i;

   for(i=1;i!=5;i++)
     if(S[i].NumAnts)
       if(S[i].Team != S[0].Team)
	 return(i);
   return(0);
}

//|

#undef Random

DefineAnt(SleepyAnt, "SleepyAnt#666699", Sleepy, struct SData)
