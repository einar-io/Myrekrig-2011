#include "Myre.h"

#define basebuilder 128+64+32+16
#define informant 128+64+32
#define gatherer 128+64
#define searcher 128

#define isSearcher(m) ((m & (searcher)) == 128)
#define isGatherer(m) ((m & (gatherer)) == 64)
#define isInformant(m) ((m & (informant)) == 32)
#define isBaseBuilder(m) ((m & (basebuilder)) == 16)
#define isGuard(m) (m < 8)

#define gatherDir(m) ((m & (32+16))>>4)
#define gatherCount(m) (m & (1+2+4+8))
#define maxGatherCount 15

#define informantBaseDir(m) ((m & (8+16))>>3)
#define informantCount(m) (m & (1))
#define informantHasSearcher(m) ((m & 4) == 4)
#define informantAlreadyExplored(m) ((m & 2) == 2)
#define informantCountLimit 1
#define MyBaseFoodLimit 35

#define searchCount(m) (m & (1+2+4+8))
#define searchDir(m) ((m & (64+32))>>5)
#define searchIndex(m) ((m & 16)>>4)

#define north 1
#define south 1
#define east 7
#define west 7

#define infnorth 3
#define infsouth 3

#define goHomeLimit 10

int getInformant(struct SquareData *felt,u_char *mem){
  int j = 0;
  int inf = 0;
  for (j=1;j<felt[0].NumAnts;j++){
	if (isInformant(mem[j]) || isBaseBuilder(mem[j]))
	  inf = j;
  }
  return inf;
}

int getRandom(struct SquareData *felt,u_char *mem,int mod){
  int i,dir = 0;
  for (i = 0; i <= 4;i++)
	dir += felt[i].NumAnts + felt[i].NumFood;
  for (i=0;i<felt[0].NumAnts;i++)
	dir += mem[i] + *mem;
  dir = dir % mod;
  return dir;
}


int getLeft(int dir){
  return (dir+3)%4;
}

int getRight(int dir){
  return (dir+1)%4;
}

const int back[16] = {0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1, 0, 0};

int whatToDo(struct SquareData *felt,u_char *mem){
  int i;
  for (i = 1; i <=4; i++){
	if (felt[i].Team){
	  *mem = 0;
	  return i;
	}
  }
  if (felt[0].Base){
	int tmp;
	if (isGatherer(*mem) && gatherCount(*mem) == 0){
	  *mem |= 1;
	  return gatherDir(*mem)+1;  
	}
	tmp = *mem % 16;
	if (tmp < north){
	  *mem = (3<<4);
	  *mem = *mem | (64+1);
	  return gatherDir(*mem)+1;
	}
	if (tmp < (north+south)){
	  *mem = (1<<4);
	  *mem = *mem | (64+1);
	  return gatherDir(*mem)+1;
	}
	if (tmp < (north+south+east)){
	  *mem = (0<<4);
	  *mem = *mem | (64+1);
	  return gatherDir(*mem)+1;
	}
	if (tmp < (north+south+east+west)){
	  *mem = (2<<4);
	  *mem = *mem | (64+1);
	  return gatherDir(*mem)+1;
	}
  }

  if (isInformant(*mem)){
	int ants = 0;
	for (i = 0; i<=4;i++)
	  ants += felt[i].NumAnts;
	if (felt[0].NumFood && ((*mem & 4) == 0) && ants <= 1){
	  *mem = (((informantBaseDir(*mem)+2)%4)<<4) | (64+8+4+2);
	  return ((gatherDir(*mem)+2)%4)+1+8;
	}
	
	return 0;
  }
  if (isBaseBuilder(*mem)){
	return 16;
  }
  if (isGuard(*mem)){
	if (*mem == 4 && felt[0].NumAnts > 1)
	  *mem = mem[felt[0].NumAnts-1];
	else {
	  if (felt[0].NumAnts > 1)
		*mem = *mem + 1;
	  else {
		for (i = 1;i<=4;i++)
		  if (felt[i].NumAnts){
			if (*mem == 4)
			  return i;
			else 
			  *mem = *mem +1;
		  }
	  }
	}
  return 0;
  }
  if (isGatherer(*mem)){
	int infid = getInformant(felt,mem);
	if (felt[0].NumFood && gatherCount(*mem) > 0){
	  if (felt[((gatherDir(*mem)+2)%4)+1].NumAnts < MaxSquareAnts){
		--*mem;
		return ((gatherDir(*mem)+2)%4)+1+8; 
	  } else {
		return 0;
	  }
	}
	if (gatherCount(*mem) == maxGatherCount){
	  if (infid > 0){
		if (informantHasSearcher(mem[infid]) || informantAlreadyExplored(mem[infid])){
		  int basedir = informantBaseDir(mem[infid]);
		  if (basedir == 1 || basedir == 3) {
			*mem = (((basedir+2)%4)<<4) | (64+1);
			return gatherDir(*mem)+1;
		  } else {
			int random = (getRandom(felt,mem,16)+mem[infid])%16;
			if (random < infnorth){
			  *mem = (3<<4) | (64+1);
			  return gatherDir(*mem)+1;
			}
			if (random < (infnorth+infsouth)){
			  *mem = (1<<4) | (64+1);
			  return gatherDir(*mem)+1;
			}
			*mem = (((basedir+2)%4)<<4) | (64+1);
			return gatherDir(*mem)+1;
		  }
		} else {
		  int count = 5;
		  mem[infid] = mem[infid] | 4 | 1;
		  *mem = (0<<5) | 128 | count;
		  return searchDir(*mem)+1;
		}
	  } else {
		int dir = (gatherDir(*mem)+2)%4;
		*mem = ((dir<<3) | 32);
		return 0;
	  }

	}
	else {
	  if (gatherCount(*mem) == 0 && felt[0].NumFood > goHomeLimit && infid){
		int dir = (informantBaseDir(mem[infid])+2)%4;
		*mem = (dir<<4) | (64 + 14);
		return ((gatherDir(*mem)+2)%4)+1+8;		
	  }
	  if (gatherCount(*mem) > 0 && felt[getLeft(gatherDir(*mem))+1].NumFood){
		int food = getLeft(gatherDir(*mem));
		*mem = (gatherDir(*mem)<<5) | (128+16+1);
		return food+1;
	  }
	  
	  if (gatherCount(*mem) > 0 && felt[getRight(gatherDir(*mem))+1].NumFood){
		int food = getRight(gatherDir(*mem));
		*mem = (((gatherDir(*mem)+2)%4)<<5) | (128+16+1);
		return food+1;
	  }
	  if (felt[gatherDir(*mem)+1].NumAnts < MaxSquareAnts){ 
		++*mem;
		return gatherDir(*mem)+1;
	  } else {
		return 0;
	  }
	}
  }
  if (isSearcher(*mem)){
	int infid = getInformant(felt,mem);
	int myCount, myDir,myIndex;
	myCount = searchCount(*mem);
	myDir = searchDir(*mem);
	myIndex = searchIndex(*mem);
	if (infid > 0){
	  if (myCount == 7){
		*mem = (*mem & (128+64+32)) | 15;
		return myDir+1;
	  }
	  if (isBaseBuilder(mem[infid]) && felt[0].NumFood >= NewBaseFood)
		return 0;
	  if (felt[0].NumFood){
		int dir = (informantBaseDir(mem[infid])+2)%4;
		*mem = (dir<<4) | (64 + 14);
		mem[infid] &= (128+64+32+16+8+2);
		return ((gatherDir(*mem)+2)%4)+1+8;
	  } else {
		int dir,count = 0;
		if (myDir == 3 && informantCount(mem[infid]) == 0){
		  int basedir = informantBaseDir(mem[infid]);
		  mem[infid] = (mem[infid] & (128+64+32+16+8)) | 2;
		  if (basedir == 1 || basedir == 3) {
			*mem = (((basedir+2)%4)<<4) | (64+1);
			return gatherDir(*mem)+1;
		  } else {
			int random = (getRandom(felt,mem,16)+mem[infid])%16;
			if (random < infnorth){
			  *mem = (3<<4) | (64+1);
			  return gatherDir(*mem)+1;
			}
			if (random < (infnorth+infsouth)){
			  *mem = (1<<4) | (64+1);
			  return gatherDir(*mem)+1;
			}
			*mem = (((basedir+2)%4)<<4) | (64+1);
			return gatherDir(*mem)+1;
		  }
		}
		if (informantCount(mem[infid]) == 0){
		  mem[infid] = (mem[infid] & (128+64+32+16+8+4)) | 1;
		  dir = (myDir+1)%4;
		  count = 5;
		} else {
		  mem[infid] &= (128+64+32+16+8+4);
		  dir = myDir;
		  count = 2;
		}
		*mem = (dir<<5) | count | 128;
		return dir+1;
	  }
	} else {
	  if (myIndex == 0){
		if (myCount == 8 || myCount == 7){
		  if (felt[0].NumFood){
			*mem = (*mem & (128+64+32)) | 7;
		  } else {
			*mem = (*mem & (128+64+32)) | 8;
		  }		   
		  return ((myDir+2)%4)+1+8;
		}
		if (myCount > 0){
		  if (felt[0].NumFood){
			*mem = (*mem & (128+64+32)) | 7;
			return ((myDir+2)%4)+1+8;
		  }
		  --*mem;
		  return myDir+1;
		} else {
		  *mem = *mem | (16+1);
		  return getLeft(myDir)+1;
		}
	  } else {
		if (felt[0].NumFood){
		  if (back[myCount] == 1){
			*mem &= (128+64+32);
			return getRight(myDir)+1+8;
		  }
		  *mem = (*mem & (128+64+32+16)) | (back[myCount]-1);
		  return getRight(myDir)+1+8;
		}
		if (felt[((myDir+2)%4)+1].NumFood){
		  *mem = (*mem & (128+64+32+16)) | back[myCount];
		  return ((myDir+2)%4)+1;
		}
		if (felt[myDir+1].NumFood){
		  *mem = (*mem & (128+64+32+16)) | back[myCount];
		  return myDir+1;
		}
		if (myCount < 7){
		  ++*mem;
		  return getLeft(myDir)+1;
		} else {
		  if (myCount == 13){
			*mem = (*mem & (128+64+32)) | 8;
			return getRight(myDir)+1;
		  }
		  ++*mem;
		  return getRight(myDir)+1; 
		}
	  }
	}
  }
  return 0;
}
DefineAnt(InformAnts, "InformAnts#AEFF00", whatToDo, u_char)
