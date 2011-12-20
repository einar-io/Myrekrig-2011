#ifndef MYREKRIG_H
#define MYREKRIG_H

/* Includes... */

#include <stdlib.h>

#if _MSC_VER
#define snprintf _snprintf
#endif

/* C++ må også godt være med */

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

/* Egne includes... */

#include "Myre.h"

/* Konstanter */

#define SYS_CONTINUE 0
#define SYS_SKIP 1
#define SYS_INTERRUPT 2
#define SYS_EXIT 3
#define SYS_RESTART 4
#define SYS_MAKELAST 5
#define SYS_MAX 5

#define TERM_CONTINUE 0
#define TERM_WIN 1
#define TERM_HALFTIME 2
#define TERM_INTERRUPTED 3
#define TERM_TIMEOUT 4
#define TERM_ERROR 5
#define TERM_MAX 5

/* Makroer... */

#define MapData(x,y) MapDatas[(x)+(y)*Used.MapWidth]
#define MapAnt(x,y) MapAnts[(x)+(y)*Used.MapWidth]
#define TeamShuffle(t1,t2) TeamShuffles[(t1)*(NumTeams+1)+(t2)]

/* Myre-registrerings-håndterings-makroer... */

#define UseAnt(name) \
{ \
   extern int (* const name##_Func)(struct SquareData *, void *); \
   extern void (* const name##_CopyInFunc)(struct AntData *s, void *a, int n); \
   extern void (* name##_CopyOutFunc)(struct AntData *s, void *a, int n); \
   extern int (* const name##_Func)(struct SquareData *, void *); \
   extern char * const name##_Title; \
   extern const int name##_MemSize; \
   TeamDatas[team_num*team_flag].TeamFunc = name##_Func; \
   TeamDatas[team_num*team_flag].CopyIn = name##_CopyInFunc; \
   TeamDatas[team_num*team_flag].CopyOut = name##_CopyOutFunc; \
   TeamDatas[team_num*team_flag].MemSize = name##_MemSize; \
   setTitle(&TeamDatas[team_num*team_flag], name##_Title); \
   if (name##_MemSize > LargestMem) LargestMem = name##_MemSize; \
   team_num++; \
}

#define UseAntBegin \
void InitTeams(void) { \
   int team_flag, team_num; \
   struct TeamData dummy; \
   TeamDatas = &dummy; \
   LargestMem = 0; \
   for(team_flag = 0 ; team_flag <= 1 ; team_flag++) { \
      team_num = 1;

#define UseAntEnd \
    if(team_flag == 0) { \
      NumTeams = team_num-1; \
      TeamDatas = (struct TeamData *)calloc(NumTeams+1,sizeof(struct TeamData)); \
    } \
  } \
}

/* structs... */

struct SquareAnts {
   struct AntData *MapFirst, *MapLast;
};

struct TeamData {
   int (*TeamFunc)(struct SquareData *, void *);
   void (*CopyIn)(struct AntData *, void *, int);
   void (*CopyOut)(struct AntData *, void *, int);
   char Title[11];
   u_long Color;
   u_short MemSize;
   u_long NumBorn, NumAnts, NumBases, BasesBuilt, FoodOwn, FoodTouch, FoodKnown;
   u_long SquareOwn, Kill, Killed, DieAge;
   u_long TimesRun, TimesTimed, TimeUsed;
};

struct GameTotal {
   u_long NumBorn, NumAnts, NumBases, BasesBuilt;
   u_long Kill, Killed, DieAge;
   u_long TimesRun, TimesTimed, TimeUsed, NumTurns;
   u_short NumBattles, NumWon;
};

struct Parameters {
   u_long MapWidth, MapHeight, StartAnts;
   u_long NewFoodSpace, NewFoodMin, NewFoodDiff;
   u_long HalfTimeTurn, TimeOutTurn;
   u_long WinPercent, HalfTimePercent;
   u_long BattleSize;
};

struct Limits {
   struct Parameters Min,Max;
   u_long NumBattles;
   u_long GameSeed;
   u_long WatchTeam;
};

/* Externs... */

extern struct Limits Args;
extern struct Parameters Used,Max;
extern struct SquareData *MapDatas;
extern struct SquareAnts *MapAnts;
extern struct AntData **AntList;
extern struct TeamData *TeamDatas;
extern struct GameTotal *GameTotals;
extern u_char *BattleTeams, *TeamIndex;
extern u_char *TeamShuffles;
extern u_long NumBorn, NumAnts, TurnAnts, NumFood, NumBases, BasesBuilt, CurrentTurn;
extern u_short Winner, BattleCount, LastFoodIndex, NumBattlesSwap;
extern u_short NumTeams, LargestMem;
extern u_long BattleSeed;
extern u_long MaxAnts;

/* System prototypes... */

bool SysGameInit(int argc, char *argv[]);
bool SysBattleInit(void);
void SysBattleExit(void);
void SysGameExit(void);
bool SysCheck(void);
void SysDrawMap(void);
void SysSquareChanged(int,int);

/* Internal prototype... */

void InitTeams(void);
void setTitle(struct TeamData *td, char *title);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif

