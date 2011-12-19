/*
   Denne myre er ment som et studie i ulæslig CSource
   At en så samtidig virker, må anses for at være et mirakel.

$VER: Hellig myre(Sun Jihad) V0.0.1.3
Cleanup done

Hvad havde vandet følt, da det opdagede det var blevet til vin? Overraskelse, svarede han dæmpet. Vantro. Angst Afsky. Men til sidst, efte svære overvejelser, godtagelse.

*/

#include "Myre.h"

void SunPrint(const char *text, ...) {}
// #define SunPrint printf

#define Abs(X) (X >= 0) ? (X) : -(X)
#define ServerOfGod       42    // (Se aske, se og smil -:)
#define LimitToAdd        8
#define MediumLimitToAdd  16
#define LargeLimitToAdd   24
#define ValueToAdd        1
#define StartAttackAt     5000
#define AttackRate        2

#undef AntFunc
#undef AntMem
#undef AntName

#define AntFunc Sun_Myre
#define AntMem struct SunData
#define AntName "Sun Jihad"
#define Const_GuardKillCount 1
#define Const_WarKillCount 30
#define Const_GuardTurns 100
#define Const_SearchForScout 1200
#define Const_SendScoutAt 300
#define BuildBaseTurns 400
#define NumStoreFoodTurns 400

#ifndef TRUE
#define TRUE 1
#define IDidDefineTRUE "At leve er at vide"
#endif

#ifndef FALSE
#define FALSE 0
#define IDidDefineFALSE "3.14159265355820974944"
#endif

#ifndef BOOL
#define BOOL short
#define IDidDefineBOOL "Qnx: For a better future"
#endif

enum { QueenOfCaerGwent=1,GetFood,Goto,None,RndGoto,Return,StoreFood,ReturnWithFood,BuildBase,Gvydion,MauraGuard,Scout,BuildNewBase,Attack };

struct SunData *Sun_FindMaura(struct SunData *M,struct SquareData *S);
void Sun_GetTaskFromMaura(struct SunData *Maura,struct SquareData *S,struct SunData *M);
void Sun_IAmMaura(struct SunData *);
BOOL Sun_MauraKnownFood(struct SquareData *S,struct SunData *M);
int Sun_ServeMaura(struct SunData *M,struct SquareData *S);
int Sun_GotoHandler(struct SunData *,short,short);
int Sun_ExistsFood(struct SquareData *S);
int Sun_ExistsEnemy(struct SunData *S,struct SquareData *M);
void Sun_IAmHome(struct SunData *M,struct SquareData *S);

u_long Sun_Random(int num, u_short *rvals);

struct SunData {
   // 24
   u_short random[2];
   short X,Y,TargetX,TargetY;
   short MaxLimit;
   short TurnsToReturn;
   short KnownFoodX,KnownFoodY;
   short BuildBaseAtX,BuildBaseAtY;

   // 8
   unsigned char MinLimit;
   char Task;
   char KnownFoodNum;
   char BaseRnd;

   char BaseStatus,ScoutCount;
   char Direction,CurrentFood;
};

#define Random(num) Sun_Random(num, M->random)

/// SunMyre
int Sun_Myre(struct SquareData *S, struct SunData *M)
{
   struct SunData *Maura;
   int Result;

   if(M->Task == 0)
   {
      if(!(Maura=Sun_FindMaura(M,S)))
         Sun_IAmMaura(M);
      else
         Sun_GetTaskFromMaura(Maura,S,M);
   }
   if(M->Task != ServerOfGod)
      Result=Sun_ServeMaura(M,S);
   else
   {
     int TurnCounter;
     M->X++;  // Turn counter :}
     TurnCounter=M->X;

      if(((TurnCounter%LimitToAdd) == 0) && TurnCounter <1000)
      {
	 M->MaxLimit += ValueToAdd; 
	 M->TurnsToReturn=M->MaxLimit;
      }

      else if(((TurnCounter%MediumLimitToAdd) == 0) && TurnCounter >2000 && TurnCounter < 3000)
      {
	 M->MaxLimit += ValueToAdd; 
	 M->TurnsToReturn=M->MaxLimit;
	 if(M->MaxLimit>250)
	    M->MaxLimit=250;
	 M->MinLimit++;
      }

      if(((TurnCounter%LargeLimitToAdd) == 0) && TurnCounter >3000)
      {
	 M->MaxLimit += ValueToAdd;
	 M->TurnsToReturn=M->MaxLimit/2 + Random(M->MaxLimit/2);

	 if(M->MaxLimit>250)
	    M->MaxLimit=250;
      }

      if(TurnCounter==Const_SendScoutAt*2)
      {
        M->ScoutCount=0;
      }

      if(TurnCounter==Const_SearchForScout)	// Search for our scouts
      {
        int BaseDist=100;
        int MaxVal=2,MaxIndex=-1;
        // Remember that M=Maura!
        int i;
        int ScoutCount[4]={ 0,0,0,0 };

	M->ScoutCount=0;  

        for(i=1;i!=S[0].NumAnts;i++)
	{
	  if(M[i].Task==Scout)
	  {
	    ScoutCount[ (M[i].ScoutCount/2)]++;
	    M[i].Task=0;
	  }
	}

	// Search for best build.
	for(i=0;i!=4;i++)
	{
	  if(ScoutCount[i] > MaxVal)
	  {
	    MaxVal=ScoutCount[i];
	    MaxIndex=i;
	  }
 	}

	M->BaseRnd=1;           // Meaning: We are building a new base.
	switch(MaxIndex)
	{
	  case 0:
	    M->BuildBaseAtX=BaseDist;
	    M->BuildBaseAtY=-BaseDist;
	    break;
	  case 1:
	    M->BuildBaseAtX=BaseDist;
	    M->BuildBaseAtY=BaseDist;
	    break;
	  case 2:
	    M->BuildBaseAtX=-BaseDist;
	    M->BuildBaseAtY=BaseDist;
	    break;
	  case 3:
	    M->BuildBaseAtX=-BaseDist;
	    M->BuildBaseAtY=-BaseDist;
	    break;
	  default:
	    M->BuildBaseAtX=0;
	    M->BuildBaseAtY=0;
	    M->BaseRnd=0;    // No now base this time.
	}
	SunPrint("Building base at %d,%d\n",M->BuildBaseAtX,M->BuildBaseAtY);
      }
      else if(M->BuildBaseAtX && TurnCounter > Const_SearchForScout+BuildBaseTurns)
      {
        // Stop building base now
        M->BuildBaseAtX=0;
	M->BuildBaseAtY=0;
      }

      return(0);
   }

   switch(Result&7)
   {
      case 0: break;

      case 1:
         if(S[1].NumAnts==MaxSquareAnts)
	 {
	   SunPrint("Overflow at 1\n");
	   Result=0;
	   M->TargetX=-(Random(10)+3);
	   M->TargetY=-(Random(10)+3);
	   M->Task=Goto;
	   M->TurnsToReturn=30;
	 }
	 else
           M->X++;break;
      case 2:
         if(S[2].NumAnts==MaxSquareAnts)
	 {
	   SunPrint("Overflow at 2\n");
	   Result=0;
	   M->TargetX=Random(10)+3;
	   M->TargetY=-(Random(10)+3);
	   M->Task=Goto;
	   M->TurnsToReturn=30;
	 }
	 else
         M->Y++;break;
      case 3:
         if(S[3].NumAnts==MaxSquareAnts)
	 {
	   Result=0;
	   SunPrint("Overflow at 3\n");
	   M->TargetX=Random(10)+3;
	   M->TargetY=Random(10)+3;
	   M->Task=Goto;
	   M->TurnsToReturn=30;
	 }
	 else
         M->X--;break;
      case 4:
         if(S[4].NumAnts==MaxSquareAnts)
	 {
	   SunPrint("Overflow at 4\n");
	   Result=0;
	   M->TargetX=-(Random(10)+3);
	   M->TargetY=Random(10)+3;
	   M->Task=Goto;
	   M->TurnsToReturn=30;
	 }
	 else
           M->Y--;break;
      case 5:
         SunPrint("case 5"); break;

      default:
         SunPrint("Unknown direction %d at %d,%d\n",Result,M->X,M->Y);Result=0;
	 Result=1;
   }

   return(Result);
}
//|
/// ServeMaura
int Sun_ServeMaura(struct SunData *M,struct SquareData *S)
{
   int i,Result=1;

   if(M->Task != ServerOfGod)
      if( (Result=Sun_ExistsEnemy(M,S)) )
      {
         if(M->Task != MauraGuard && M->Task != M->KnownFoodNum>0 && S[0].NumFood==0)
         {
           M->TurnsToReturn+=Const_GuardTurns;
           M->Task=Gvydion;
         }
         return(Result);
      }
      else ;
   else
      SunPrint("None here, pleace/n");

   if(M->Task == RndGoto || M->Task == Goto || M->Task == Return)
   {
      if(M->KnownFoodNum == 0)
      {
         Result=Sun_ExistsFood(S);
         if(Result == 0)
         {
            M->KnownFoodNum=S->NumFood-S->NumAnts;
            M->KnownFoodX=M->X;
            M->KnownFoodY=M->Y;
            M->TargetX=M->TargetY=0;
            M->Task=Return;
            return(Sun_GotoHandler(M,0,0)+8);
         }
         else if(Result != -1)
            return(Result);
      }
      // Search for other ants and ask about their killing.
      for(i=1;i!=S[0].NumAnts;i++)
      {
        if(M[i].Task==Gvydion &&  S[0].NumAnts<=3 && S[0].NumFood==0) //  && M->X && M->Y)	// M[i].KillCount >= Const_GuardKillCount &&
	{
	  int j;
	  M->Task=Gvydion;

	  for(j=0;j!=S[0].NumAnts;j++)
	  {
	    if(M[j].Task==Gvydion)
	      M[j].TurnsToReturn+=Const_GuardTurns;
	  }
	}
      }
   }

   switch(M->Task)
   {
     case ReturnWithFood:
     Result=Sun_GotoHandler(M,M->BuildBaseAtX,M->BuildBaseAtY);

     if(Result==0) {
       if(S[0].Base) {
	 if(M->X || M->Y) {
	   memset(M,0,sizeof(struct SunData));
	 }
	 Sun_IAmHome(M,S); 
	 Result=Sun_ServeMaura(M,S); 
       }
     }
     else
       Result+=8;
     return(Result);
      case Goto:
         Result=Sun_GotoHandler(M,M->TargetX,M->TargetY);

         if(!Result)
         {
            M->Task=RndGoto;
            if(M->X)
               if(Random(2))
                  { M->BaseRnd=4 ; return(2); }
               else
                  { M->BaseRnd=2 ; return(4); }
            else
               if(Random(2))
                  { M->BaseRnd=3 ; return(1); }
               else
                  { M->BaseRnd=1 ; return(3); }

         }
         else
	    return(Result);
      case RndGoto:
      if(M->TurnsToReturn-- <=0)
      {
         M->Task=Return;
//	 M->KillCount=0;
         return(Sun_GotoHandler(M,0,0));
      }
      if(M->BaseRnd==0)
      {
        Result=Random((4));
         switch(Result)
         {
            case 0: Result=3;break;
            case 1: Result=4;break;
            case 2: Result=1;break;
            case 3: Result=2;break;
         }
      }
      else
        Result=M->BaseRnd;
      return(Result);

      case Return:
	  if(M->BaseStatus==0 && M->BuildBaseAtX)
	  {
	    M->BuildBaseAtX=M->BuildBaseAtY=0;
	  }

          Result=Sun_GotoHandler(M,0,0);

          if(!Result)
          { 
	    if(S[0].Base)
	    {
	      if(M->X || M->Y)
	        memset(M,0,sizeof(struct SunData));

	      Sun_IAmHome(M,S); 
	      Result=Sun_ServeMaura(M,S); 
	    }
	    else
	    {
      	      Result=8;
	    }
	  }
          return(Result+8);

      case GetFood:
         Result=Sun_GotoHandler(M,M->KnownFoodX,M->KnownFoodY);
         if(!Result)
         {
	    if(S[0].NumFood)
            {
	       if(M->BuildBaseAtX)
                 M->Task=BuildNewBase;
	       else
	         M->Task=Return;
               M->KnownFoodNum=0;
	       M->BaseStatus=1;
               return(Sun_GotoHandler(M,M->BuildBaseAtX,M->BuildBaseAtY)+8);               
            }
            else
	    {
	       // Someone have taken the food away.

	      switch(Random(4)) {
	      case 0:  // Go near the base
	      case 1:
	      case 3:
		if(M->Y > 0) {
		  M->TargetX = M->X;
		  M->TargetY = M->Y - Random(10);
		}
	        else {
		  M->TargetX = M->X;
		  M->TargetY = M->Y + Random(10);
		}
		M->TurnsToReturn=3;
		M->Task=Goto;
		break;
	      case 2:
		M->TurnsToReturn=150;
		M->Task=RndGoto;
		M->BaseRnd=0;
		break;
              }
	    }
          }
	 return(Result);

      case Gvydion:
        if(M->TurnsToReturn-- <= 0 )
           { M->Task=Return; return(Sun_GotoHandler(M,0,0)); }
       else
          return(0);

     case MauraGuard:
        if(M->X == 0 && M->Y==0)
           return(0);
        else
           return(Sun_GotoHandler(M,0,0));
 
     case BuildBase:
       if(S->Base)
         Sun_IAmHome(M,S);

       return(16); 

     case None:
        SunPrint("None\n");
     case ServerOfGod:
        return(0);
     case Scout:
       {
         Result=Sun_GotoHandler(M,M->TargetX,M->TargetY);
	 if(!Result)
	 {
	   M->TargetX=M->TargetY=0;
	   Result=Sun_GotoHandler(M,0,0);
	 }
         return(Result);     
       }
     break;

     case BuildNewBase:
       Result=Sun_GotoHandler(M,M->BuildBaseAtX,M->BuildBaseAtY);
       if(Result==0)
       {
         M->Task=BuildBase;
	 M->BuildBaseAtX=M->BuildBaseAtY=0;
	 M->X=M->Y=0;
       }
       return(Result+8);
       break;

     case StoreFood:
       Result=Sun_GotoHandler(M,0,0);
       if(M->TurnsToReturn-- > 0) {
	 if(S[0].NumAnts < MaxSquareAnts-10)
	   return(0);
	 else
	   SunPrint("Error, ant overflow\n");
       }
       M->KnownFoodNum=0;
       M->Task=0;
       return(0);
	 break;

#define GridSize 2

     case Attack:
       Result=Sun_GotoHandler(M,M->TargetX,M->TargetY);
       if(!Result) {
	 if(S[0].NumAnts>1) {

	   if(abs(M->X) >= 200) {
	     M->TargetY-=GridSize; 
	   }
	   else {
	     M->TargetX+= M->X>0 ? GridSize : -GridSize;
	     //	     M->TargetY+= M->Y<0 ? GridSize : -GridSize;
	   }
	     /*
	   if(M->TargetX>0) {
	     // Ind mod midten fra hoejre.
	     M->TargetX-=GridSize;
	     M->TargetY+=GridSize * (M->Y>0?1:-1);
	   }
	   else if(M->TargetX<0) {
	     // Ind mod midten fra venstre.
	     M->TargetX+=GridSize;
	     M->TargetY+=GridSize * (M->Y>0?1:-1);
	   }
	   else {
	     // Vi har ramt midten.
	     M->TargetY+=GridSize * (M->Y>0?1:-1);
	   }
	     */
	   /*
	   if(Random(2)!=1)
	     M->TargetX+=GridSize;
	   else
	     M->TargetY-=GridSize;
	   */
	   Result=Sun_GotoHandler(M,M->TargetX,M->TargetY);
	 }
	 else {
	   M->Task=Gvydion;
	   M->TurnsToReturn=Random(400)+Random(400);
	 }
       }
       break;
   default:
     SunPrint("Yet another default Task %d \n",M->Task);
     break;
   }
   return(Result);
}
//|
/// IAmHome
void Sun_IAmHome(struct SunData *M,struct SquareData *S)
{
   struct SunData *Maura;
   M->X=M->Y=0;
   M->Task=0;

   M->BuildBaseAtX=M->BuildBaseAtY=0;

   Maura=Sun_FindMaura(M,S);
   if(!Maura)
   {
      M->Task=BuildBase;
      return;
   }

   // Stay home as long as i know about food.
   if(M->KnownFoodNum > 0)
   {
     int i;
     // Do other ants know that this food exists?
     for(i=1;i!=S[0].NumAnts;i++)
     {
       if(M[i].Task==StoreFood && M[i].KnownFoodX==M->KnownFoodX && M[i].KnownFoodY==M->KnownFoodY)
       {
         M[i].KnownFoodNum--;
	 Sun_GetTaskFromMaura(Maura,S,M);
	 return;
       }
     }

     M->Task=StoreFood;
     M->TurnsToReturn=NumStoreFoodTurns;
     return;
   }
   Sun_GetTaskFromMaura(Maura,S,M);

}
//|
/// GetTaskFromMaura
void Sun_GetTaskFromMaura(struct SunData *Maura,struct SquareData *S,struct SunData *M)
{
   const int Const_MinLimit=100;
   int Limit,TargetSize;
   int MinLimit,MaxLimit,TurnsToReturn;
   int TurnCounter=Maura->X;

   M->BuildBaseAtX=0;
   M->BuildBaseAtY=0;

   if(TurnCounter > Const_SendScoutAt && Maura->ScoutCount<8)
   {
     const int Pos1=210-100;
     const int Pos2=190-100;

     switch(Maura->ScoutCount)
     {
       case 0:
         M->TargetX=Pos1;
	 M->TargetY=-Pos2;
	 break;
       case 1:
         M->TargetX=Pos2;
	 M->TargetY=-Pos1;
	 break;
       case 2:
         M->TargetX=Pos1;
	 M->TargetY=Pos2;
	 break;
       case 3:
         M->TargetX=Pos2;
	 M->TargetY=Pos1;
	 break;
       case 4:
         M->TargetX=-Pos1;
	 M->TargetY=Pos2;
	 break;
       case 5:
         M->TargetX=-Pos2;
	 M->TargetY=Pos1;
	 break;
       case 6:
         M->TargetX=-Pos1;
	 M->TargetY=-Pos2;
	 break;
       case 7:
         M->TargetX=-Pos2;
	 M->TargetY=-Pos1;
	 break;
       default:
         SunPrint("ARG!");
     }
     M->Task=Scout;
     M->ScoutCount=Maura->ScoutCount++;
     return;
   }

   if(Sun_MauraKnownFood(S,M) )
   {
      M->BuildBaseAtX=Maura->BuildBaseAtX;
      M->BuildBaseAtY=Maura->BuildBaseAtY;
      M->Task=GetFood;
      return;
   }

   if(M->BuildBaseAtX || M->BuildBaseAtY)
     SunPrint("Panic print\n");

   if(TurnCounter > (StartAttackAt+1000*(Maura->BaseRnd)) && Random(AttackRate)==0) {
     int Faktor=Random(4);
     int ExtraOffsetX=TurnCounter-StartAttackAt;
     int OffsetY=110;

     if(TurnCounter > 7000)
       OffsetY+=50;


     ExtraOffsetX=0;

     M->Task=Attack;

     switch(Faktor) {
     case 0:
       M->TargetX=10;
       M->TargetY=-OffsetY;
       break;
     case 1:
       M->TargetX=10;
       M->TargetY=OffsetY;
       break;
     case 2:
       M->TargetX=-10;
       M->TargetY=OffsetY;
       break;

     case 3:
       M->TargetX=-10;
       M->TargetY=-OffsetY;
       break;
     }
   }
   else {
     M->Task=Goto;
     MinLimit=Maura->MinLimit;
     MaxLimit=Maura->MaxLimit;
     TurnsToReturn=Maura->TurnsToReturn;
     Limit=MaxLimit-MinLimit;
     TargetSize=Random(Limit);
     TargetSize += MinLimit;
     M->TurnsToReturn=MaxLimit;

     switch( ( ( Maura->Direction=Maura->Direction == 4 ? 1 : Maura->Direction+1)) ) {
     case 1:
       M->TargetX=TargetSize;
       break;

     case 2:
       M->TargetX=0;
       M->TargetY=TargetSize;
       break;

     case 3:
       M->TargetX= -TargetSize;
       M->TargetY= 0;
       break;

     case 4:
       M->TargetX=0;
       M->TargetY= -TargetSize;
       break;

     default:
       SunPrint("GetTaskFromMaura default %d\n",M->Direction);
       break;
     }
   }
}
//|
/// FindMaura
struct SunData *Sun_FindMaura(struct SunData *M,struct SquareData *S)
{
   int i;

   for(i=1;i!=S->NumAnts;i++)
      if( M[i].Task==ServerOfGod)
         return(&M[i]);

   return(0);
}
//|
/// IAmMaura
void Sun_IAmMaura(struct SunData *M)
{
   memset(M,0,sizeof(struct SunData));

   M->Task=ServerOfGod;
   M->MinLimit=2;
   M->MaxLimit=30;
   M->X=0;

   M->TurnsToReturn=M->MaxLimit;

   M->Direction=0;         // Unødvendige.
}
//|
/// MauraKnownFood
BOOL Sun_MauraKnownFood(struct SquareData *S,struct SunData *M)
{
   int i,MinDist=999,Index=0;

   for(i=1;i!=S[0].NumAnts;i++)
      if(M[i].Task==StoreFood && M[i].KnownFoodNum > 0)
      {
        int NewMinDist=abs(M[i].KnownFoodX) + abs(M[i].KnownFoodY);
	if(NewMinDist < MinDist)
	{
          Index=i;
	  MinDist=NewMinDist;
	}
      }

      if(MinDist!=999)
      {
         if( M[Index].KnownFoodNum--)
	 M[0].KnownFoodX=M[Index].KnownFoodX;
	 M[0].KnownFoodY=M[Index].KnownFoodY;
         return(TRUE);
      }
   return(FALSE);
}
//|
/// Rnd fra MyreKrig.c
u_long Sun_Random(int num, u_short *rvals ) 
{
  u_long *Tmp=(u_long *)rvals;
  u_long BattleSeed=*Tmp;
   u_long a = (BattleSeed = BattleSeed*(42*42-42/42+42)+7);
   *Tmp=BattleSeed;
   return ((a<<19)+(a>>13))%num;
}
//|

/// GotoHandler
int Sun_GotoHandler(struct SunData *M,short X,short Y)
{
   int Result=0;                    // =0 for debug only.

  int AbsX=abs(M->X-X);
  int AbsY=abs(M->Y-Y);
  int Dist=AbsX+AbsY;
  
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
//      else
//      printf("Panic\n");
    }
    else		// Move in Y direction
    {
      if(M->X > X)
        Result=3;
      else if(M->X < X)
        Result=1;
//      else
//        printf("Panic\n");
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
/*
   if(M->GotoDir = M->GotoDir ? 0 : 1)
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
   else
      if(M->Y != Y)
         if(M->Y > Y)
            Result=4;
         else
            Result=2;
      else
         if(M->X >X)
            Result=3;
         else if(M->X == X)
            Result=0;
         else
            Result=1;
*/
   return(Result);
}
//|
/// ExistsFood
int Sun_ExistsFood(struct SquareData *S)
{
   int i;

   if(S[0].NumFood)
      if(S[0].NumFood >= S[0].NumAnts)
         return(0);

   for(i=1;i!=5;i++)
      if(S[i].NumFood && S[i].NumAnts==0)
         return(i);

   return(-1);
}
//|
/// ExistsEnemy
int Sun_ExistsEnemy(struct SunData *S,struct SquareData *M)
{
   int i;

   for(i=1;i!=5;i++)
      if(M[i].NumAnts)
         if(M[i].Team != M[0].Team)
	 {
            int j;
//	    S->KillCount++;

	    for(j=0;j!=M[0].NumAnts;j++)
	      if(S[j].Task==Gvydion)
		S[j].TurnsToReturn+=Const_GuardTurns;


            return(i);
	 }
   return(0);
}

//|
/// Undef
#ifdef IDidDefineBOOL
#undef IDidDefineBOOL
#undef BOOL
#endif

#ifdef SunPrint
#undef SunPrint
#endif

#ifdef IDidDefineTRUE
#undef IDidDefineTRUE
#undef TRUE
#endif

#ifdef IDidDefineFALSE
#undef IDidDefineFALSE
#undef FALSE
#endif

#undef Abs
#undef ServerOfGod
#undef LimitToAdd
#undef ValueToAdd
//|

DefineAnt(SunMyre, "Sun myre#eeee22", Sun_Myre, struct SunData)

