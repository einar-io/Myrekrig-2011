//Lavet af Jesper Torp Kristensen, u993197.

//#include <stdio.h>
//#include <string.h>
#include "Myre.h"

#define AntMem struct DocBrain

#define NULL 0

#define INITIAL_RADIUS 20 //dvs. virkelig radius 3*INITIAL_RADIUS.
#define RADIUS_STEP 10
#define MAX_REPORT_COUNT 16

#define min(x,y) ((x)<(y) ? (x) : (y))
#define abs(x) ((x)<0 ? -(x) : (x)) 
#define baseDist(xx) (abs((xx).x)+abs((xx).y))
#define gotoDest(mem) \
    if ((mem)->pos.x != (mem)->dest.x) { \
      if ((mem)->pos.x < (mem)->dest.x) return 1; \
      return 3; \
    } else if ((mem)->pos.y != (mem)->dest.y) { \
      if ((mem)->pos.y < (mem)->dest.y) return 4; \
      return 2; \
    } else return 0;
#define gotoBase(mem) {mem->dest.x = mem->dest.y = 0; gotoDest(mem);}
#define destReached(mem) (((mem)->pos.x==(mem)->dest.x) && ((mem)->pos.y==(mem)->dest.y))
#define morphToAndReturnRookie(felt, mem); \
    (mem)->rang = ROOKIE; \
    (mem)->carryingFood = 0; \
    return DocRookie(felt, mem);
#define zeroPos(mem) (!((mem)->pos.x  ||  (mem)->pos.y))
#define nulstilQueen(mem) \
   (mem)->u.q.lastExpanded = (mem)->u.q.next = (mem)->u.q.antsBorn = (mem)->u.q.enemyReportCount = 0;
//#define getKvadrant(pos) ((pos).x<0 ? ((pos).y<0 ? 2 : 3) : ((pos).y<0 ? 1 : 0))

#define ROOKIE 0
#define EXPLORER 1
#define SOLDIER 2
#define COLLECTOR 3
#define WAR_REPORTER 4
#define FOOD_GUARD 5
#define FOOD_REPORTER 6
#define QUEEN 7

char *DocAntNames[] = {"ROOKIE", "EXPLORER", "SOLDIER", "COLLECTOR", "WAR_REPORTER", "FOOD_GUARD", "FOOD_REPORTER", "QUEEN"};

struct DocPos{
  short x,y;
};

struct DocFoodReport{
  struct DocPos pos;
  short amount,dist;
};

struct DocBrain{
  u_long randseed;
  char carryingFood;//er enten 0 eller 8
  char rang;
  unsigned short turn;
  struct DocPos pos,dest;
  union{
    struct{
      //EXPLORER
      int ignoreFood;//Hvis positiv, saa skal myren ikke gaa efter mad.
    } e;
    struct{
      //FOOD GUARD
      int timeToRetire;
      int isReported;
    } fg;
    struct{
      //QUEEN:
      int radius; //skal ganges med 3.
      int next;
      int antsBorn;
      int enemyReportCount;
      int lastExpanded;
    } q;
    struct{
      //FOOD_REPORTER:
      struct DocFoodReport report[MAX_REPORT_COUNT];
      int reportCount;
    } fr;
  } u;
};

u_long DocGetRand(AntMem *mem){
  //returnerer et pseudo tilfældigt tal mellem 0 og 65535.
  mem->randseed = mem->randseed*1103515245+12345;
  return (u_long)((mem->randseed>>16) & 65535);
}

int DocEnemyInSight(struct SquareData *felt) {
  //Returnerer 0, hvis der ingen fjender er. Ellers returneres retningen.
  int i, value, bestHit=0, bestHitPos=0;
  for (i=1; i<5; i++) {
    if (felt[i].Team) {
      value = felt[i].NumAnts;
      if (felt[i].Base) value+=BaseValue;
      if (value>bestHit) {
	bestHit=value;
	bestHitPos=i;
      }
    }
  }
  return bestHitPos;
}

AntMem *DocGetAnt(struct SquareData *felt, AntMem *mem, int antRang) {
  //Finder og returnerer den foerste myre paa feltet med rang=antRang.
  //Hvis ingen myre har den givne rang, returneres NULL.
  int i;
  for (i=1; i<felt->NumAnts; i++)
    if (mem[i].rang == antRang) return &(mem[i]);
  return NULL;
}

int DocRookie(struct SquareData *felt, AntMem *mem) {
  AntMem *ant,*queen;
  int i,r,tmp,minDist;
  if (r=DocEnemyInSight(felt)) {
    //printf("Basen forsvares\n");
    return r;
  }
  if (!zeroPos(mem)) gotoBase(mem);
  if (!felt->Base) return 0;
  if ((queen = DocGetAnt(felt,mem,QUEEN)) == NULL) {
    //Der er ingen dronning! Lav sig selv til dronning
    mem->rang = QUEEN;
    mem->u.q.radius = INITIAL_RADIUS;
    nulstilQueen(mem);
    return DocQueen(felt,mem);
  }
  if (!mem->turn) queen->u.q.antsBorn++;

  minDist=0x7FFF;
  ant = NULL;
  for (i=1; i<felt->NumAnts; i++)
    if (mem[i].rang == FOOD_REPORTER)
      if (mem[i].u.fr.report[0].dist < minDist) {
	minDist = mem[i].u.fr.report[0].dist;
	ant = &mem[i];
      }
  if (ant != NULL) {
    //Faa fortalt, hvor der er mad.
    mem->rang = COLLECTOR;
    mem->dest = ant->u.fr.report[0].pos;
    if (--ant->u.fr.report[0].amount < 1) {
      minDist=0x7FFF;
      r=0;
      for (i=1; i<ant->u.fr.reportCount; i++)
	if (ant->u.fr.report[i].dist < minDist) {
	  minDist = ant->u.fr.report[i].dist;
	  r=i;
	}
      ant->u.fr.report[0] = ant->u.fr.report[r];
      ant->u.fr.report[r] = ant->u.fr.report[--ant->u.fr.reportCount];
      if (!ant->u.fr.reportCount) {
	//FOOD_REPORTER har ikke flere meddelelser.
	ant->rang = ROOKIE;
	ant->carryingFood = 0;
      }
    }
    return DocCollector(felt, mem);
  }
  //Faa ordre af dronning.
  //Myrerne sendes af sted parvist.
  tmp = (unsigned int)queen->u.q.next >> 3;
  switch (queen->u.q.next & 6) {
  case 0:{ //1 Kvadrant:
    mem->dest.x = tmp;
    mem->dest.y = queen->u.q.radius - tmp;
    break;
  }
  case 2:{//Kvadrantet under 1. kvadrant
    mem->dest.x = tmp;
    mem->dest.y = tmp - queen->u.q.radius;
    break;
  }
  case 4:{//3. kvadrant
    mem->dest.x = -(tmp+1);
    mem->dest.y = (tmp+1) - queen->u.q.radius;
    break;
  }
  case 6:{//Kvadrantet til venstre for 1. kvadrant
    mem->dest.x = -(tmp+1);
    mem->dest.y = queen->u.q.radius - (tmp+1);
    break;
  }
  }
  mem->dest.x *= 3;
  mem->dest.y *= 3;
  mem->rang = EXPLORER;
  mem->u.e.ignoreFood = 0;
  //Opdater dronningens status.
  if (++queen->u.q.next == 6*queen->u.q.radius) {
    //printf("%d %d\n",queen->u.q.antsBorn,queen->u.q.enemyReportCount);
    if (60*queen->u.q.enemyReportCount > queen->u.q.antsBorn) {
      //War time.
      if (25*queen->u.q.enemyReportCount > queen->u.q.antsBorn) {
	//printf("Lot of war => decrease territory\n");
	if (queen->u.q.radius != INITIAL_RADIUS) queen->u.q.radius -= RADIUS_STEP;
      } else {
	//printf("Some war => territory size remains\n");
      }
    } else {
      //Udvid!
      //printf("Little war => expand\n");
      queen->u.q.radius += RADIUS_STEP;
    }
    nulstilQueen(queen);
  }
  return DocExplorer(felt, mem);
}

int DocExplorer(struct SquareData *felt, AntMem *mem) {
  int i,r;
  AntMem *ant;
  if (r=DocEnemyInSight(felt)) {
    if (DocGetRand(mem)&7) {
      mem->rang = SOLDIER;
    } else {
      mem->rang = WAR_REPORTER;
      mem->dest.x = mem->dest.y = 0;
    }
    return r;
  }
  //Er der mad paa dette felt?
  if (felt->NumFood >= felt->NumAnts) {
    if ((ant = DocGetAnt(felt, mem, FOOD_GUARD))==NULL) {
      //Lav myren til FOOD_GUARD
      mem->rang = FOOD_GUARD;
      mem->u.fg.isReported = 0;
      mem->u.fg.timeToRetire = 3*baseDist(mem->pos);
      mem->dest = mem->pos;
      return DocFoodGuard(felt, mem);
    } else if (ant->u.fg.isReported) {
      //Glem alt om det mad, der ligger her.
      mem->u.e.ignoreFood = 2;
    } else {
      //Bliv FOOD_REPORTER
      if (felt->NumFood > 2) {
	mem->rang = FOOD_REPORTER;
	mem->u.fr.report[0].pos = mem->pos;
	mem->u.fr.report[0].amount = felt->NumFood-2;
	mem->u.fr.report[0].dist = baseDist(mem->pos);
	mem->u.fr.reportCount = 1;
	ant->u.fg.isReported = 1;
      } else mem->rang = COLLECTOR;
      mem->carryingFood = 8;
      gotoBase(mem);
      return DocFoodReporter(felt, mem);
    }
  }
  //Er der mad ved siden af?
  if (--mem->u.e.ignoreFood < 0) 
    for (i=1; i<5; i++)
      if (felt[i].NumAnts < felt[i].NumFood) return i;
  if (destReached(mem)) {
    if (zeroPos(mem)) {
      morphToAndReturnRookie(felt, mem);
    } else {
      gotoBase(mem);
    }
  }
  gotoDest(mem);
}

int DocSoldier(struct SquareData *felt, AntMem *mem) {
  int r;
  if (r=DocEnemyInSight(felt)) return r;
  return 0;
}

int DocCollector(struct SquareData *felt, AntMem *mem) {
  int r;
  if (r=DocEnemyInSight(felt)) {
    if (DocGetRand(mem)&7) {
      mem->rang = SOLDIER;
    } else {
      mem->rang = WAR_REPORTER;
      mem->dest.x = mem->dest.y = 0;
    }
    return r;
  }
  if (destReached(mem)) {
    if (zeroPos(mem)) {
      morphToAndReturnRookie(felt, mem);
    } else {
      mem->carryingFood = 8;
      gotoBase(mem);
    }
  }
  gotoDest(mem);
}

int DocWarReporter(struct SquareData *felt, AntMem *mem) {
  AntMem *queen;
  int r;
  if (r=DocEnemyInSight(felt)) return r;
  if (zeroPos(mem)) {
    if ((queen = DocGetAnt(felt, mem, QUEEN)) != NULL) {
      queen->u.q.enemyReportCount++;
    }
    morphToAndReturnRookie(felt, mem);
  }
  gotoDest(mem);
}

int DocFoodGuard(struct SquareData *felt, AntMem *mem) {
  int r;
  if (r=DocEnemyInSight(felt)) return r;
  if (!mem->u.fg.timeToRetire--) {
    //aabn for nye food reportere.
    mem->u.fg.isReported = 0;
    mem->u.fg.timeToRetire = 3*baseDist(mem->pos);
  }
  if (destReached(mem) && felt->NumFood<2) {
    mem->rang = COLLECTOR;
    return DocCollector(felt, mem);
  }
  gotoDest(mem);
}

int DocFoodReporter(struct SquareData *felt, AntMem *mem) {
  int r;
  if (r=DocEnemyInSight(felt)) return r;
  gotoDest(mem);
}

int DocQueen(struct SquareData *felt, AntMem *mem) {
  int i,j,a,b;
  AntMem *ant;
  /* if (felt->NumAnts==100) {
    printf("\nBasen er fyldt med myrer!\n");
    for (i=0; i<100; i++) printf("Myre %2d : %s\n",i,DocAntNames[mem[i].rang]);
    exit(0);
  } */
  mem->u.q.lastExpanded++;
  //Reducer antallet af food reportere
  ant=NULL;
  for (i=1; i<felt->NumAnts; i++) {
    if (mem[i].rang==FOOD_REPORTER && mem[i].u.fr.reportCount<MAX_REPORT_COUNT) {
      if (ant != NULL) {
	while (ant->u.fr.reportCount < MAX_REPORT_COUNT  &&  mem[i].u.fr.reportCount>0)
	  ant->u.fr.report[ant->u.fr.reportCount++] = mem[i].u.fr.report[--mem[i].u.fr.reportCount];
	if (mem[i].u.fr.reportCount) {
	  ant = &mem[i];
	} else {
	  mem[i].rang = ROOKIE;
	  mem[i].carryingFood = 0;
	}
      } else ant = &mem[i];
    }
  }
  return 16;//Forsoeg altid at bygge ny base
}

int Doc(struct SquareData *felt, AntMem *mem) {
  int r;
  AntMem *queen;
  //for (r=0; r<100000; r++);
  //printf("%s\n",antNames[mem->rang]);
  switch (mem->rang) {
  case ROOKIE       : r = DocRookie(felt, mem);       break;
  case EXPLORER     : r = DocExplorer(felt, mem);     break;
  case SOLDIER      : r = DocSoldier(felt, mem);      break;
  case COLLECTOR    : r = DocCollector(felt, mem);    break;
  case WAR_REPORTER : r = DocWarReporter(felt, mem);  break;
  case FOOD_GUARD   : r = DocFoodGuard(felt, mem);    break;
  case FOOD_REPORTER: r = DocFoodReporter(felt, mem); break;
  case QUEEN        : r = DocQueen(felt, mem);        break;
  }
  //printf("%s+%d\n",antNames[mem->rang],r);
  if (felt[r].NumAnts != MaxSquareAnts) {
    switch (r) {
    case 4 :mem->pos.y++; break;
    case 1 :mem->pos.x++; break;
    case 2 :mem->pos.y--; break;
    case 3 :mem->pos.x--; break;
    }
  }
  mem->turn++;
  return r | mem->carryingFood;
}

DefineAnt(TheDoctor, "The Doctor", Doc, struct DocBrain)
