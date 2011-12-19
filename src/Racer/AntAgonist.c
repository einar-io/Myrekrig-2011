/* AntAgonist.c by Jacob Illum <illum@myrekrig.dk>
 * Jeg vil sige tak til Urkel der har været sparringspartner igennem udviklingen,
 * for inspirerende input og interessante diskussioner
 *
 * Beskrivelse:
 * I en verden af mørke, løgn, bedrag og "obfuscated" kode træder pludselig
 * AntAgonist ind i lyset. En ny type myre, en myre af ære, en myre der tør
 * vise sit sande jeg, en myre med en mission: "Udryddelsen af alle myrer med
 * ulæselig kode".
 *
 * Korstogets formål om åbenhed og reformation udspringer fra "lysets
 * katedral" og som budskabet spredes af stolte korsriddere opspringer nye
 * fraktioner omkring i landskabet.
 *
 */

#include "Myre.h"

#define go(m) (m+1)
#define goFood(m) (m+9)
#define isFoodInformer(m) ((m & 192) == 192)
#define makeFoodInformer(fooddir,dir,numfood) (192 | (fooddir<<5) | (dir<<3) | (numfood))
#define foodInformerDir(m) ((m & 24)>>3)
#define foodInformerNumFood(m) (m & 7)
#define foodInformerFoodDir(m) ((m & 32)>>5)
#define isInformer(m) ((m & 240) == 32)
#define makeInformer(basedir,rightsearch,leftsearch) (32 | (basedir<<2) | (rightsearch<<1) | leftsearch)
#define informerHasSearchers(m) ((m & 3) == 3)
#define informerHasRightSearcher(m) ((m & 2) == 2)
#define informerHasLeftSearcher(m) ((m & 1) == 1) 
#define informerDir(m) ((m & 12)>>2)
#define informerRightSearchDir(dir) ((dir + 1) % 4)
#define informerLeftSearchDir(dir) (dir)
#define isGatherer(m) ((m & 240) == 48)
#define makeGatherer(basedir,count) (48 | (basedir<<2) | count)
#define gatherDir(m) ((m & 12)>>2)
#define gatherCount(m) (m & 3)
#define isGuard(m) (m < 4)
#define makeGuard(count) (count)
#define isSearcher(m) ((m & 224) == 128)
#define makeSearcher(dir,outside,sent,count) (128 | (dir<<3) | (outside<<2) | (sent<<1) | count)
#define searchDir(m) ((m & 24)>>3)
#define searchOutside(m) ((m & 4) == 4)
#define searchSent(m)  ((m & 2) == 2)
#define searchCount(m) (m & 1)
#define searchNextMove(m) ((searchCount(m) == 0) ? searchDir(m) : left(searchDir(m)))
#define searchAhead(m) ((searchCount(m) == 0) ? left(searchDir(m)) : searchDir(m))
#define makeNewLeftSearcher(infdir,sent) makeSearcher(informerLeftSearchDir(infdir),0,sent,0)
#define makeNewRightSearcher(infdir,sent) makeSearcher(informerRightSearchDir(infdir),0,sent,1)
#define isFoodReturner(m) ((m & 192) == 64)
#define makeFoodReturner(dir,count,food) (64 | (dir<<4) | (count<<3) | food)
#define foodReturnerDir(m) ((m & 48)>>4)
#define foodReturnerCount(m) ((m & 8)>>3)
#define foodReturnerFood(m) (m & 7)
#define foodReturnerNextMove(m) ((foodReturnerCount(m) == 0) ? reverseDir(left(foodReturnerDir(m))) : reverseDir(foodReturnerDir(m)))
#define isBaseBuilder(m) ((m & 224) == 160)
#define makeBaseBuilder(ready,dir,count) (160 | (ready<<4) | (dir<<2) | count)
#define baseBuilderReady(m) ((m & 16)>>4)
#define baseBuilderDir(m) ((m & 12)>>2)
#define baseBuilderCount(m) (m & 3)
#define timeForBase(m) ((m & 255) == 255)
#define left(dir) ((dir + 3) % 4)
#define right(dir) ((dir + 1) % 4)
#define reverseDir(dir) ((dir + 2) % 4)
int getInformer(struct SquareData *felt,u_char *mem){
  int j = 0;
  for (j=1;j<felt[0].NumAnts;j++){
	if (isInformer(mem[j]) || isFoodInformer(mem[j]))
	  return j;
  }
  return 0;
}
int getNumberOfFoodReturners(struct SquareData *felt,u_char *mem){
  int number = 0;
  int j = 0;
  for (j = 1; j<felt[0].NumAnts;j++)
	if (isFoodReturner(mem[j]))
	  ++number;
  return number;
}
int getHighestFoodInformer(struct SquareData *felt,u_char *mem){
  int number = 0;
  int j = 0;
  for (j = 1; j<felt[0].NumAnts;j++)
	if (isFoodInformer(mem[j]))
	  number = j;
  return number;
}
int getBuilderCount(struct SquareData *felt,u_char *mem){
  int j = 0;
  int builders = 0;
  for (j = 0; j<felt[0].NumAnts;j++)
	if (isBaseBuilder(mem[j]))
	  ++builders;
  return builders;
}
int getRandomBuilder(struct SquareData *felt,u_char *mem){
  int j = 0;
  for (j=0;j<felt[0].NumAnts;j++){
	if (isBaseBuilder(mem[j]))
	  return j;
  }
  return 0;
}
int getRandomValue(struct SquareData *felt,u_char *mem){
  int random = 0;
  int j = 0;
  for (j = 0;j<felt[0].NumAnts;j++)
	random += mem[j];
  for (j = 0;j<=4;j++)
	random += felt[j].NumAnts + felt[j].NumFood;
  return random;
}
int getZeroAnts(struct SquareData *felt,u_char *mem){
  int number = 0;
  int j = 0;
  for (j=1;j<felt[0].NumAnts;j++)
	if (mem[j] == 0)
	  ++number;
  return number;
}
int orders(struct SquareData *felt,u_char *mem){
  int i;
  for (i = 1; i <= 4; i++){
	if (felt[i].Team){
	  *mem = makeGuard(0);
	  return i;
	}
  }
  if (felt[0].Base){
	int dir = gatherDir(*mem);
	int count = 0;
	if (*mem == 0)
	  return 0;
	if (timeForBase(*mem)){
	  int zeroants = getZeroAnts(felt,mem);
	  if (zeroants < 4){
		*mem = 0;
	  } else {
		int random = getRandomValue(felt,mem) % 4;
		for (i = 1;i<felt[0].NumAnts;i++)
		  if (mem[i] == 0)
			mem[i] = 160;
		if (random == 3 || random == 1)
		  *mem = makeBaseBuilder(0,random,1);
		else
		  *mem = makeBaseBuilder(0,random,0);
		return go(random);
	  }
	}
	if (dir == 3 || dir == 1)
	  count = 3;
	else
	  count = 0;
	*mem = makeGatherer(dir,count);
	return go(dir);
  }
  if (isGatherer(*mem)){
	int myDir = gatherDir(*mem);
	int myCount = gatherCount(*mem);
	if (felt[0].NumFood){
	  int newCount = (myCount + 3) % 4;
	  int builders = getBuilderCount(felt,mem);
	  if (builders > 0){
		int rndBuilder = getRandomBuilder(felt,mem);
		if (rndBuilder > 0){
		  int ready = baseBuilderReady(mem[rndBuilder]);
		  if (ready == 0){
			if (builders < (NewBaseFood+3) && felt[0].NumFood >= (NewBaseFood+3)  && (baseBuilderCount(mem[rndBuilder]) == 0)){
			  *mem = makeBaseBuilder(0,baseBuilderDir(mem[rndBuilder]),baseBuilderCount(mem[rndBuilder]));
			  return 0;
			} else {
			  *mem = makeGatherer(myDir,((myCount+1)%4));
			  return go(myDir);
			}
		  } else {
			*mem = makeGatherer(myDir,((myCount+1)%4));	  
			return go(myDir);
		  }
		} else 
		  return 0;
	  } else {
		if (felt[reverseDir(myDir)+1].NumAnts < MaxSquareAnts){
		  *mem = makeGatherer(myDir,newCount);	  
		  return goFood(reverseDir(myDir));
		} else {
		  *mem = makeGatherer(myDir,((myCount+1)%4));
		  return go(myDir);
		}
	  }
	} else {
	  int informer = getInformer(felt,mem);
	  if (myCount == 3){
		if (informer){
		  if (isInformer(mem[informer])){
			int informerDir = informerDir(mem[informer]);
			if (informerHasSearchers(mem[informer])){
			  *mem = makeGatherer(myDir,0);
			  return go(myDir);
			} else {
			  if (informerHasRightSearcher(mem[informer])){
				mem[informer] = makeInformer(informerDir,1,1);
				*mem = makeNewLeftSearcher(informerDir,0);
				return go(left(informerDir));
			  } else {
				mem[informer] = makeInformer(informerDir,1,informerHasLeftSearcher(mem[informer]));
				*mem = makeNewRightSearcher(informerDir,0);
				return go(right(informerDir));
			  }
			}
		  } else {
			int numFood = foodInformerNumFood(mem[informer]);
			int foodDir = foodInformerFoodDir(mem[informer]);
			int infDir = foodInformerDir(mem[informer]);
			if (numFood > 0){
			  if (foodDir == 0){
				mem[informer] = makeFoodInformer(foodDir,infDir,numFood-1);
				*mem = makeNewRightSearcher(myDir,1);
				return go(right(myDir));
			  } else {
				mem[informer] = makeFoodInformer(foodDir,infDir,numFood-1);
				*mem = makeNewLeftSearcher(myDir,1);
				return go(left(myDir));
			  }
			} else {
			  int highest = getHighestFoodInformer(felt,mem);
			  if (informer < highest){
				mem[informer] = makeGatherer(myDir,3);
				return 0; 
			  } else {
				mem[informer] = makeInformer(myDir,0,0);
				return 0; 
			  }
			}
		  }
		} else {
		  *mem = makeInformer(myDir,0,0);
		  return 0;
		}
	  } else {
		if (informer > 0){
		  *mem = makeGatherer(myDir,0);
		  return go(myDir);
		} else {
		  int newCount = myCount+1;
		  if (felt[myDir+1].NumAnts < MaxSquareAnts){
			*mem = makeGatherer(myDir,newCount);
			return go(myDir);
		  } else {
			*mem = makeGatherer(myDir,((myCount + 3)%4));
			return go(reverseDir(myDir));
		  }
		}
	  }
	}	
  }
  if (isSearcher(*mem)){
	int myDir = searchDir(*mem);         
	int myCount = searchCount(*mem);     
	int mySent = searchSent(*mem);       
	int myOutside = searchOutside(*mem); 
	int myMove = searchNextMove(*mem);
	int ahead = searchAhead(*mem);
 	if (myOutside == 1){
	  if (felt[0].NumFood){
		int foodCount = 0;
		if (mySent == 0){
		  int foodExcess = felt[0].NumFood - felt[0].NumAnts;
		  if (foodExcess < 1 && foodExcess > 0)
			foodCount = 1;
		  else
			foodCount = foodExcess / 1;
		  if (foodCount > 7)
			foodCount = 7;
		  if (foodCount < 0)
			foodCount = 0;
		}
		*mem = makeFoodReturner(myDir,myCount,foodCount);
	  } else {
		*mem = makeSearcher(myDir,0,mySent,myCount);
	  }
	  return goFood(reverseDir(ahead));
	}
	if (felt[0].NumFood){
	  int foodret = getNumberOfFoodReturners(felt,mem);
	  if (felt[0].NumFood <= foodret){
	  } else {
		int foodCount = 0;
		int foodReturnerMove;
		if (mySent == 0){
		  int foodExcess = felt[0].NumFood - foodret - 1;
		  if (foodExcess < 1 && foodExcess > 0)
			foodCount = 1;
		  else
			foodCount = foodExcess / 1;
		  if (foodCount > 7)
			foodCount = 7;
		  if (foodCount < 0)
			foodCount = 0;
		}
		*mem = makeFoodReturner(myDir,myCount,foodCount);
		foodReturnerMove = foodReturnerNextMove(*mem);
		*mem = makeFoodReturner(myDir,((myCount+1)%2),foodCount);
		return goFood(foodReturnerMove);
	  }
	} 
	if (felt[ahead+1].NumFood){
	*mem = makeSearcher(myDir,1,mySent,myCount);
	  return go(ahead);
	} else {
	  *mem = makeSearcher(myDir,0,mySent,((myCount+1)%2));
	  return go(myMove);
	}
  }
  if (isFoodReturner(*mem)){
	int myDir = foodReturnerDir(*mem);
	int myCount = foodReturnerCount(*mem);
	int myFood = foodReturnerFood(*mem);
	int informer = getInformer(felt,mem);
	int myMove = foodReturnerNextMove(*mem);
	if (informer == 0){
	  *mem = makeFoodReturner(myDir,((myCount+1)%2),myFood);
	  return goFood(myMove);
	} else {
	  int baseDir = 0;
	  if (isInformer(mem[informer]))
		baseDir = informerDir(mem[informer]);
	  else 
		baseDir = foodInformerDir(mem[informer]);
	  if (myFood > 0){
		int foodCount = myFood * 1;
		if (foodCount > 7)
		  foodCount = 7;
		if (isInformer(mem[informer])){
		  mem[informer] = makeFoodInformer(myCount,baseDir,foodCount);
		} else {
		  *mem = makeFoodInformer(myCount,baseDir,foodCount);
		}
	  }
	  *mem = makeGatherer(baseDir,(3-1));
	  return goFood(reverseDir(baseDir));
	}
  }
  if (isFoodInformer(*mem)){
	return 0;
  }
  if (isInformer(*mem)){
	return 0;
  }
  if (isGuard(*mem)){
	if (*mem == 3){
	  for (i=1;i<felt[0].NumAnts;i++){
		*mem = mem[i];
		return 0;
	  }
	  for (i=1;i<=4;i++){
		if (felt[i].NumAnts)
		  return i;
	  }	  
	} else {
	  for (i=0;i<=4;i++){
		if (felt[i].NumAnts)
		  ++*mem;
	  }
	}
	return 0;
  }
  if (isBaseBuilder(*mem)){
	int myDir = baseBuilderDir(*mem);
	int myReady = baseBuilderReady(*mem);
	int myCount = baseBuilderCount(*mem);
	if (myReady == 1){
	  if (myCount == 3){
		int informer = getInformer(felt,mem);
		if (informer > 0){
		  *mem = makeBaseBuilder(1,myDir,0);
		  return goFood(myDir);
		}
		return 16;
	  } else {
		*mem = makeBaseBuilder(1,myDir,(myCount+1));
		return goFood(myDir);
	  }
	} else {
	  if (myCount == 1){
		*mem = makeBaseBuilder(0,myDir,0);
		return go(myDir);
	  }
	  if (felt[0].NumFood >= (NewBaseFood+3)){
		int builders = getBuilderCount(felt,mem);
		if (builders < NewBaseFood+3)
		  return 0;
		*mem = makeBaseBuilder(1,myDir,myCount);
		for (i = 1;i<felt[0].NumAnts;i++)
		  mem[i] = makeBaseBuilder(1,myDir,myCount);
		return 0;
	  } else {
		return 0;
	  }
	}
  }
  return 4;
}
DefineAnt(AntAgonist, "AntAgonist#11AAFF", orders, u_char)
