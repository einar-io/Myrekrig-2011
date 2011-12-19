/* MyreKrig officiel version 2.4.7 (26.06.03) */
/* Copyright (c) 1998-2002 Aske Simon Christensen */

/// Historie...
/*       (09.08.01): Version 2 påbegyndt. */
/* 2.0.0 (09.10.01): Første frigivelse af version 2. */
/* 2.0.1 (26.10.01): Tilpasning til C++ i header-filerne.
                     Default signed char i Makefile. */
/* 2.1.0 (24.11.01): Specialiserede hjerne-kopierings-funktioner.
                     NumTeams-afhængig default banestørrelse.
                     Intern myre-ram-håndtering isf. malloc/free. */
/* 2.1.1 (10.12.01): Deallokering af TeamDatas og GameTotals. */
/* 2.1.2 (02.01.02): Argumenter fortolkes altid decimalt. */
/* 2.1.3 (05.02.02): Hold 1 placeres altid i midten.
                     Flere forsøg på fornuftig baseplacering. */
/* 2.2.0 (09.04.02): Farver udfra titel eller eksplicit angivelse. */
/* 2.3.0 (16.04.02): Tilfældigt udvalg af myrerne til hver kamp.
                     Anderledes output.
		     Hyppigere tidstagning. */
/* 2.3.1 (17.04.02): Det først udvalgte hold var usynligt for de andre. DOH!
                     Småændringer i output. */
/* 2.3.2 (19.04.02): Nedskalering for at undgå overflow i DieAge og TimesRun.
                     Bedre default-størrelser. */
/* 2.3.3 (06.05.02): Nye definitioner af Performance of Prestige:
                     Performance = vict/((1-slow)*vict+slow)
		     Prestige = vict*4/MemSize
                     Tidsforbruget af clock() fratrækkes de målte tider.
                     Diverse output-ændringer. */
/* 2.3.4 (13.05.02): Nye definitioner af Performance of Prestige:
                     Performance = vict/slow
		     Prestige = vict*(gennemsnitlig MemSize)/MemSize
		     (tilbage til de gamle, blot skaleret anderledes) */
/* 2.3.5 (14.05.02): Atter nye definitioner af Performance of Prestige:
                     Performance = vict*(median Time)/Time
		     Prestige = vict*(median MemSize)/MemSize */
/* 2.3.6 (16.05.02): Bogstav på kampens vinderangivelse. */
/* 2.4.0 (27.05.02): Myrerækkefølge korregeret til den dokumenterede.
		     Fornuftige vinderkriterier ved BattleSize på 1.
		     Rettelse i output ved max BattleSize < 5.
		     Det er nu muligt at have myrehjerner på under 4 bytes. */
/* 2.4.1 (31.05.02): WatchTeam-parameter indført.
                     Optælling af NumBattles ved overspringning. */
/* 2.4.2 (10.06.02): Optælling af NumBattles fjernet.
                     NumBattles default ændret til 100. */
/* 2.4.3 (17.06.02): Hjernekopiering medtages ikke længere i tidsforbrug. */
/* 2.4.4 (19.08.02): Diverse variable forstørret til longs. */
/* 2.4.5 (28.02.03): Hjernestørrelse 0 tæller som 0.1 ved udregning af prestige.
                     Tidsforbrug angives i hele nanosekunder med minimum 1. */
/* 2.4.6 (07.03.03): Turtæller i argument "_____". Kan ses med ps.
                     BattleSize-grænser rettes til hvis de er ugyldige. */
/* 2.4.7 (26.06.03): Tættere tidstagning. Korrekt madejerskab. */
//|

/// Konstanter og makroer...
#define ZeroMemFactor 10
#define MapSizeMin 100
#define MapSizeMax 200
#define TimeRate 10
#define LastFoodMem 42
#define BasePlaceTries 10000
//|
/// Includes...
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "MyreKrig.h"
//|
/// Prototyper...
/* Initialisering */
void ParseArgs(int argc, char *argv[]);
bool GameInit(void);
void BattleInit(void);
void BattleExit(void);
void GameExit(void);

/* Simulering */
void DoTurn(void);
int RunAnt(struct AntData *ant);
void DoAction(struct AntData *ant, int action);
void PlaceFood(void);
void FoodOwnTouch(struct SquareData *sd, int factor);
bool TermCheck(void);

/* Myrehåndtering */
void NewAnt(int x, int y);
void KillAnt(struct AntData *ant, int team);

/* Output */
void PrintGameBegin(void);
void PrintBattleBegin(void);
void PrintBattleResult(int tc);
void PrintGameResult(int br);
u_long ScaleDiv(u_long, u_long, u_long);
void ScaleToSum(u_long *array, u_long num, u_long sum);

/* Diverse */
u_long Random(u_long num);
//|
/// Parameter variable (defaults)...
struct Limits Args = {
   { 0,0,15,10,10,5,10000,20000,75,60,0 },
   { 0,0,40,50,30,20,10000,20000,75,60,0 },
   100,0,0
};

struct Parameters Used,Max;
//|
/// Interne globale variable...
struct SquareData *MapDatas;
struct SquareAnts *MapAnts;
struct AntData **AntList;
struct TeamData *TeamDatas;
struct GameTotal *GameTotals;
struct AntData *AntFreeList;
u_char *BattleTeams, *TeamIndex;
u_char *TeamShuffles;
u_char *AntSpace, *AntTemp;
u_long *pgr_vict, *pgr_base, *pgr_time, *pgr_comb, *pgr_slow, *pgr_size, *pgr_perf, *pgr_pres;
u_short *x,*y, *bestx,*besty;
u_char *taken;
bool skipped;
u_long NumBorn, NumAnts, TurnAnts, NumFood, NumBases, BasesBuilt, CurrentTurn;
u_short Winner, BattleCount, LastFoodIndex, NumBattlesSwap = 0;
u_short NumTeams, LargestMem;
u_long MaxAnts, AntMem;
u_long BattleSeed;

char *TurnString = NULL;
char CrashTeam[13] = "CrashTeam:  ";
//|

static u_long DetermineParameter(u_long mmin, u_long mmax)
{
	return mmin + Random(mmax-mmin+1);
}

static void SetupBattleParameters(struct Parameters *Used, struct Parameters *Min, struct Parameters *Max)
{
	//for(p = 0 ; p < sizeof(struct Parameters)/4 ; p++) {
	//int min = ((u_long *)&Args.Min)[p], max = ((u_long *)&Args.Max)[p];
	//((u_long *)&Used)[p] = min + Random(max-min+1);
	//}
	Used->MapWidth = DetermineParameter(Min->MapWidth, Max->MapWidth);
	Used->MapHeight = DetermineParameter(Min->MapHeight, Max->MapHeight);
	Used->StartAnts = DetermineParameter(Min->StartAnts, Max->StartAnts);
	Used->NewFoodSpace = DetermineParameter(Min->NewFoodSpace, Max->NewFoodSpace);
	Used->NewFoodMin = DetermineParameter(Min->NewFoodMin, Max->NewFoodMin);
	Used->NewFoodDiff = DetermineParameter(Min->NewFoodDiff, Max->NewFoodDiff);
	Used->HalfTimeTurn = DetermineParameter(Min->HalfTimeTurn, Max->HalfTimeTurn);
	Used->TimeOutTurn = DetermineParameter(Min->TimeOutTurn, Max->TimeOutTurn);
	Used->WinPercent = DetermineParameter(Min->WinPercent, Max->WinPercent);
	Used->HalfTimePercent = DetermineParameter(Min->HalfTimePercent, Max->HalfTimePercent);
	Used->BattleSize = DetermineParameter(Min->BattleSize, Max->BattleSize);

}

/* main() indeholder den yderste struktur.
   setTitle() sætter titel og farve på en myre udfra det angivne navn
   ParseArgs() læser argumenter og bruger dem.
   TermCheck() afgører om spillet er slut.
   PrintGameBegin() udskriver generel info om det aktuelle spil.
   PrintBattleBegin() udskriver kampens nummer og randomseed.
   PrintBattleResult() udskriver information efter en kamp.
   PrintGameResult() udskriver samlet statistik ved spillets slutning.
   ScaleDiv() returnerer a*scale/b, eller 0.
   ---------------------------------------------------------- */
/// int  main(int argc, char *argv[])
int main(int argc, char *argv[]) {
   int tc, sc = 0, br = 0, n;

   InitTeams();
   Args.GameSeed = time(0);
   ParseArgs(argc,argv);

   Max = Args.Max;
   Max.MapWidth = (Max.MapWidth+63)&-64;
   Max.MapHeight = (Max.MapHeight+63)&-64;

   skipped = false;

   PrintGameBegin();
   if(GameInit() && SysGameInit(argc, argv)) {
      for(BattleCount = 0 ; BattleCount < Args.NumBattles ; BattleCount++) {
	 /* Hold evt. øje med det angivne hold. */
         int WatchTeamSeen = (Args.WatchTeam == 0);

         /* Sæt parametre op til kampen. */
         BattleSeed = Args.GameSeed;
		 SetupBattleParameters(&Used, &Args.Min, &Args.Max);
         Used.MapWidth = (Used.MapWidth+63)&-64;
         Used.MapHeight = (Used.MapHeight+63)&-64;

	 /* Udvælg hold */
	 for(n = 0 ; n < Used.BattleSize ; n++) {
		 u_long t;
	    int f,n2;
	    do {
	       t = Random(NumTeams)+1;
	       f = 0;
	       for (n2 = 0 ; n2 < n ; n2++) {
		  if (BattleTeams[n2] == t) {
		     f = 1;
		  }
	       }
	    } while (f);
	    if (Args.WatchTeam == t) WatchTeamSeen = 1;
	    BattleTeams[n] = (u_char) t;
	    TeamIndex[t] = (u_char) n;
	 }

	 if (WatchTeamSeen) {
	    PrintBattleBegin();
	    if(SysBattleInit()) {
	       BattleInit();
	       do {
		  DoTurn();
		  if (TurnString) {
		     snprintf(TurnString, 5+1, "%5ld", CurrentTurn);
		  }
		  SysDrawMap();
		  tc = TermCheck();
		  sc = SysCheck();
		  if(sc == SYS_MAKELAST) {
		     if(NumBattlesSwap == 0) {
		        NumBattlesSwap = (u_short) Args.NumBattles;
			Args.NumBattles = BattleCount+1;
		     } else {
		        Args.NumBattles = NumBattlesSwap;
			NumBattlesSwap = 0;
		     }
		  }
	       } while(tc == TERM_CONTINUE && (sc == SYS_CONTINUE || sc == SYS_MAKELAST || sc > SYS_MAX));
	       skipped = false;
	       switch(sc) {
	       case SYS_SKIP:
		  skipped = true;
		  break;
	       case SYS_INTERRUPT:
		  tc = 3;
		  break;
	       case SYS_EXIT:
		  Args.NumBattles = BattleCount--;
		  br = 1;
		  break;
	       case SYS_RESTART:
		  BattleCount--;
		  break;
	       }
	    } else {
	       tc = TERM_ERROR;
	    }
	    BattleExit();
	    SysBattleExit();
	    if(tc) PrintBattleResult(tc);
	    fflush(stdout);
	 } else {
            sc = SYS_SKIP;
	 }
	 if(sc != SYS_RESTART) Args.GameSeed = Args.GameSeed*123456789+12345;
      }
   }
   SysGameExit();
   PrintGameResult(br);
   GameExit();
   return 0;
}
//|
/// void setTitle(struct TeamData *td, char *title)
void setTitle(struct TeamData *td, char *title) {
  char *klaf = strchr(title, '#');
  if (klaf) {
    strncpy(td->Title, title, klaf-title);
    td->Color = strtoul(klaf+1, NULL, 16);
  } else {
    int col,i,fac,l;
    strncpy(td->Title, title, 10);
    fac = 424242;
    do {
      col = 0;
      for (i = 0 ; i < strlen(title) ; i++) {
	col = (col>>5)+col*fac+title[i];
      }
      fac++;
      l = (((col>>16)&0xff)*2+((col>>8)&0xff)*3+((col>>0)&0xff));
    } while (l < 500 || l > 1200);
    td->Color = col;
  }
}
//|
/// void ParseArgs(int argc, char *argv[])
void ParseArgs(int argc, char *argv[]) {
   int i;
   u_long val;
   char *pos, *param;

   param = "whafmdtopebWHAFMDTOPEBnsz";
   for(i = 1 ; i < argc ; i++) {
      if(strcmp(argv[i],"_____") == 0) {
	 TurnString = argv[i];
      } else if(argv[i][0] != '-') {
         val = strtoul(&argv[i][1],0,10);
         /* Sæt parametren */
         if((pos = strchr(param,argv[i][0])) != 0) {
            ((u_long *)&Args)[pos-param] = val;
            /* Hvis minimum, sæt også maximum */
            if((pos = strchr(param,argv[i][0]-32)) != 0) {
               ((u_long *)&Args)[pos-param] = val;
             }
         } else {
            printf(
                   "Commandline help:\n"
                   "\n"
                   "Arg  Variable\n"
                   "--------------------\n"
                   "w/W: MapWidth\n"
                   "h/H: MapHeight\n"
                   "a/A: StartAnts\n"
                   "f/F: NewFoodSpace\n"
                   "m/M: NewFoodMin\n"
                   "d/D: NewFoodDiff\n"
                   "t/T: HalfTimeTurn\n"
                   "o/O: TimeOutTurn\n"
                   "p/P: WinPercent\n"
                   "e/E: HalfTimePercent\n"
		   "b/B: BattleSize\n"
                   "n:   NumBattles\n"
                   "s:   RandomSeed\n"
		   "z:   WatchTeam\n"
                   "\n");
            exit(0);
         }
      }
   }

   /* Ret BattleSize til udfra antal myrehold. */
   if (!Args.Max.BattleSize || Args.Max.BattleSize > NumTeams) Args.Max.BattleSize = NumTeams;
   if (!Args.Min.BattleSize || Args.Min.BattleSize > Args.Max.BattleSize) Args.Min.BattleSize = Args.Max.BattleSize;
   /* Initialiser størrelser udfra antal BattleSize. */
   for (val = 0 ; val*val < (Args.Min.BattleSize+Args.Max.BattleSize)/2*MapSizeMax*MapSizeMax ; val++);
   if (!Args.Min.MapWidth) Args.Min.MapWidth = val*MapSizeMin/MapSizeMax;
   if (!Args.Min.MapHeight) Args.Min.MapHeight = val*MapSizeMin/MapSizeMax;
   if (!Args.Max.MapWidth) Args.Max.MapWidth = val;
   if (!Args.Max.MapHeight) Args.Max.MapHeight = val;

}
//|
/// bool TermCheck()
bool TermCheck(void) {
   if(Used.BattleSize == 1) {
      Winner = BattleTeams[0];
      return (NumFood == 0 ? CurrentTurn >= Used.HalfTimeTurn ? TERM_HALFTIME : TERM_WIN :
	      CurrentTurn >= Used.TimeOutTurn ? TERM_TIMEOUT : TERM_CONTINUE);
   } else {
      int n;
	  u_long val, val1 = 0, val2 = 0;
      for(n = 0 ; n < Used.BattleSize ; n++) {
	 int t = BattleTeams[n];
	 val = TeamDatas[t].NumAnts+BaseValue*TeamDatas[t].NumBases;
	 if(val > val1) {
	    Winner = t;
	    val2 = val1;
	    val1 = val;
	 } else if(val > val2) {
           val2 = val;
	 }
      }
      return (CurrentTurn >= Used.TimeOutTurn ? TERM_TIMEOUT :
	      CurrentTurn >= Used.HalfTimeTurn &&
	      val1*100 >= (val1+val2)*Used.HalfTimePercent ? TERM_HALFTIME :
	      val1*100 >= (val1+val2)*Used.WinPercent ? TERM_WIN : TERM_CONTINUE);
   }
}
//|
/// void PrintGameBegin()
void PrintGameBegin(void) {
   int t;
   printf("MyreKrig version 2.4.7 (26.06.03)\n\n");

   printf("NumTeams:       %7d\n",NumTeams);
   printf("NewBaseAnts:    %7d\n",NewBaseAnts);
   printf("NewBaseFood:  %9d\n\n",NewBaseFood);

   printf("                MinVal MaxVal\n");
   printf("MapWidth:       %6ld %6ld\n", Args.Min.MapWidth,        Args.Max.MapWidth);
   printf("MapHeight:      %6ld %6ld\n", Args.Min.MapHeight,       Args.Max.MapHeight);
   printf("StartAnts:      %6ld %6ld\n", Args.Min.StartAnts,       Args.Max.StartAnts);
   printf("NewFoodSpace:   %6ld %6ld\n", Args.Min.NewFoodSpace,    Args.Max.NewFoodSpace);
   printf("NewFoodMin:     %6ld %6ld\n", Args.Min.NewFoodMin,      Args.Max.NewFoodMin);
   printf("NewFoodDiff:    %6ld %6ld\n", Args.Min.NewFoodDiff,     Args.Max.NewFoodDiff);
   printf("HalfTimeTurn:   %6ld %6ld\n", Args.Min.HalfTimeTurn,    Args.Max.HalfTimeTurn);
   printf("TimeOutTurn:    %6ld %6ld\n", Args.Min.TimeOutTurn,     Args.Max.TimeOutTurn);
   printf("WinPercent:     %6ld %6ld\n", Args.Min.WinPercent,      Args.Max.WinPercent);
   printf("HalfTimePercent:%6ld %6ld\n", Args.Min.HalfTimePercent, Args.Max.HalfTimePercent);
   printf("BattleSize:     %6ld %6ld\n", Args.Min.BattleSize,      Args.Max.BattleSize);

   printf("NumBattles:     %6ld\n",       Args.NumBattles);
   printf("RandomSeed: %10lu\n\n",        Args.GameSeed);

   printf("Team letters and ant memory sizes:\n");
   for(t = 1 ; t <= NumTeams ; t++) {
      printf("%c %-10s %4d\n", '@'+t, TeamDatas[t].Title, TeamDatas[t].MemSize);
   }

   printf("\nBatt Randomseed Widt Heig Ant Spc Min Dif %-*s Turns T Winner\n", (int)Args.Max.BattleSize, "Teams");
}
//|
/// void PrintBattleBegin()
void PrintBattleBegin(void) {
   int n;
   printf("%s%4d %10lu %4ld %4ld %3ld %3ld %3ld %3ld ", (skipped ? "\x0d" : ""),
      BattleCount+1, Args.GameSeed,
      Used.MapWidth, Used.MapHeight, Used.StartAnts,
      Used.NewFoodSpace, Used.NewFoodMin, Used.NewFoodDiff);
   for (n = 0 ; n < Used.BattleSize ; n++) {
      printf("%c", '@'+BattleTeams[n]);
   }
   printf("%*s", (int)((Max.BattleSize <= 5 ? 5 : Max.BattleSize)-Used.BattleSize+1), "");

   fflush(stdout);
}
//|
/// void PrintBattleResult(int tc)
void PrintBattleResult(int tc) {
   int n1,t2;
   static const char TermCodes[TERM_MAX+1] = "WHITE";
   printf("%5ld %c %c %s\n", CurrentTurn, TermCodes[tc-1], '@'+Winner, TeamDatas[Winner].Title);
   fflush(stdout);

   /* Optæl data. */
   for(n1 = 0 ; n1 < Used.BattleSize ; n1++) {
      int t1 = BattleTeams[n1]; 
      for(t2 = 0 ; t2 <= t1 ; t2 += t1) {
         GameTotals[t2].NumBorn    += TeamDatas[t1].NumBorn;
         GameTotals[t2].NumAnts    += TeamDatas[t1].NumAnts;
         GameTotals[t2].NumBases   += TeamDatas[t1].NumBases;
         GameTotals[t2].BasesBuilt += TeamDatas[t1].BasesBuilt;
         GameTotals[t2].Kill       += TeamDatas[t1].Kill;
         GameTotals[t2].Killed     += TeamDatas[t1].Killed;
         GameTotals[t2].DieAge     += (TeamDatas[t1].DieAge+500)/1000;
         GameTotals[t2].TimesRun   += (TeamDatas[t1].TimesRun+500)/1000;
         GameTotals[t2].TimesTimed += TeamDatas[t1].TimesTimed;
         GameTotals[t2].TimeUsed   += ScaleDiv(TeamDatas[t1].TimeUsed,CLOCKS_PER_SEC,10000);
         GameTotals[t2].NumTurns   += CurrentTurn;
	 GameTotals[t2].NumBattles += 1;
      }
   }
   GameTotals[Winner].NumWon++;
   GameTotals[0].NumWon++;
}
//|
/// void PrintGameResult(int br)
static int compare(const void *a, const void *b) {
  return *((long *)a)-*((long *)b);
}
void PrintGameResult(int br) {
   u_long t,i,basetime,slowmed,sizemed,perfsum,pressum;

   /* Time the timer */
   basetime = clock();
   for (i = 1 ; i < 100000 ; i++) {
      clock();
   }
   basetime = clock()-basetime;
   basetime = basetime*1000/CLOCKS_PER_SEC;

   if (br) printf("\x0d%70s\n","");
   if (BattleCount == 0) return;

   for (t = 0 ; t <= NumTeams ; t++) {
      pgr_vict[t] = ScaleDiv(GameTotals[t].NumWon, GameTotals[t].NumBattles, 1000);
      pgr_base[t] = ScaleDiv(GameTotals[t].BasesBuilt, GameTotals[t].NumBattles, 10);
      pgr_comb[t] = ScaleDiv(GameTotals[t].Kill, GameTotals[t].Kill+GameTotals[t].Killed, 1000);
      pgr_time[t] = ScaleDiv(GameTotals[t].TimeUsed, GameTotals[t].TimesTimed, 100000)-basetime;
      if (((long)pgr_time[t]) < 1) pgr_time[t] = 1;
      pgr_slow[t] = pgr_time[t];
      pgr_size[t] = TeamDatas[t].MemSize;
   }

   qsort(&pgr_slow[1], NumTeams, sizeof(u_long), compare);
   qsort(&pgr_size[1], NumTeams, sizeof(u_long), compare);

   slowmed = (pgr_slow[(NumTeams+1)/2]+pgr_slow[(NumTeams+2)/2])/2;
   sizemed = (pgr_size[(NumTeams+1)/2]+pgr_size[(NumTeams+2)/2])/2;

   perfsum = 0;
   pressum = 0;
   for (t = 0 ; t <= NumTeams ; t++) {
      pgr_perf[t] = ScaleDiv(pgr_vict[t]*slowmed, pgr_time[t], 1);
      if (TeamDatas[t].MemSize == 0) {
	if (sizemed == 0) {
	  pgr_pres[t] = pgr_vict[t];
	} else {
	  pgr_pres[t] = pgr_vict[t]*sizemed*ZeroMemFactor;
	}
      } else {
	pgr_pres[t] = ScaleDiv(pgr_vict[t]*sizemed, TeamDatas[t].MemSize, 1);
      }
      if (t != 0) {
         perfsum += pgr_perf[t];
	 pressum += pgr_pres[t];
      }
   }
   pgr_perf[0] = perfsum/NumTeams;
   pgr_pres[0] = pressum/NumTeams;

   printf("\nTeam    Battles  Won Bases  Ants  Size  Ages   Comb   Time   Vict   Perf   Pres\n");
   for (i = 1 ; i <= NumTeams+1 ; i++) {
      t = i % (NumTeams+1);
      printf("%-10s%5d%5d%4ld.%01ld%6ld%6ld%6ld%4ld.%01ld%%%7ld%4ld.%01ld%%%4ld.%01ld%%%4ld.%01ld%%\n",
	     (t ? TeamDatas[t].Title : "Tot./Aver."),
	     GameTotals[t].NumBattles,
	     GameTotals[t].NumWon,
	     pgr_base[t]/10, pgr_base[t]%10,
	     ScaleDiv(GameTotals[t].NumBorn, GameTotals[t].NumBattles, 1),
	     ScaleDiv(GameTotals[t].TimesRun, GameTotals[t].NumTurns, 1000),
	     ScaleDiv(GameTotals[t].DieAge, GameTotals[t].NumBorn-GameTotals[t].NumAnts, 1000),
	     pgr_comb[t]/10, pgr_comb[t]%10,
	     pgr_time[t],
	     pgr_vict[t]/10, pgr_vict[t]%10,
	     pgr_perf[t]/10, pgr_perf[t]%10,
	     pgr_pres[t]/10, pgr_pres[t]%10);
   }
}
//|
/// u_long ScaleDiv(u_long a, u_long b, u_long scale)
u_long ScaleDiv(u_long a, u_long b, u_long scale) {
   while(a < 400000000 && b < 400000000 && scale > 1) a *= 10, scale /= 10;
   return (b >= scale ? (a+(b/scale)/2)/(b/scale) : 0);
}
//|

/* DoTurn() udfører en generation.
   RunAnt() samler argumenterne til en myrerutine og kører den.
   DoAction() udfører en kommando fra en myrerutine.
   PlaceFood() Placerer en ny klump mad.
   FoodOwnTouch() tæller mad-ejendom op for det givne felt.
   ------------------------------------------------------------ */
/// void DoTurn()
void DoTurn(void) {
   struct AntData *ant;
   /* Ny tur */
   CurrentTurn++;
   /* Kald alle myrene i tilfældig rækkefølge */
   TurnAnts = NumAnts;
   while(TurnAnts) {
      /* Vælg tilfældig myre */
      ant = AntList[Random(TurnAnts)];
      /* Flyt myren til den udførte del af listen */
      AntList[ant->Index] = AntList[--TurnAnts];
      AntList[ant->Index]->Index = ant->Index;
      AntList[TurnAnts] = ant;
      ant->Index = (u_short) TurnAnts;
      /* Kald myrerutinen og udfør dens ordre */
      DoAction(ant,RunAnt(ant));
   }
 
   /* Placer ny mad */
   while(NumAnts+NumFood+BaseValue*NumBases < ((Used.MapWidth*Used.MapHeight)/Used.NewFoodSpace))
      PlaceFood();
}
//|
/// int  RunAnt(struct AntData *ant)
int RunAnt(struct AntData *ant) {
   int num,team,size, retval;
   int timeflag = (Random(TimeRate) == 0);
   struct SquareData FeltData[5];
   int x = ant->XPos, y = ant->YPos;
   int f;
   u_char *ts = &TeamShuffle(ant->Team-1,0);

   /* Udfyld et array med indholdet af hukommelsen af samtlige myrer på
    * feltet.
    */
   num = MapData(x,y).NumAnts;
   team = MapData(x,y).Team;
   size = TeamDatas[team].MemSize;

   if (ts[team]) printf("%d ",ts[team]);

   /* Udfyld et array med information om indholdet af felterne lige
    * omkring, hvor myren står.
    */
   FeltData[0]=MapData(x,y);                           /* Selve feltet */
   FeltData[1]=MapData((x==Used.MapWidth-1 ? 0 : x+1), y);  /* Til højre for */
   FeltData[2]=MapData(x, (y==Used.MapHeight-1 ? 0 : y+1)); /* Under */
   FeltData[3]=MapData((x==0 ? Used.MapWidth-1 : x-1), y);  /* Til venstre for */
   FeltData[4]=MapData(x, (y==0 ? Used.MapHeight-1 : y-1)); /* Over */

   /* Bland holdnumrene. Myren tror, den selv er hold 0. */
   for(f = 0 ; f <= 4 ; f++) {
     FeltData[f].Team =
       (FeltData[f].NumAnts || FeltData[f].Base
        ? ts[FeltData[f].Team]
        : 0);
   }

   /* Fjern den kaldte midlertidigt fra feltet. */
   ant->MapPrev->MapNext = ant->MapNext;

   /* Kopier myrerne ind. */
   TeamDatas[team].CopyIn(ant, AntTemp, 1);
   TeamDatas[team].CopyIn(MapAnt(x,y).MapFirst, AntTemp+size, num-1);

   /* Tidstagning? */
   if(timeflag) {
      int t1,t2;
      t1 = clock();
      retval=TeamDatas[team].TeamFunc(FeltData, AntTemp);
      t2 = clock();
      TeamDatas[team].TimeUsed += t2-t1;
      TeamDatas[team].TimesTimed++;
   } else {
      retval=TeamDatas[team].TeamFunc(FeltData, AntTemp);
   }

   /* Kopier myrerne tilbage igen */
   TeamDatas[team].CopyOut(ant, AntTemp, 1);
   TeamDatas[team].CopyOut(MapAnt(x,y).MapFirst, AntTemp+size, num-1);

   /* Sæt den kaldte myre ind i feltlisten igen. */
   ant->MapPrev->MapNext = ant;

   /* Opdater tællere. */
   TeamDatas[team].TimesRun++;
   ant->Age++;
   ant->NextTurn++;

   return retval;
}
//|
/// void DoAction(struct AntData *ant, int action)
void DoAction(struct AntData *ant, int action) {
   int t;
   int x = ant->XPos, y = ant->YPos;
   u_long nx = x, ny = y;
   struct SquareData *sd,*sd2;

   /* Hvor vil myren hen? */
   switch(action&7) {
      case 1: { nx = (x == Used.MapWidth-1  ? 0 : x+1); break; } /* Til højre */
      case 2: { ny = (y == Used.MapHeight-1 ? 0 : y+1); break; } /* Nedad */
      case 3: { nx = (x == 0 ? Used.MapWidth-1  : x-1); break; } /* Til ventre */
      case 4: { ny = (y == 0 ? Used.MapHeight-1 : y-1); break; } /* Opad */
   }
   sd = &MapData(x,y);
   sd2 = &MapData(nx,ny);

   /* Skal der bygges en ny base? */
   if(action == 16 && sd->NumAnts > NewBaseAnts
                   && sd->NumFood >= NewBaseFood
                   && sd->Base == 0) {
      /* Gammel mad-egendom på feltet. */
      FoodOwnTouch(sd,-1);
      /* Fjern den byggende myre midlertidigt */
      ant->MapNext->MapPrev = ant->MapPrev;
      ant->MapPrev->MapNext = ant->MapNext;
      /* Dræb NewBaseAnts myrer */
      for(t = NewBaseAnts ; t-- ;)
         KillAnt(MapAnt(x,y).MapFirst, ant->Team);
      /* Sæt den byggende myre ind igen */
      ant->MapNext = MapAnt(x,y).MapFirst;
      ant->MapPrev = MapAnt(x,y).MapFirst->MapPrev;
      MapAnt(x,y).MapFirst->MapPrev = ant;
      MapAnt(x,y).MapFirst = ant;
      /* Fjern NewBaseFood mad og lav myrer for resten */
      for(t = sd->NumFood-NewBaseFood ; t-- ;) {
         NewAnt(x,y);
      }
      NumFood -= sd->NumFood;
      sd->NumFood = 0;
      /* Byg basen */
      sd->Base = 1;
      TeamDatas[ant->Team].NumBases++;
      TeamDatas[ant->Team].BasesBuilt++;
      NumBases++;
      BasesBuilt++;
      /* Ny mad-egendom på feltet. */
      FoodOwnTouch(sd,1);
      /* Opdater grafikken */
      SysSquareChanged(x,y);
   }
   /* Vil den overhovedet nogen vegne? */
   if((nx != x || ny != y) && (sd2->NumAnts < MaxSquareAnts || sd2->Team != ant->Team)) {
      /* Gammel mad-egendom */
      FoodOwnTouch(sd,-1);
      FoodOwnTouch(sd2,-1);
      /* Fjern myren fra det gamle felt */
      ant->MapNext->MapPrev = ant->MapPrev;
      ant->MapPrev->MapNext = ant->MapNext;
      sd->NumAnts--;
      /* Dræb eventuelle fjendtlige myrer på det nye felt */
      if(sd2->Team != ant->Team) {
         while(sd2->NumAnts)
            KillAnt(MapAnt(nx,ny).MapFirst, ant->Team);
         if(sd2->Base) {
            sd2->Base = 0;
            TeamDatas[sd2->Team].NumBases--;
            NumBases--;
         }
         TeamDatas[sd2->Team].SquareOwn--;
         TeamDatas[ant->Team].SquareOwn++;
      }
      /* Indsæt myren på det nye felt */
      ant->MapNext = MapAnt(nx,ny).MapLast->MapNext;
      ant->MapPrev = MapAnt(nx,ny).MapLast;
      MapAnt(nx,ny).MapLast->MapNext = ant;
      MapAnt(nx,ny).MapLast = ant;
      ant->XPos = (u_short) nx; ant->YPos = (u_short) ny;
      sd2->Team = ant->Team;
      sd2->NumAnts++;
      /* Flyt mad med, hvis der bliver bedt om det */
      if((action&8) && sd->NumFood && sd2->NumFood < MaxSquareFood) {
         sd->NumFood--;
         if(sd2->Base) {
            NewAnt( (int) nx, (int) ny);		//HACK: NewAnt should probably have unsigned arguments
            NumFood--;
         } else
            sd2->NumFood++;
      }
      /* Ny mad-egendom */
      FoodOwnTouch(sd,1);
      FoodOwnTouch(sd2,1);
      /* Fortæl system-rutinerne, at der er sket noget */
      SysSquareChanged( x, y);					//HACK: SysSquareChanged should probably have unsigned arguments
      SysSquareChanged( (int) nx, (int) ny);
   }
}
//|
/// void PlaceFood()
void PlaceFood(void) {
   static short LastFoodX[LastFoodMem],LastFoodY[LastFoodMem];
   short i,j, x,y, xdist,ydist, bestx = 0,besty = 0, mindist, bestdist = -1, num;
   struct SquareData data;
   for(i = 0 ; i < 20 ; i++) {
      do {
         x = (short) Random(Used.MapWidth); y = (short) Random(Used.MapHeight);
         data = MapData(x,y);
      } while(data.NumAnts || data.NumFood || data.Base);

      mindist = Used.MapWidth+Used.MapHeight;
      for(j = 0 ; j < (LastFoodIndex < LastFoodMem ? LastFoodIndex : LastFoodMem) ; j++) {
         xdist = x-LastFoodX[j];
         ydist = y-LastFoodY[j];
         if(xdist < 0) xdist = -xdist;
         if(ydist < 0) ydist = -ydist;
         if(xdist > Used.MapWidth/2) xdist = Used.MapWidth-xdist;
         if(ydist > Used.MapHeight/2) ydist = Used.MapHeight-ydist;
         if(xdist+ydist < mindist) mindist = xdist+ydist;
      }

      if(mindist > bestdist) {
         bestdist = mindist;
         bestx = x;
         besty = y;
      }
   }

   j = (LastFoodIndex++)%LastFoodMem;
   LastFoodX[j] = bestx;
   LastFoodY[j] = besty;

   num = Used.NewFoodMin+Random(Used.NewFoodDiff+1);
   MapData(bestx,besty).NumFood += num;
   NumFood += num;
   SysSquareChanged(bestx,besty);
}
//|
/// void FoodOwnTouch(struct SquareData *sd, int factor)
void FoodOwnTouch(struct SquareData *sd, int factor) {
   int fo = (sd->NumFood <= sd->NumAnts ? sd->NumFood : sd->NumAnts);
   int ft = sd->NumFood-fo;
   int fk = sd->NumFood;
   TeamDatas[sd->Team].FoodOwn += fo*factor;
   TeamDatas[sd->Team].FoodTouch += ft*factor;
   TeamDatas[sd->Team].FoodKnown += fk*factor;
}
//|

/* AllocAnt() allokerer en myre fra myre-bufferen.
   FreeAnt() frigiver en myre igen.
   NewAnt() skaber en ny myre på den angivne base.
   KillAnt() dræber den angivne myre.
   ----------------------------------------------- */
/// struct AntData *AllocAnt()
struct AntData *AllocAnt(void) {
  struct AntData *ant = AntFreeList;
  AntFreeList = ant->MapNext;
  return ant;
}
//|
/// void FreeAnt(struct AntData *ant)
void FreeAnt(struct AntData *ant) {
  ant->MapNext = AntFreeList;
  AntFreeList = ant;
}
//|
/// void NewAnt(int x, int y)
void NewAnt(int x, int y) {
   int team = MapData(x,y).Team;
   struct AntData *ant;

	assert(x >= 0);
	assert(y >= 0);
   /* Reserver RAM */
   ant = AllocAnt();
   /* Indsæt myren i myrelisten */
   ant->Index = NumAnts;
   AntList[NumAnts++] = ant;
   /* Indsæt myren i feltlisten */
   ant->MapNext = MapAnt(x,y).MapLast->MapNext;
   ant->MapPrev = MapAnt(x,y).MapLast;
   MapAnt(x,y).MapLast->MapNext = ant;
   MapAnt(x,y).MapLast = ant;
   MapData(x,y).NumAnts++;
   /* Initialiser myrens header */
   ant->Team = team;
   ant->XPos = x;
   ant->YPos = y;
   ant->Age = 0;
   ant->NextTurn = CurrentTurn+1;
   /* Opdater øvrige antal-variable */
   NumBorn++;
   TeamDatas[team].NumBorn++;
   TeamDatas[team].NumAnts++;
   /* Lidt at starte på... */
   ant->Mem[0] = (Random(1<<16)<<16)+Random(1<<16);
   if (TeamDatas[team].MemSize > sizeof(u_long)) {
       memset(&ant->Mem[1],0,TeamDatas[team].MemSize-sizeof(u_long));
   }
}
//|
/// void KillAnt(struct AntData *ant, int team)
void KillAnt(struct AntData *ant, int team) {
   /* Tag myren ud af myrelisten */
   if(ant->NextTurn == CurrentTurn) {
      /* Myren har ikke trukket i denne tur. */
      AntList[ant->Index] = AntList[--TurnAnts];
      AntList[ant->Index]->Index = ant->Index;
      AntList[TurnAnts] = AntList[--NumAnts];
      AntList[TurnAnts]->Index = TurnAnts;
   } else {
      /* Myren har trukket i denne tur. */
      AntList[ant->Index] = AntList[--NumAnts];
      AntList[ant->Index]->Index = ant->Index;
   }
   /* Tag myren ud af feltlisten */
   ant->MapNext->MapPrev = ant->MapPrev;
   ant->MapPrev->MapNext = ant->MapNext;
   MapData(ant->XPos, ant->YPos).NumAnts--;
   /* Opdater øvrige antal-variable */
   TeamDatas[ant->Team].NumAnts--;
   TeamDatas[ant->Team].DieAge += ant->Age;
   TeamDatas[ant->Team].Killed++;
   TeamDatas[team].Kill++;
   /* Frigiv RAM */
   FreeAnt(ant);
}
//|

/* GameInit() sætter op til et spil.
   BattleInit() sætter op til en kamp.
   BattleExit() ryder op efter en kamp.
   GameExit() ryder op efter et spil.
   ---------------------------------- */
/// bool GameInit()
bool GameInit(void) {
   MaxAnts = NumTeams*Max.StartAnts + Max.MapWidth*Max.MapHeight/Args.Min.NewFoodSpace+Max.NewFoodMin+Max.NewFoodDiff;
   AntMem = LargestMem <= sizeof(u_long) ? sizeof(struct AntData) : sizeof(struct AntData)-sizeof(u_long)+LargestMem;
   return (GameTotals   = (struct GameTotal *)  calloc(NumTeams+1,sizeof(struct GameTotal)))
       && (MapDatas     = (struct SquareData *) calloc(Max.MapWidth*Max.MapHeight,sizeof(struct SquareData)))
       && (MapAnts      = (struct SquareAnts *) calloc(Max.MapWidth*Max.MapHeight,2*sizeof(struct AntData *)))
       && (AntList      = (struct AntData **)   calloc(MaxAnts,sizeof(struct AntData *)))
       && (AntSpace     = (u_char *)            calloc(MaxAnts,AntMem))
       && (AntTemp      = (u_char *)            calloc(255,AntMem))
       && (BattleTeams  = (u_char *)            calloc(NumTeams,sizeof(u_char)))
       && (TeamIndex    = (u_char *)            calloc((NumTeams+1),sizeof(u_char)))
       && (TeamShuffles = (u_char *)            calloc(NumTeams*(NumTeams+1),sizeof(u_char)))
       && (pgr_vict     = (u_long *)            calloc((NumTeams+1),sizeof(u_long)))
       && (pgr_base     = (u_long *)            calloc((NumTeams+1),sizeof(u_long)))
       && (pgr_time     = (u_long *)            calloc((NumTeams+1),sizeof(u_long)))
       && (pgr_comb     = (u_long *)            calloc((NumTeams+1),sizeof(u_long)))
       && (pgr_slow     = (u_long *)            calloc((NumTeams+1),sizeof(u_long)))
       && (pgr_size     = (u_long *)            calloc((NumTeams+1),sizeof(u_long)))
       && (pgr_perf     = (u_long *)            calloc((NumTeams+1),sizeof(u_long)))
       && (pgr_pres     = (u_long *)            calloc((NumTeams+1),sizeof(u_long)))
       && (x            = (u_short *)           calloc((NumTeams+1),sizeof(u_short)))
       && (y            = (u_short *)           calloc((NumTeams+1),sizeof(u_short)))
       && (bestx        = (u_short *)           calloc((NumTeams+1),sizeof(u_short)))
       && (besty        = (u_short *)           calloc((NumTeams+1),sizeof(u_short)))
       && (taken        = (u_char *)            calloc((NumTeams+1),sizeof(u_char)))
       ;
}
//|
/// void BattleInit()
void BattleInit(void) {
   int i,n,t;
   struct SquareAnts *ptr;
   struct AntData *ant;

   /* Initialiser simple variable */
   NumBorn = 0; NumAnts = 0; NumFood = 0; NumBases = 0; BasesBuilt = 0;
   CurrentTurn = 0; LastFoodIndex = 0;

   /* Rens kortet */
   memset(MapDatas,0,Used.MapWidth*Used.MapHeight*sizeof(struct SquareData));

   /* Initialiser myrelister */
   ptr = MapAnts;
   for(i = 0 ; i < Used.MapWidth*Used.MapHeight ; i++) {
      ptr->MapFirst = (struct AntData *) ptr;
      ptr->MapLast  = (struct AntData *) ptr;
      ptr++;
   }

   /* Initialiser freelist */
   AntFreeList = (struct AntData *) AntSpace;
   ant = AntFreeList;
   for (i = 0 ; i < MaxAnts-1 ; i++) {
      struct AntData *next = (struct AntData *)(((u_char *)ant)+AntMem);
      ant->MapNext = next;
      ant = next;
   }
   ant->MapNext = 0;

   /* Initialiser hold-specifikke variable */
   {
      int bestdist = 0;
      for(i = 0 ; i < BasePlaceTries ; i++) {
	 int mindist = Used.MapWidth+Used.MapHeight;
	 for(n = 0 ; n < Used.BattleSize ; n++) {
	    t = BattleTeams[n];
	    if (t == 1) {
	       x[n] = Used.MapWidth/2;
	       y[n] = Used.MapHeight/2;
	    } else {
	       x[n] = Random(Used.MapWidth);
	       y[n] = Random(Used.MapHeight);
	    }
         }
	 for(n = 0 ; n < Used.BattleSize-1 ; n++) {
            int m;
            for(m = n+1 ; m < Used.BattleSize ; m++) {
               int dx = (x[m] > x[n] ? x[m]-x[n] : x[n]-x[m]);
	       int dy = (y[m] > y[n] ? y[m]-y[n] : y[n]-y[m]);
	       if(dx > Used.MapWidth/2) dx = Used.MapWidth-dx;
	       if(dy > Used.MapHeight/2) dy = Used.MapHeight-dy;
	       if(dx+dy < mindist) mindist = dx+dy;
	     }
         }
         if(mindist > bestdist) {
            bestdist = mindist;
            for(n = 0 ; n < Used.BattleSize ; n++) {
               bestx[n] = x[n];
               besty[n] = y[n];
            }
         }
      }

      for (t = 1 ; t <= NumTeams ; t++) {
         TeamDatas[t].NumBorn    = 0;
         TeamDatas[t].NumAnts    = 0;
         TeamDatas[t].NumBases   = 0;
         TeamDatas[t].BasesBuilt = 0;
         TeamDatas[t].Kill       = 0;
         TeamDatas[t].Killed     = 0;
         TeamDatas[t].DieAge     = 0;
         TeamDatas[t].TimesRun   = 0;
         TeamDatas[t].TimesTimed = 0;
         TeamDatas[t].TimeUsed   = 0;
         TeamDatas[t].SquareOwn  = 0;
         TeamDatas[t].FoodOwn    = 0;
         TeamDatas[t].FoodTouch  = 0;
         TeamDatas[t].FoodKnown  = 0;
      }

      for (n = 0 ; n < Used.BattleSize ; n++) {
         t = BattleTeams[n];
         TeamDatas[t].NumBases   = 1;
         TeamDatas[t].BasesBuilt = 1;
         TeamDatas[t].SquareOwn  = 1;
         NumBases++;
         BasesBuilt++;
         MapData(bestx[n],besty[n]).Team = t;
         MapData(bestx[n],besty[n]).Base = 1;
         for(i = Used.StartAnts ; i-- ;) NewAnt(bestx[n],besty[n]);
         SysSquareChanged(bestx[n],besty[n]);
      }
   }

   /* Lav en blandingstabel til holdnumrene. */
   {
      int n1,n2,v;
      for (v = 0 ; v < Used.BattleSize ; v++) taken[v] = 0;
      for (n1 = 0 ; n1 < Used.BattleSize ; n1++) {
         int t1 = BattleTeams[n1];
         taken[0]++;
	 TeamShuffle(t1-1,0) = 0;
	 for (n2 = 0 ; n2 < Used.BattleSize ; n2++) {
            int t2 = BattleTeams[n2];
            if (n2 == n1) {
               TeamShuffle(t1-1,t2) = 0;
            } else {
               do {
		  v = Random(Used.BattleSize);
	       } while (taken[v] > n1);
               TeamShuffle(t1-1,t2) = v;
               taken[v]++;
            }
         }
      }
   }

}
//|
/// void BattleExit()
void BattleExit(void) {
}
//|
/// void GameExit()
void GameExit(void) {
   /* Frigiv RAM */
   if (TeamDatas)    { free(TeamDatas);    TeamDatas    = 0; }
   if (GameTotals)   { free(GameTotals);   GameTotals   = 0; }
   if (MapDatas)     { free(MapDatas);     MapDatas     = 0; }
   if (MapAnts)      { free(MapAnts);      MapAnts      = 0; }
   if (AntList)      { free(AntList);      AntList      = 0; }
   if (AntSpace)     { free(AntSpace);     AntSpace     = 0; }
   if (AntTemp)      { free(AntTemp);      AntTemp      = 0; }
   if (BattleTeams)  { free(BattleTeams);  BattleTeams  = 0; }
   if (TeamIndex)    { free(TeamIndex);    TeamIndex    = 0; }
   if (TeamShuffles) { free(TeamShuffles); TeamShuffles = 0; }
   if (pgr_vict)     { free(pgr_vict);     pgr_vict     = 0; }
   if (pgr_base)     { free(pgr_base);     pgr_base     = 0; }
   if (pgr_time)     { free(pgr_time);     pgr_time     = 0; }
   if (pgr_comb)     { free(pgr_comb);     pgr_comb     = 0; }
   if (pgr_slow)     { free(pgr_slow);     pgr_slow     = 0; }
   if (pgr_size)     { free(pgr_size);     pgr_size     = 0; }
   if (pgr_perf)     { free(pgr_perf);     pgr_perf     = 0; }
   if (pgr_pres)     { free(pgr_pres);     pgr_pres     = 0; }
   if (x)            { free(x);            x            = 0; }
   if (y)            { free(y);            y            = 0; }
   if (bestx)        { free(bestx);        bestx        = 0; }
   if (besty)        { free(besty);        besty        = 0; }
   if (taken)        { free(taken);        taken        = 0; }
}
//|

/* Random() genererer næste pseudotilfældige tal.
   -------------------------------------------- */
/// u_long  Random(u_long num)
u_long Random(u_long num) {
   u_long a = (BattleSeed = BattleSeed*(42*42-42/42+42)+7);
   return ((a<<19)+(a>>13))%num;
}
//|
