/* IDEER:
   Scout's skal sik-sakke
   Slaves skal kunne rapportere mad de har set
   Perimeter search

DONE
   Scout's skal kunne gå et sted hen og så søge efter mad

*/

#include "Myre.h"

extern u_short CurrentTurn;

#undef AntFunc
#undef AntMem
#undef AntName
#undef Random(a)
#undef QUEEN
#undef NANNY
#undef WORKER
#undef SQUAD
#undef GUARD
#undef SCOUT

//Author: Anders Rosendal Denmark Juli/Aug 1999

#define Random(a) Borg_Random((a),&mem->random)
#define QMax(a,b) ((a)<(b) ? b:a)
#define QMin(a,b) ((a)>(b) ? b:a)

#define QUEEN 1
#define NANNY 2
#define SLAVE 3
#define SQUAD 4
#define GUARD 5
#define SCOUT 6
//#define go_to(x,y) Borg_goto(mem,x,y)
//#define go_rand(x,y) Borg_gotoRand(mem, x, y)

#define workmem mem->brain.worker
#define queenmem mem->brain.queen
#define FOODSIZE 20
#define NANNYSIZE 5
#define QDANGER 100
#define QDANGERMAX 500
#define SLAVEMIN 40
#define SERVICETIME1 180
#define SERVICETIME2 350

#define QIDLE 122
#define QGETFOOD 123
#define QNOFOODFOUND 124
#define QGOINGHOME 125
#define QSEEKING 126
#define QFOUNDFOOD 127
#define QHAVEFOOD 128

//union REGS i86regs;

struct BorgPrivData {
  u_long random;
  char klasse;
  int active;
  union {
    struct {
      short foodx[FOODSIZE],foody[FOODSIZE],foodnumber[FOODSIZE];
      short datas;
      unsigned int turn;
    } queen;
    struct {
      short xpos, ypos, xdest, ydest, foodx, foody, foodnumber;
      short danger, status, workturns, retning, turnchance;
    } worker;
  } brain;
};

#define MAX(a,b) (a)>(b) ? (a) : (b)
#define MIN(a,b) (a)<(b) ? (a) : (b)
inline int Borg_abs(int tal);
u_long Borg_Random(int tal, u_long *randval);
int Borg_goto(struct BorgPrivData *mem, int x, int y);
int Borg_gotorand(struct BorgPrivData *mem, int x, int y);

int QUEENFunc(struct SquareData *felter, struct BorgPrivData *mem);
int SLAVEFunc(struct SquareData *felter, struct BorgPrivData *mem);
int SQUADFunc(struct SquareData *felter, struct BorgPrivData *mem);
int SCOUTFunc(struct SquareData *felter, struct BorgPrivData *mem);

DefineAnt(borg, "The Borg", BorgFunc, struct BorgPrivData);


int BorgFunc(struct SquareData *felter, struct BorgPrivData *mem)
{
  int retval=0;
  int num,t;

//////////////////////////////////////////////////
  if(mem->klasse==GUARD && mem->active==424242) {
    num=0;
    for(t=1 ; t<=4 ; t++) {
      if(felter[t].Team!=felter[0].Team && felter[t].NumAnts>num) {
        retval=t;
        num=felter[t].NumAnts;
      }
    }
    return retval+8;
  }
//////////////////////////////////////////////////

  if(mem->klasse==QUEEN && mem->active==424242)
    return QUEENFunc(felter,mem);

  //BORN
  if(mem->klasse<1 || mem->klasse>6 || mem->active!=424242) {
    int result=0;
    mem->active=424242;
//    mem->id=Random(1000000);

    //ER DER EN DRONNING
    for(t=1; t<felter[0].NumAnts; t++)
      if(mem[t].klasse==QUEEN) {
        result=1;
        break;
      }
    //ASSUME COMMAND
    if(!result) {
      for(t=1; t<felter[0].NumAnts; t++) {
        mem[t].klasse=SCOUT;
        mem[t].active=424242;
//        mem[t].brain.worker.foodx=mem[t].brain.worker.foody=mem[t].brain.worker.foodnumber=0;
        mem[t].brain.worker.status=QSEEKING;
        mem[t].brain.worker.workturns=90;
        mem[t].brain.worker.turnchance=7;
        mem[t].brain.worker.xdest=(15+Random(10))*(Random(2) ? -1:1);
        mem[t].brain.worker.ydest=(15+Random(10))*(Random(2) ? -1:1);
      }/**/
      mem->klasse=QUEEN;
      for(t=0; t<FOODSIZE; t++)
        queenmem.foodx[t]=queenmem.foody[t]=queenmem.foodnumber[t]=0;
      queenmem.datas=0;
//////////////////////////////////////////////////
/*      if(Random(5)<2)
        for(t=0; t<felter[0].NumAnts; t++)
          mem[t].klasse=GUARD;/**/
//////////////////////////////////////////////////
      return 0;
    } else {
      mem->klasse=SLAVE;
      workmem.status=QIDLE;
    }
    workmem.danger=0;
  }

  //ER DER EN FJENDE I ET NABOFELT
  num=0;
  for(t=1; t<=4; t++) {
    if(felter[t].Team && felter[t].NumAnts>num) {
      retval=t;
      num=felter[t].NumAnts;
    }
  }
  if(num) {
    workmem.danger += QDANGER;
    workmem.danger=MAX(workmem.danger,QDANGERMAX/2);
    if(workmem.danger>QDANGERMAX) workmem.danger=QDANGERMAX;
    workmem.xpos+=(retval==1)-(retval==3);
    workmem.ypos+=(retval==2)-(retval==4);
    return retval+8;
  }

  if(mem->klasse==NANNY) {
    if(workmem.xpos || workmem.ypos) {
      retval=Borg_goto(mem,0,0);
      workmem.xpos+=(retval==1)-(retval==3);
      workmem.ypos+=(retval==2)-(retval==4);
      return retval;
    }
    return 0;
  }

  //IN COMBAT
  if(workmem.danger) {
    if(Borg_abs(workmem.xpos)+Borg_abs(workmem.ypos)>60)
      workmem.danger--;
    else
      workmem.danger-=5;
    if(felter->NumAnts==1 && felter->NumFood)
      workmem.danger=0;
    return 0;
  }

  switch(mem->klasse) {
    case SLAVE:
      return SLAVEFunc(felter,mem);
    case SQUAD:
      return SQUADFunc(felter,mem);
    case SCOUT:
      return SCOUTFunc(felter,mem);
    default:
//      exit(9);
  };
  mem->active=424242;
  mem->klasse=SLAVE;
  workmem.status=QGOINGHOME;
  return SLAVEFunc(felter,mem);
  return 2;
}

void B_insertfood(struct BorgPrivData *qmemory,struct BorgPrivData *smemory)
{
#define qmem qmemory->brain.queen
#define smem smemory->brain.worker
  int i,k,datas=qmem.datas;

/*  if(!smem.foodnumber) {
    usleep(200000);
    *((int *)0)=42;
  }
if(qmem.turn==346) {
i86regs.x.ax=3;
int86(0x10,&i86regs,&i86regs);
puts("insert1");
printf("\nx:%i y:%i num:%i\n",smem.foodx,smem.foody,smem.foodnumber);
printf("turn %i datas %i\n",qmem.turn,qmem.datas);
    for(i=0;i<qmem.datas;i++)
      printf("x %i y %i num %i\n",qmem.foodx[i],qmem.foody[i],qmem.foodnumber[i]);
exit(9);
} /**/

  i=0;
  while(i<datas) { //CHECK FOR DUBLET
    if(qmem.foodx[i]==smem.foodx && qmem.foody[i]==smem.foody) {
      qmem.foodnumber[i]=QMin(qmem.foodnumber[i],smem.foodnumber);
      i=-1;
			break;
    }
    i++;
  }
  if(datas<FOODSIZE && i!=-1) {
    qmem.foodx[datas]=smem.foodx;
    qmem.foody[datas]=smem.foody;
    qmem.foodnumber[datas]=smem.foodnumber;

    i=datas; //SORT - MODSTAMDERNES MAD SMAGER DOBBELT SÅ GODT (NÆSTEN)
    while(i>0 && (Borg_abs(qmem.foodx[i])+Borg_abs(qmem.foody[i])) <
                 (Borg_abs(qmem.foodx[i-1])+Borg_abs(qmem.foody[i-1])) ) {
            k=qmem.foodx[i];qmem.foodx[i]=qmem.foodx[i-1];qmem.foodx[i-1]=k;
            k=qmem.foody[i];qmem.foody[i]=qmem.foody[i-1];qmem.foody[i-1]=k;
            k=qmem.foodnumber[i];qmem.foodnumber[i]=qmem.foodnumber[i-1];qmem.foodnumber[i-1]=k;
      i--;
    }/**/
    qmem.datas++;
  }
#undef qmem
#undef smem
}

int QUEENFunc(struct SquareData *felter, struct BorgPrivData *mem)
{
  int t,i,j,k,x,y;
  static int firstfood=0;
//  static int bombarray[][] = {{40,40}, {50,30}, {}};

  queenmem.turn++;
  if(queenmem.turn==100)
    for(t=1; t<NANNYSIZE; t++)
      mem[t].klasse=NANNY;

/*  if(!Random(30) && queenmem.turn>400) {
    queenmem.foodx[queenmem.datas]=(20+Random(15))*(Random(2) ? -1:1);
    queenmem.foody[queenmem.datas]=(20+Random(15))*(Random(2) ? -1:1);
    queenmem.foodnumber[queenmem.datas++]=10;
  }/**/


  //COMMAND THE TROOPS!
  for(t=1; t<felter->NumAnts; t++) {
    //GET REPORTS ABOUT FOOD
    if(mem[t].klasse!=NANNY) {
      if(mem[t].klasse==SCOUT && mem[t].brain.worker.status==QFOUNDFOOD &&
                               mem[t].brain.worker.foodnumber)
        B_insertfood(mem,&mem[t]);

//      if(queenmem.foodnumber[0] && t<felter->NumAnts-3) {
      if(queenmem.foodnumber[0]) {
        firstfood=1;
        mem[t].klasse=SLAVE;
//        mem[t].random=(queenmem.foodx[0])*(queenmem.foody[0]);
//        mem[t].random=1234567;

        mem[t].brain.worker.status=QGETFOOD;
        mem[t].brain.worker.xdest=queenmem.foodx[0];
        mem[t].brain.worker.ydest=queenmem.foody[0];
        queenmem.foodnumber[0]--;
        if(queenmem.foodnumber[0]==0) {
          for(i=0;i<queenmem.datas-1; i++) {
            queenmem.foodx[i]=queenmem.foodx[i+1];
            queenmem.foody[i]=queenmem.foody[i+1];
            queenmem.foodnumber[i]=queenmem.foodnumber[i+1];
          }
          queenmem.datas--;
        }
//      } else if(t<felter->NumAnts-(7+SLAVEMIN* (queenmem.turn>1000 ? 1:0) )) {
      } else if(t<felter->NumAnts-(7*(firstfood ? 0:1) )) {
//      } else {
        mem[t].klasse=SCOUT;
        mem[t].brain.worker.status=QSEEKING;
        if(queenmem.turn<500) {
          mem[t].brain.worker.workturns=70;//SERVICETIME1;
          mem[t].brain.worker.turnchance=5;
          do {
            x=Random(35);
            y=Random(35);
          }while(Borg_abs(x)+Borg_abs(y)<35);
        } else if(queenmem.turn<1000) {
          mem[t].brain.worker.workturns=100;//SERVICETIME2-50;
          mem[t].brain.worker.turnchance=7;
          do {
            x=Random(45);
            y=Random(45);
          }while(Borg_abs(x)+Borg_abs(y)<42);
        } else if(queenmem.turn<1800) {
          mem[t].brain.worker.workturns=70;//SERVICETIME2+50;
          mem[t].brain.worker.turnchance=9;
          do {
            x=Random(45);
            y=Random(45);
          }while(Borg_abs(x)+Borg_abs(y)<42);
        } else {
          mem[t].brain.worker.workturns=SERVICETIME2;
          mem[t].brain.worker.turnchance=15;
          do {
            x=Random(60);
            y=Random(60);
          }while(Borg_abs(x)+Borg_abs(y)<42);
        }
        mem[t].brain.worker.xdest=x*(Random(2) ? -1:1);
        mem[t].brain.worker.ydest=y*(Random(2) ? -1:1);
      }/* else {
        mem[t].klasse=SLAVE;
        mem[t].brain.worker.status=QIDLE;
      }  */
    }
  }

  return 0;
}

int SLAVEFunc(struct SquareData *felter, struct BorgPrivData *mem)
{
  int retval=0,num,t;

  switch(workmem.status) {
    case QIDLE:   return 0;
    case QGETFOOD:
           if(workmem.xdest==workmem.xpos && workmem.ydest==workmem.ypos) {
             if(felter[0].NumFood) //ER JEG SKUFFET?
               workmem.status=QGOINGHOME;
             else {
//               workmem.status=QNOFOODFOUND;
               mem->klasse=SCOUT; //SPLIT AND SEEK
               workmem.turnchance=7;
               workmem.workturns=SERVICETIME2/2;
//               workmem.workturns=1000;
               workmem.status=QSEEKING;
               return SCOUTFunc(felter,mem);
             }
             workmem.xdest=workmem.ydest=0;
           }
           retval=Borg_gotodiag(mem,workmem.xdest,workmem.ydest);
           workmem.xpos+=(retval==1)-(retval==3);
           workmem.ypos+=(retval==2)-(retval==4);
           return retval+8*(workmem.status==QGOINGHOME);
    case QGOINGHOME:
    case QNOFOODFOUND:
           if(!workmem.xpos && !workmem.ypos && felter->Base==0) {
             return 16;
           }
           retval=Borg_goto(mem,workmem.xdest,workmem.ydest);
           workmem.xpos+=(retval==1)-(retval==3);
           workmem.ypos+=(retval==2)-(retval==4);
           return retval+8*(workmem.status==QGOINGHOME);
    default:
           workmem.status=QNOFOODFOUND;
           retval=Borg_goto(mem,0,0);
           workmem.xpos+=(retval==1)-(retval==3);
           workmem.ypos+=(retval==2)-(retval==4);
           return retval;
  }
}
int SQUADFunc(struct SquareData *felter, struct BorgPrivData *mem)
{
  int retval;
  retval=(workmem.xpos&1)^(workmem.ypos&1)+1;
  retval=workmem.retning;
  workmem.xpos+=(retval==1)-(retval==3);
  workmem.ypos+=(retval==2)-(retval==4);
  if(workmem.xpos==0 & workmem.ypos==0) {
    mem->active=424242;
    mem->klasse=SLAVE;
    workmem.status=QIDLE;
  }
  return retval;
}

int SCOUTFunc(struct SquareData *felter, struct BorgPrivData *mem)
{
  int retval,t,c;

  if(workmem.workturns==0) {
    if(workmem.status!=QFOUNDFOOD)
      workmem.status=QGOINGHOME;
    workmem.workturns=1;
  }

  if(workmem.status!=QFOUNDFOOD && workmem.status!=QHAVEFOOD) {
    retval=0;
    //ER DER MAD I ET NABOFELT?
    for(t=1; t<=4; t++)
      if(felter[t].NumFood>felter[t].NumAnts)
        retval=t;
    if(retval) {
      workmem.status=QFOUNDFOOD;
  
      workmem.xpos+=(retval==1)-(retval==3);
      workmem.ypos+=(retval==2)-(retval==4);

      workmem.foodx=workmem.xpos;
      workmem.foody=workmem.ypos;
      workmem.foodnumber=felter[retval].NumFood-1-felter[retval].NumAnts+5;
      workmem.xdest=workmem.ydest=0;
      return retval;
    }
  }
  if(workmem.foodnumber<2 && workmem.status==QFOUNDFOOD) {
    workmem.foodx=workmem.foody=workmem.foodnumber=0;
    workmem.status=QHAVEFOOD;
  }

  if(workmem.status==QGOINGHOME || workmem.status==QFOUNDFOOD || workmem.status==QHAVEFOOD) {
    if(workmem.xpos || workmem.ypos) {
      retval=Borg_goto(mem,workmem.xdest,workmem.ydest);
      workmem.xpos+=(retval==1)-(retval==3);
      workmem.ypos+=(retval==2)-(retval==4);
      return retval+8*(workmem.status==QFOUNDFOOD);
    } else if(workmem.status==QFOUNDFOOD) {
      //DO QUEEN UPDATES
      for(c=1; c<felter[0].NumAnts; c++)
        if(mem[c].klasse==QUEEN) { //FIND DRONNING
          B_insertfood(&mem[c],mem);
          break;
        }
      mem->klasse=SLAVE;
      workmem.status=QIDLE;
      return 0;
    }
    mem->active=424242;
    mem->klasse=SLAVE;
    workmem.status=QIDLE;
    return 0;
    //RAMPAGE
    mem->klasse=SQUAD;
    return SQUADFunc(felter,mem);
/*    retval=(workmem.xpos&1)^(workmem.ypos&1)+1;
    workmem.xpos+=(retval==1)-(retval==3);
    workmem.ypos+=(retval==2)-(retval==4);
    if(workmem.xpos==0 & workmem.ypos==0) {
      mem->active=424242;
      mem->klasse=SLAVE;
      workmem.status=QIDLE;
    }
    return retval; /**/
  }
  if(workmem.status!=QSEEKING) {
    workmem.status=QGOINGHOME;
    *((int *)0)=42;
    return 1;
//    exit(9);
  }

//DO WHAT QUEEN WANT US TO DO
  if(workmem.xdest || workmem.ydest) {
    if(workmem.xdest!=workmem.xpos && workmem.ydest!=workmem.ypos) {
      retval=Borg_goto(mem,workmem.xdest,workmem.ydest);
      workmem.xpos+=(retval==1)-(retval==3);
      workmem.ypos+=(retval==2)-(retval==4);
      return retval;
    } else
      workmem.xdest=workmem.ydest=0;
  }
  //QSEEKING
  workmem.workturns--;
//WALK ABOUT
  if(!workmem.retning) workmem.retning=Random(4)+1;
  if(!Random(workmem.turnchance))
//  if(!Random(8+30*(workmem.workturns>SERVICETIME2 ? 1:0)))
    workmem.retning=((workmem.retning+Random(3)-2)%4)+1;

  retval=workmem.retning;
  workmem.xpos+=(retval==1)-(retval==3);
  workmem.ypos+=(retval==2)-(retval==4);
  return retval;
}

int Borg_goto(struct BorgPrivData *mem, int x, int y)
{
  int dir[2];
  dir[0]=(workmem.xpos<x)+3*(workmem.xpos>x);
  dir[1]=2*(workmem.ypos<y)+4*(workmem.ypos>y);
  if(workmem.xpos==x)
    return dir[1];
  else if(workmem.ypos==y)
    return dir[0];
  else if(Borg_abs(workmem.xpos-x)>Borg_abs(workmem.ypos-y))
    return dir[0];
  else
    return dir[1];
}

int Borg_gotodiag(struct BorgPrivData *mem, int x, int y)
{
  int dir[2];
  dir[0]=(workmem.xpos<x)+3*(workmem.xpos>x);
  dir[1]=2*(workmem.ypos<y)+4*(workmem.ypos>y);
  if(workmem.xpos==x)
    return dir[1];
  else if(workmem.ypos==y)
    return dir[0];
  else if((Borg_abs(workmem.xpos-x)&1) ^ (Borg_abs(workmem.ypos-y)&1))
    return dir[0];
  else
    return dir[1];
}

int Borg_gotorand(struct BorgPrivData *mem, int x, int y)
{
  int dir[2];
  dir[0]=(workmem.xpos<x)+3*(workmem.xpos>x);
  dir[1]=2*(workmem.ypos<y)+4*(workmem.ypos>y);
  if(workmem.xpos==x)
    return dir[1];
  else if(workmem.ypos==y)
    return dir[0];
  else if(Random(2))
    return dir[0];
  else
    return dir[1];
//    return dir[Random(Borg_abs(workmem.xpos-x)+Borg_abs(workmem.ypos-y))<Borg_abs(workmem.ypos-y)];
}

int Borg_abs(int tal)
{
   return (tal<0 ? -tal : tal);
}


u_long Borg_Random(int tal, u_long *randval)
{
/*  u_long a;

  srand(*randval);
  a=rand();
  *randval=a;
  return (int) (tal*a/(RAND_MAX/4.0+1));
/*   u_long a = (*randval^77)*16811;
   *randval = (a = (a<<16)+(a>>16));
   return (tal? tal*a/1800000000 : 0);/**/
   u_long a = (*randval^77)*16811;
   *randval = (a = (a<<16)+(a>>16));
   return (tal? a%tal : 0);/**/
/*   u_long a = (*randval+7)*16807;
   *randval = (a = (a<<16)+(a>>16));
   return (tal? a%tal : 0);
/**/
//  return 1+(int) (a*rand()/(RAND_MAX+1));
}


/*if(CurrentTurn>240 && workmem.foodnumber==0) {
fprintf(stderr,"\n\nturn: %i id: %i\n",CurrentTurn,mem->id);
fprintf(stderr,"xpos: %i ypos: %i status: %i\n",workmem.xpos,workmem.ypos,workmem.status);
fprintf(stderr,"xfood: %i yfood: %i foodnumber: %i\n",workmem.foodx,workmem.foody,workmem.foodnumber);
fflush(stderr);
exit(9);
}/**/
