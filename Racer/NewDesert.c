//Lavet af Jesper Torp Kristensen, u993197.

//#include <stdio.h>
//#include <string.h>
#include "Myre.h"

#define AntMem struct NewDBrain

#define NULL 0

#define ushort unsigned short
#define ulong unsigned long
#define schar signed char

#define north 4
#define south 2
#define east 1
#define west 3

#define INITIAL_RADIUS 60
#define RADIUS_STEP 20
#define MAX_FOOD_REPORTS 32
#define HASH_TABLE_SIZE 64//Maa ej aendres, uden at tjekke koden.
#define MAX_KNOWLEDGE_QUEEN 32
//Naar en soldat naar SOLDIER_BRAKE*enemyDist/256 felter ud,
//begynder den at geare (liniaert) ned, indtil soldaten har
//naaet enemyDist ud, og har antaget gnms.hastigheden
// 1/SOLDIER_INVERSE_COMBAT_SPEED
#define SOLDIER_BRAKE 200
#define SOLDIER_INVERSE_COMBAT_SPEED 8
//Naar en soldat naar SOLDIER_STOP*enemyDist/256 felter ud, stopper den helt.
#define SOLDIER_STOP 320
#define SOLDIER_WAIT 1000 //antal ture uden kamp der skal gaa, for at en soldat
//stopper sin karriere.
#define AGGRESSIVITY 4 //stoerrelsen af en haerenhed
#define MINIMUM_ANTS_BORN_PR_ENEMY_REPORT 4

#define abs(x) ((x)<0 ? -(x) : (x))
#define sign(x) ((x)<0 ? -1 : ((x)==0 ? 0 : 1))
#define min(x,y) ((x)<(y) ? (x) : (y))
#define max(x,y) ((x)<(y) ? (y) : (x))
#define dist(x1,x2) (abs((x1).x - (x2).x)+abs((x1).y - (x2).y))
#define zeroPos(mem) (!((mem)->pos.x  ||  (mem)->pos.y))
#define baseDist(xx) (abs((xx).x)+abs((xx).y))
#define destReached(mem) (((mem)->pos.x==(mem)->dest.x) && ((mem)->pos.y==(mem)->dest.y))
#define showStatus(name, mem) printf("%s is moving from x=%d y=%d\n",(name),(mem)->pos.x,(mem)->pos.y)
#define hashFunction(pos) (((pos).x ^ ((pos).y << 8)) & 65535)
#define morphToRookie(rookie) (rookie->rang) = ROOKIE; (rookie)->carryingFood = 0;
#define gotoBase(mem) NewDInitGotoDest(mem,0,0);
#define posEqual(p1,p2) ((p1).x==(p2).x  &&  (p1).y==(p2).y)

//defines for at lette notationen
#define pq(primaryQueen) ((primaryQueen)->r.pQueen)
#define kq(knowledgeQueen) ((knowledgeQueen)->r.kQueen)
#define hq(hashtableQueen) ((hashtableQueen)->r.htQueen)
#define kr(pQueen,index) (kq(pq(pQueen).kQueen[(unsigned int)(index)>>5]).report[(index)&31])//knowledge report
#define reportDist(pQueen,index) baseDist(kr(pQueen,index).pos)
#define rr(reporterReport) ((reporterReport)->r.reporter.report)
#define so(Soldier) ((Soldier)->r.soldier)
#define co(Collector) ((Collector)->r.collector)

#define ROOKIE 0
#define SURRENDER 1
#define COLLECTOR 2
#define SOLDIER 3
#define REPORTER 4
#define KNOWLEDGE_QUEEN 5
#define HASH_TABLE_QUEEN 6
#define PRIMARY_QUEEN 7

char *NewDAntNames[]={"ROOKIE", "SURRENDER", "COLLECTOR", "SOLDIER", "REPORTER", "KNOWLEDGE QUEEN", "HASH TABLE QUEEN", "PRIMARY QUEEN"};

struct NewDPos{
  short x,y;
};

struct NewDReport{
  struct NewDPos pos;
  char amount;
  char isFood;//1=>Food  0=>Army
};

struct NewDMoveData{
  short absdx, absdy;
  char returnX, returnY;
};

struct NewDBrain{
  ulong randseed;
  char carryingFood;//er enten 0 eller 8
  int rang;
  ushort turn;
  struct NewDPos pos,dest;
  struct NewDMoveData moveData;
  //AntMove antMove;
  union{
    struct{
    } rookie;
    struct{
      //Hvis basen mistes, opgives spillet.
      //Surrender myrerne skal absolut ingenting foretage sig.
      //Herved undgaas at skrive besvaerlig kode til denne
      //situation, der alligel naesten altid vil vaere tabt.
    } surrender;
    struct{
      int receivedReport;
    } collector;
    struct{
      //Vigtigt: foodReport skal vaere foerste attribut
      //(Udnyttes under morphingen til dronning)
      struct NewDReport report;
    } reporter;
    struct{
      int enemyDist;
      int brakeDist;
      int enemyMinusBrakeDist;
      int ownDist;
      int lastBattleTurn;
    } soldier;
    struct{
      //Vigtigt: foodReport skal vaere foerste attribut
      //Oversigt over, hvor der skal sendes myrer hen:
      struct NewDReport report[MAX_FOOD_REPORTS];
      int index;//Hvor i primary queen's referenceliste denne dronning skal ligge.
    } kQueen;
    struct{
      //Hashtabel over for nyligt opdagede (og udtoemte) madomraader.
      //Herved undgaas foelgende scenarie:
      //myre A opdager mad og rapporter dette til basen.
      //Herfra sendes der straks det paakraevede myrer ud for at hente maden.
      //I mellemtiden har myre B opdaget det samme mad.
      //Denne myre rapporterer fundet til basen, som i mellemtiden har
      //glemt alt om lokationen, da det paakraevede myrer jo er sendt af sted.
      //Resultatet er, at en ny gruppe myre sendes af sted forgaeves.
      short hashTable[HASH_TABLE_SIZE];
      char hashTableIndex;
    } htQueen;
    struct{
      //Foelgende pointere skal oprettes HVER gang dronningen benyttes.
      AntMem *htQueen;
      AntMem *kQueen[MAX_KNOWLEDGE_QUEEN];
      int kQueenCount;//Egentlig unoedvendig variabel, men sikkert rar at have under debugging.
      int reportCount;
      int radius;
      int timeToExpand;//I de naeste timeToExpand ture, kan radius ej udvides.
      int antsBorn,enemyReports;
    } pQueen;
  } r;
};

ushort NewDGetRand(AntMem *mem){
  //returnerer et pseudo tilfældigt tal mellem 0 og 65535.
  mem->randseed = mem->randseed*1103515245+12345;
  return (ushort)((mem->randseed>>16) & 65535);
}

ushort NewDGetRandMax(AntMem *mem, ushort max){
  //returnerer et pseudo tilfældigt tal mellem 0 og max-1
  mem->randseed = mem->randseed*1103515245+12345;
  return ((mem->randseed >> 16)*max + (((mem->randseed & 65535)*max) >> 16)) >> 16;
}

int NewDGotoDest(AntMem *mem) {
  if (destReached(mem)) return 0;
  if (mem->moveData.absdy*abs(mem->dest.x - mem->pos.x) <
      mem->moveData.absdx*abs(mem->dest.y - mem->pos.y)) {
    //Vertical movement.
    return mem->moveData.returnY;
  } else {
    //Horizontal movement.
    return mem->moveData.returnX;
  }
}

void NewDInitGotoDest(AntMem *mem, int destX, int destY) {
  mem->dest.x = destX;
  mem->dest.y = destY;
  mem->moveData.absdx = abs(mem->pos.x - destX);
  mem->moveData.absdy = abs(mem->pos.y - destY);
  mem->moveData.returnX = (mem->pos.x < destX ? east : west);
  mem->moveData.returnY = (mem->pos.y < destY ? north : south);
}

void NewDAssignAntToPosOnRhombe(struct NewDBrain *mem, int rhombeRadius, int pos) {
  //Saetter myrens dest til det punkt paa rhomben, med radius rhombeRadius og origo 0,
  //som naas ved at starte i (rhombeRadius, 0) og dernaest foretage pos skraa skridt
  //i yderkanten af rhomben imod urets retning.
  if (pos<rhombeRadius) {
    NewDInitGotoDest(mem, rhombeRadius-pos, pos);
    return;
  }
  pos-=rhombeRadius;
  if (pos<rhombeRadius) {
    NewDInitGotoDest(mem, -pos, rhombeRadius-pos);
    return;
  }
  pos-=rhombeRadius;
  if (pos<rhombeRadius) {
    NewDInitGotoDest(mem, pos-rhombeRadius, -pos);
    return;
  }
  pos-=rhombeRadius;
  NewDInitGotoDest(mem, pos, pos-rhombeRadius);
}

int NewDEnemyInSight(struct SquareData *felt, AntMem *mem) {
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
  if (bestHitPos && mem->rang != SOLDIER) {
    //Der er fundet en fjende!
    //Myrens tidligere rang og status slettes.
    //En madrapport kan herved gaa tabt, men sandsynlighed for at en rapporter
    //kommer i krig er ikke saerlig stor, da de jo altid er paa vej mod basen.
    mem->rang = REPORTER;
    rr(mem).pos = mem->pos;//Egentlig er det ikke dette felt,
    //men 1 felt fra eller til betyder ikke saa meget i denne sammenhaeng.
    rr(mem).amount = AGGRESSIVITY;
    rr(mem).isFood = 0;
    gotoBase(mem);//Men i denne tur skal fjenden naturligvis slaas.
  }
  return bestHitPos;
}

struct NewDBrain *NewDGetAnt(struct SquareData *felt, AntMem *mem, int antRang) {
  //Finder og returnerer den foerste myre paa feltet med rang=antRang.
  //Hvis ingen myre har den givne rang, returneres NULL.
  int i;
  for (i=1; i<felt->NumAnts; i++)
    if (mem[i].rang == antRang) return &(mem[i]);
  return NULL;
}

AntMem *NewDFindQueenOrSurrender(struct SquareData *felt, AntMem *mem) {
  //Returnerer true, hvis myren skal overgive sig.
  int i;
  AntMem *queen = NewDGetAnt(felt, mem, PRIMARY_QUEEN);
  if ((queen) == NULL) {
    (mem)->rang == SURRENDER;
    return NULL;
  }
  for (i=0; i<felt->NumAnts; i++) {
    if (mem[i].rang == KNOWLEDGE_QUEEN) {
      //Opdater pointeren til denne knowledge queen.
      pq(queen).kQueen[kq(&mem[i]).index] = &mem[i];
    } else if (mem[i].rang == HASH_TABLE_QUEEN) {
      //Opdater pointeren til hash table dronningen.
      pq(queen).htQueen = &mem[i];
    }
  }
  //printf("pq(queen).kQueen[0] = %p, id=%d\n",pq(queen).kQueen[0],queen->randseed);
  return queen;
}

int NewDTjekForNewFoodDepot(struct SquareData *felt, AntMem *mem) {
  int i;
  if (felt->NumFood >= felt->NumAnts) {
    mem->carryingFood = 8;
    if (co(mem).receivedReport || felt->NumFood==felt->NumAnts) {
      //Hvis felt->NumFood==felt->NumAnts:
      //Hvis der rapporteres til basen om 0 mad, saa vil der opstaa fejl.
      //Transporter derfor blot maden hjem
      gotoBase(mem);
      mem->rang = COLLECTOR;//if felt->NumFood==felt->NumAnts
      return NewDCollector(felt, mem);
    } else {
      //Ny mad fundet! Lav myren til FOOD_REPORTER
      mem->rang = REPORTER;
      rr(mem).pos = mem->pos;
      rr(mem).amount = felt->NumFood-felt->NumAnts;
      rr(mem).isFood = 1;
      gotoBase(mem);
      return NewDReporter(felt, mem);
    }
  }
  return 0;
}

int NewDTjekForNearFoodSurplus(struct SquareData *felt) {
  //Hvis der er madoverskud i naerheden, returneres retningen.
  //Ellers returneres 0.
  int i;
  for (i=1; i<5; i++)
    if (felt[i].NumAnts < felt[i].NumFood) return i;
  return 0;
}

int NewDCommitReport(struct SquareData *felt, AntMem *mem) {
  int i,newDist,newHash;
  AntMem *queen;

  if ((queen=NewDFindQueenOrSurrender(felt, mem))==NULL) return 0;

  //printf("reportCount %d\n",pq(queen).reportCount);

  if (rr(mem).isFood) {
    //Ligger madrapporten allerede i databasen?
    for (i=pq(queen).reportCount; i>0; i--) {
      if (posEqual(rr(mem).pos, kr(queen, i).pos)) {
	//Rapporten ligger der i forvejen
	//Tael amount ned med 1, medmindre den herved kommer ned paa 0
	//(saa skal der nemlig foretages noget oprydningsarbejde).
	if(!--kr(queen, 1).amount) kr(queen, 1).amount++;
	morphToRookie(mem);
	return NewDRookie(felt,mem);
      }
    }
    
    //Ligger madrapporten i hashtabellen?
    newHash = hashFunction(rr(mem).pos);
    for (i=0; i<HASH_TABLE_SIZE; i++) {
      if (newHash == hq(pq(queen).htQueen).hashTable[i]) {
	//madrapporten er med sandsynlighed > 99% tidligere blevet rapporteret.
	morphToRookie(mem);
	return NewDRookie(felt,mem);
      }
    }
  } else {
    pq(queen).enemyReports++;
  }

  //Der er med stor sandsynlighed tale om en ny rapport.
  //printf("New report\n");
  //Rapporterne i dronningens database er repraesenteret vha. et heap.
  //Foer den nye rapport placeres, undersoeges der, hvor den skal ligge.
  newDist = baseDist(rr(mem).pos);
  i = ++pq(queen).reportCount;//Husk at position 0 ej bruges i en heap.
  while (i>1 && newDist < reportDist(queen,(unsigned int)i>>1)) {
    //Afstanden til rapport i>>1 er stoerre end afstanden til den nye rapport.
    //rapport i>>1 kopieres derfor over i rapport i, hvor den nye rapport
    //ellers skulle have vaeret indsat.
    kr(queen,i) = kr(queen,(unsigned int)i>>1);
    (unsigned int)i>>=1;
  }
  //Indsaet den nye rapport i heapen paa det korrekte sted.
  kr(queen,i) = rr(mem);

  //Skal der oprettes en ny knowledge myre?
  if ((pq(queen).reportCount & 31) == 31) {
    //Ja, for databasen er fyldt ud og der skal altid vaere plads til at
    //indsaette en rapport.
    //printf("%d. Opret knowledge myre %d\n",pq(queen).reportCount,pq(queen).kQueenCount);
    mem->rang = KNOWLEDGE_QUEEN;
    kq(mem).index = pq(queen).kQueenCount;
    pq(queen).kQueen[pq(queen).kQueenCount++] == mem;
    return 0;//return NewDKnowledgeQueen(felt,mem);
  } else {
    morphToRookie(mem);
    return NewDRookie(felt,mem);
  }
}

void NewDPrintAnts(struct SquareData *felt, AntMem *mem) {
  int i;
  //printf("\nPaa det aktuelle felt er der flg. myrer:\n");
  //for (i=0; i<felt->NumAnts; i++) printf("%s\n", NewDAntNames[mem[i].rang]);
  //printf("\n");
}

int NewDGetAssignment(struct SquareData *felt, AntMem *mem) {
  int i,dist,reportCount,rnd;
  AntMem *queen;
  struct NewDReport report;

  if ((queen=NewDFindQueenOrSurrender(felt, mem)) == NULL) return 0;

  if (!mem->turn) pq(queen).antsBorn++;

  if (pq(queen).reportCount) {
    //Der er en mad/fjende rapport at udfoere.
    //NewDPrintAnts(felt,mem);
    report = kr(queen, 1);
    //printf("Report.amount = %d\n",report.amount);
    if (--report.amount <= 0) {
      //printf("Deleting report nr %d...\n", pq(queen).reportCount);
      //Rapporten skal slettes fra heapen. Tilfoej den dog foerst til hashtabellen.
      hq(pq(queen).htQueen).hashTableIndex++;
      hq(pq(queen).htQueen).hashTable[hq(pq(queen).htQueen).hashTableIndex &= (HASH_TABLE_SIZE-1)]
	=hashFunction(report.pos);
      //Naar der slettes fra en heap, skal det sidste element flyttes til
      //position 1, og herfra rokeres i position.
      report = kr(queen, pq(queen).reportCount);
      pq(queen).reportCount--;//MAA IKKE UDFOERES I MAKROEN OVENFOR
      dist = baseDist(report.pos);
      reportCount = pq(queen).reportCount;
      i=1;
      //printf("...deleting continued...\n");
      while((i<<1 <= reportCount) && ((dist > reportDist(queen, i<<1)) ||
	    (i<<1+1 <= reportCount && dist > reportDist(queen, i<<1+1)))) {
	//Den korrekte lokation til report er stadig ikke fundet.
	if (dist > reportDist(queen, i<<1)) {
	  //(i<<1 <= reportCount) er true og report i og i<<1 staar forkert
	  //i forhold til hinanden.
	  kr(queen, i) = kr(queen, i<<1);
	  i<<=1;
	} else {
	  //Saa maa der jo gaelde at
	  //i<<1+1 <= reportCount && dist > reportDist(queen, i<<1+1)
	  kr(queen, i) = kr(queen, i<<1+1);
	  i<<=1; i++; //i=2*i+1
	}
      }
      //printf("...deleting continued...\n");
      //report, der foer stod sidst i heapen, skal nu indsaettes paa position i.
      kr(queen, i) = report;
      //heapen er nu faerdigopdateret.

      if ((pq(queen).reportCount & 31) == 30) {
	//Der kan frigoeres en knowledge queen til andre formaal.
	//printf("Delete knowledge queen\n");
	pq(queen).kQueenCount--;
	//printf("%d %p %p\n",pq(queen).kQueenCount,pq(queen).kQueen[0],pq(queen).kQueen[1]);
	morphToRookie(pq(queen).kQueen[pq(queen).kQueenCount]);
      }
      //printf("...deleting report done. %d reports left\n",pq(queen).reportCount);
    } else {
      kr(queen, 1).amount--;//foer blev der blot talt ned i en kopi.
    }
    if (report.isFood) {
      mem->rang = COLLECTOR;
      NewDInitGotoDest(mem, report.pos.x, report.pos.y);
      //if (!(report.pos.x || report.pos.y)) printf("report for pos (0,0)\n");
      co(mem).receivedReport = 1;
      return NewDCollector(felt, mem);
    } else {
      //!isFood => isEnemy
      mem->rang = SOLDIER;
      so(mem).lastBattleTurn = 0;
      //Der vaelges et tilfaeldigt punkt "uendelig" langt vaek som destination.
      //Soldaten bevaeger sig i omtrent den retning, hvor fjenden blev set.
      //Eller mere praecist:
      //abs(theta(EnemySpotted)-theta(Soldier.Direction)) <= tan^-1(1/8)
      rnd = NewDGetRandMax(queen,17)-8;
      NewDInitGotoDest(mem, (report.pos.x<<6)-rnd*report.pos.y, (report.pos.y<<6)+rnd*report.pos.x);
      so(mem).ownDist = 0;
      so(mem).enemyDist = baseDist(report.pos);
      so(mem).brakeDist = (unsigned int)(so(mem).enemyDist * SOLDIER_BRAKE) >> 8;
      so(mem).enemyMinusBrakeDist = so(mem).enemyDist - so(mem).brakeDist;
      return NewDSoldier(felt, mem);
    }
  } else {//if (pq(queen).reportCount)
    //Dronningen har ikke flere rapporter at dele ud af.
    if (!pq(queen).timeToExpand) {
      if (pq(queen).antsBorn > MINIMUM_ANTS_BORN_PR_ENEMY_REPORT*pq(queen).enemyReports) {
	//Det er paa tide at omvide territoriet.
	pq(queen).radius += RADIUS_STEP;
	pq(queen).timeToExpand = pq(queen).radius << 1;
      }
    }
    //Myren sendes i en tilfældig retning.
    mem->rang = COLLECTOR;
    mem->r.collector.receivedReport = 0;
    NewDAssignAntToPosOnRhombe(mem, pq(queen).radius,NewDGetRandMax(queen, pq(queen).radius << 2));
    return NewDCollector(felt, mem);
  }
}

int NewDRookie(struct SquareData *felt, AntMem *mem) {//Boer virke
  int i,queenFound=0;
  struct NewDBrain *queen;

  //Skal basen beskyttes?
  if (i=NewDEnemyInSight(felt,mem)) return i;

  //Er myren ikke paa basen? (efter at have slaaet en fjende)
  if (!zeroPos(mem)) {
    gotoBase(mem);
    //printf("Ant was in combat\n");
    return NewDGotoDest(mem);
  }
  if ((queen = NewDGetAnt(felt,mem,PRIMARY_QUEEN)) != NULL) {
    //report to queen for duty
    return NewDGetAssignment(felt, mem);
  }
  //Der er ingen dronning.
  if (mem->turn) {
    //Ej i starten i spillet => Basen er mistet, da dronningerne oprettes straks
    //printf("Surrender\n");
    (mem)->rang == SURRENDER;
    return 0;
  } else {
    //Undersoeg om der er myrer nok til mange dronninge strategien.
    if (felt->NumAnts < 4) {
      //Det er der ikke. Opgiv derfor! TODO: Lav overlevelsesstrategi.
      for (i=0; i<felt->NumAnts; i++) mem[i].rang = SURRENDER;
      return 0;
    }
    //Dette er den foerste myre, der laver sit foerste traek.
    //Der skal straks uddelegeres 3 poster:
    //En primary queen, en knowledge queen, og en hash table queen.
    //Da denne myre er foerst ude, er det kun retfaerdigt, at den
    //faar lov til at vaere primary queen.
    mem->rang = PRIMARY_QUEEN;
    pq(mem).antsBorn = 0;
    pq(mem).enemyReports = 0;
    pq(mem).kQueenCount = 1;
    pq(mem).reportCount = 0;
    pq(mem).radius = INITIAL_RADIUS;
    mem[1].rang = HASH_TABLE_QUEEN;
    mem[2].rang = KNOWLEDGE_QUEEN;
    kq(&mem[2]).index = 0;
    //Resten af disse myrers attributter behoever ikke at blive sat,
    //da startvaerdien paa 0 er ok.

    //Fordel resten af myrene uniformt ud i en cirkel med radius INITIAL_RADIUS
    for (i=3; i<felt->NumAnts; i++) {
      mem[i].rang = COLLECTOR;
      NewDAssignAntToPosOnRhombe(&(mem[i]), INITIAL_RADIUS, (i*INITIAL_RADIUS<<2)/felt->NumAnts);
    }
    return NewDPrimaryQueen(felt,mem);
  }
}

int NewDSoldier(struct SquareData *felt, AntMem *mem) {//Boer virke
  int r;

  if (r=NewDEnemyInSight(felt,mem)) {
    so(mem).lastBattleTurn = mem->turn;
    return r;
  }

  if (so(mem).lastBattleTurn + SOLDIER_WAIT < mem->turn) {
    mem->rang = COLLECTOR;
    gotoBase(mem);
    return NewDCollector(felt, mem);
  }

  if (so(mem).lastBattleTurn || ((SOLDIER_STOP*so(mem).enemyDist)>>8 < so(mem).ownDist))
    return 0;//Saa stopper du!

  if (so(mem).ownDist <= so(mem).brakeDist) {
    //Soldaten er stadig langt fra krigen, saa den skal skynde sig.
    so(mem).ownDist++;
    return NewDGotoDest(mem);
  }

  if (so(mem).ownDist < so(mem).enemyDist) {
    //Soldaten er taet paa krigsomraadet, saa den skal bevaege sig stadig forsigtigere.
    if (NewDGetRandMax(mem, SOLDIER_INVERSE_COMBAT_SPEED) <=
	1+((SOLDIER_INVERSE_COMBAT_SPEED-2)*(so(mem).enemyDist - so(mem).ownDist))/
	so(mem).enemyMinusBrakeDist) {
      //Soldaten skal bevaege sig i denne tur.
      so(mem).ownDist++;
      return NewDGotoDest(mem);
    } else {
      //Soldaten holder en pause.
      return 0;
    }
  }

  //Soldaten er naaet ind i krigsomraadet, og skal bevaege sig varsomt.
  if (NewDGetRandMax(mem, SOLDIER_INVERSE_COMBAT_SPEED)) {
    //Soldaten holder en pause.
    return 0;
  } else {
    //Soldaten skal bevaege sig i denne tur.
    so(mem).ownDist++;
    return NewDGotoDest(mem);
  }
}

int NewDCollector(struct SquareData *felt, AntMem *mem) {
  int r,temp;
  struct NewDBrain *queen;

  if (r=NewDEnemyInSight(felt, mem)) return r;

  if (destReached(mem)) {
    if (zeroPos(mem)) {
      //En collector en vendt tilbage til basen, med eller uden mad.
      morphToRookie(mem);
      return NewDRookie(felt, mem);
    } else {
      //Is not carrying food (else -> base)
      if (co(mem).receivedReport && (felt->NumFood < felt->NumAnts)) {
	//Lad vaere med at tage praecist den samme vej tilbage til basen.
	NewDInitGotoDest(mem, sign(mem->pos.x)*NewDGetRandMax(mem,abs(mem->pos.x)),
			 sign(mem->pos.y)*NewDGetRandMax(mem,abs(mem->pos.y)));
      }
      gotoBase(mem);

    }
  }
  if (mem->carryingFood) return NewDGotoDest(mem);
  if (temp = NewDTjekForNewFoodDepot(felt,mem)) return temp;
  if (temp = NewDTjekForNearFoodSurplus(felt)) return temp;
  return NewDGotoDest(mem);
}

int NewDReporter(struct SquareData *felt, AntMem *mem) {
  int r;
  if (!zeroPos(mem)) {
    if (r=NewDEnemyInSight(felt, mem)) return r;
    return NewDGotoDest(mem);
  }
  //Have returned to base
  return NewDCommitReport(felt, mem);
}

int NewDPrimaryQueen(struct SquareData *felt, AntMem *mem) {
  //Dronningen feder den bare for vildt!!!
  if (pq(mem).timeToExpand) pq(mem).timeToExpand--;
  return 0;
}

int NewDAnt(struct SquareData *felt, struct NewDBrain *mem) {
  switch (mem->rang) {
  case ROOKIE: return NewDRookie(felt, mem);
  case SURRENDER: return 0;
  case COLLECTOR: return NewDCollector(felt, mem);
  case SOLDIER: return NewDSoldier(felt, mem);
  case REPORTER: return NewDReporter(felt, mem);
  case KNOWLEDGE_QUEEN: return 0;
  case HASH_TABLE_QUEEN: return 0;
  case PRIMARY_QUEEN: return NewDPrimaryQueen(felt, mem);
  }
}

int NewDesert(struct SquareData *felt, AntMem *mem) {
  int x,y,a,r;
  x = mem->pos.x;
  y = mem->pos.y;
  //  printf("%s\n",NewDAntNames[mem->rang]);
  r = NewDAnt(felt,mem);
  //if (x!=mem->pos.x || y!=mem->pos.y) printf("Pos has changed illegally!\n");
  //if (zeroPos(mem) && !felt->Base) printf("Pos (0,0) men ingen base?\n");
  //if (zeroPos(mem) && felt->Base && mem->rang!=QUEEN) printf("POS (0,0) & Base & !Queen\n");
  //if (mem->rang==QUEEN && r) printf("Queen is moving\n");
  for (a=0; a<2000; a++);
  (mem->turn)++;
  if (felt[r&7].NumAnts < MaxSquareAnts) {
    switch (r&7) {
    case north:mem->pos.y++; break;
    case east :mem->pos.x++; break;
    case south:mem->pos.y--; break;
    case west :mem->pos.x--; break;
    }
  } else return 0;
  /*  if (abs(mem->pos.x)>300) {
    printf("dest.x=%d  dest.y=%d\n",mem->dest.x,mem->dest.y);
    printf("pos.x=%d  pos.y=%d\n",mem->pos.x,mem->pos.y);
    printf("absdx=%d  absdy=%d\n",mem->moveData.absdx,mem->moveData.absdy);
    printf("%s\n\n",NewDAntNames[mem->rang]);
    for (a=0; a<2000000; a++);
    } */
  //  if (!(mem->dest.x || mem->dest.y) && mem->rang==COLLECTOR && co(mem).receivedReport) printf("%s heading for (0,0)\n",NewDAntNames[mem->rang]);
  return r | mem->carryingFood;
}

DefineAnt(NewDesert, "New Desert", NewDesert, struct NewDBrain)
