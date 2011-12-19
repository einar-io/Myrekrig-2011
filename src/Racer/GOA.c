#include "Myre.h"

struct GOAData 

{
	short nTime;

	char nFood[2];

	char nNumFood;

	char nRingNo;

	char nPos[2];

	unsigned char nCaution;

	char nStatus;

};


int GOAGetDirectionForDeltaPosition(struct GOAData *pData,int nX,int nY);



int GOAMove(int nDir,char *pPos);

#define GOAMMove(nDir,BringFood,CreateBase,pPos) GOAMove(nDir+BringFood*8+CreateBase*16,pPos)



void GOAShareKnowledge(struct GOAData *pTo,struct GOAData *pFrom);

int GOASelectNewDirection(struct SquareData *pFields, struct GOAData *pData);

struct GOAData* GetEmperor(struct SquareData *pFields, struct GOAData *pData);



enum

{

	DIR_Stand=0,

	DIR_Left,

	DIR_Down,

	DIR_Right,

	DIR_Up

};


#define BASESPACING 64

#define CAUTION 20



int GOATeamFunc(struct SquareData *pFields, struct GOAData *pData)

{
	register int i;

	register struct GOAData *ipData;

	register struct SquareData* ipFields;

	register short nSpeedReg;



	if(pData->nStatus==0)

	{

		pData->nTime&=0x03; // New born (or returned soldier) - clear to rand(4)

		pData->nNumFood=pData->nRingNo=0;

		pData->nStatus=-1; // Normal Ant

	}



	pData->nTime++;



	if(pData->nStatus==1) // Am I emperor

	{

		if(*(short*)pData->nPos!=0) pData->nStatus=0; // Moved due to earlier attack. No longer emperor.

		if((pData->nTime++>1500)||(pFields->Base==0)) pData->nTime&=3;

	}

	else

	{

		if(pData->nNumFood!=0)

		{

			if(pFields->NumAnts>1) GOAShareKnowledge(pData+1,pData); // Only try to tell one ant.

			if(pFields->NumAnts>2) GOAShareKnowledge(pData+2,pData); // OK - two then

	//		ipData=pData+1;

	//		for(i=pFields->NumAnts-1;i--;) GOAShareKnowledge(ipData++,pData);

		}

	}





	nSpeedReg=pData->nPos[1];

	if(*pData->nPos>=(5*BASESPACING/8))

	{

		pData->nPos[0]-=BASESPACING; pData->nStatus=0;

	}

	else if(*pData->nPos<=(-5*BASESPACING/8))

	{

		pData->nPos[0]+=BASESPACING; pData->nStatus=0;

	}

	else if(nSpeedReg>=(5*BASESPACING/8))

	{

		pData->nPos[1]-=BASESPACING; pData->nStatus=0;

	}

	else if(nSpeedReg<=(-5*BASESPACING/8))

	{

		pData->nPos[1]+=BASESPACING; pData->nStatus=0;

	}

	if(pData->nStatus==0)

	{

		*(short*)pData->nFood=0;

		pData->nNumFood=1; // Go to base, unless something better happens.

	}



	ipFields=pFields;

	for(i=1;i<=4;i++)

	{

		if((++ipFields)->Team)

		{

			pData->nCaution=CAUTION;

			return GOAMMove(i,0,0,pData->nPos); // Kill enemy

		}

	}



	if(pData->nStatus==1) return 0; // Am I emperor



	// No enemies

	if(pData->nCaution) // Proceed with caution. Enemies nearby.

	{

		if(pData->nCaution==1) pData->nCaution++; // Forever cautious (move slower)

		else

		{

			pData->nCaution--;

			return 0;

		}

	}



	if(*(short*)pData->nPos==0) // Base check

	{

		pData->nCaution=0;

		



		if(((ipData=GetEmperor(pFields,pData))==0)||(pFields->NumAnts<3)) // Too few emperors

		{

			pData->nStatus=1; // I am it.

			pData->nRingNo=0;

			pData->nTime&=0x03;

			return 0;

		}

		pData->nRingNo=++(ipData->nRingNo);

		if(ipData->nRingNo>=(ipData->nTime/128+BASESPACING/6+8)) ipData->nRingNo=ipData->nTime/128;



		if(pFields->Base==0) // On base field without base

		{

			if(pFields->NumFood>=NewBaseFood) return GOAMMove(0,0,1,pData->nPos);

		}

		else // On base field with base

		{

			if(pFields->NumAnts<5) // Too few emperors

			{

				pData->nStatus=1; // I am it.

				pData->nRingNo=0;

				pData->nTime&=0x03;

				return 0;

			}

				

			if(pData->nTime>100) // Old enough to go to war

			{

				if(ipData->nTime>1000) // KILL time

				{

					pData->nStatus=(pData->nTime%4)+2;

//					pData->nTime&=0x03;

					pData->nNumFood=0;

					return GOAMMove(pData->nStatus-1,0,0,pData->nPos);

				}

			}

		}

	}

	// No base problems

	else

	{

		if(nSpeedReg=pFields->NumFood) // Got food. Go home (UGH!)

		{

			nSpeedReg-=pFields->NumAnts;

			if(nSpeedReg>0)

			{

				*(short*)pData->nFood=*(short*)pData->nPos;

				pData->nNumFood =(char)nSpeedReg; // Update food amount

			}

			return GOAMMove(GOAGetDirectionForDeltaPosition(pData,0,0),1,0,pData->nPos); // Go to base

		}

	}



	// No food at this field

	ipFields=pFields+DIR_Left;

	nSpeedReg=*(short*)pData->nPos;

	if((ipFields->NumFood>(ipFields++)->NumAnts)&&(nSpeedReg!=0x0001)) return GOAMMove(DIR_Left,0,0,pData->nPos);

	if((ipFields->NumFood>(ipFields++)->NumAnts)&&(nSpeedReg!=0x0100)) return GOAMMove(DIR_Down,0,0,pData->nPos);

	if((ipFields->NumFood>(ipFields++)->NumAnts)&&(nSpeedReg!=0x00ff)) return GOAMMove(DIR_Right,0,0,pData->nPos);

	if((ipFields->NumFood>ipFields->NumAnts)&&(nSpeedReg!=(short)0xff00)) return GOAMMove(DIR_Up,0,0,pData->nPos);



	// Check for known food position

	if(pData->nNumFood>0)

	{

		if(*(short*)pData->nFood==nSpeedReg) // Have we arrived (where there's no food)

		{

			pData->nNumFood=0;

		}

		else

		{

			return GOAMMove(GOAGetDirectionForDeltaPosition(pData,pData->nFood[0],pData->nFood[1]),0,0,pData->nPos);  // Go to food

		}

	}



	if(pData->nStatus>=2) // War Ant?

	{

		if(pData->nTime++>3000)

		{

			pData->nStatus=0;

			pData->nTime=*(short*)pData; // To have a "random" value for init.

		}

		if(pFields->NumAnts<4) return 0;

		return GOAMMove(pData->nStatus-1,0,0,pData->nPos);

	}



	// Search for food.

	return GOAMMove(GOASelectNewDirection(pFields,pData),0,0,pData->nPos);

}


int GOAMove(int nDir,char* pPos)

{

	switch(nDir&0x7)

	{

		case DIR_Left:

            (*pPos)--;

			break;

		case DIR_Right:

			(*pPos)++;			

			break;

		case DIR_Up:

			(*(++pPos))++;

			break;

		case DIR_Down:

			(*(++pPos))--;

			break;

	}



	return nDir;

}



struct GOAData* GetEmperor(struct SquareData *pFields, struct GOAData *pData)

{

	int i;



	pData++;

	for(i=pFields->NumAnts;i>1;i--)

	{

		if((pData++)->nStatus==1) return (pData-1);

	}



	return 0;

}





int GOAGetDirectionForDeltaPosition(struct GOAData *pData,int nX,int nY)

{

	if(pData->nPos[0]>nX) return DIR_Left;

	if(pData->nPos[0]<nX) return DIR_Right;

	if(pData->nPos[1]>nY) return DIR_Down;

	return DIR_Up; // pData->nY<nY

}



void GOAShareKnowledge(struct GOAData *pTo,struct GOAData *pFrom)

{

	if((pFrom->nNumFood>1)&&(pTo->nNumFood<=1)) // To hasn't knowledge, From has

	{

		*(short*)pTo->nFood=*(short*)pFrom->nFood;

		pTo->nNumFood=pFrom->nNumFood;

	}

	if((pFrom->nCaution!=0)&&(pTo->nCaution==0)) // To hasn't knowledge

	{

		pTo->nCaution=pFrom->nCaution;

	}

}





int GOASelectNewDirection(struct SquareData *pFields, struct GOAData *pData)

{

	short nLeft=pData->nRingNo*3;

	char Dirs[28]={0,1,1,1,1,0,1,-1,0,-1,-1,-1,-1,0,-1,1,0,1,1,1,1,0,1,-1,0,-1,-1,-1};



	int i;

	int nDirStart;

	pData->nNumFood=1;



	for(i=0;i<28;i++) Dirs[i]*=nLeft;



	nDirStart=(pData->nTime&0x03);



	*(short*)pData->nFood=0;



	if(*(short*)pData->nPos==0)

	{

		pData->nFood[1^(nDirStart&0x01)]= (nDirStart&0x02) ? -nLeft : nLeft;

		return GOAGetDirectionForDeltaPosition(pData,pData->nFood[0],pData->nFood[1]);

	}



	for(i=nDirStart*2;i<(nDirStart*2+7);i++)

	{

		if(*(short*)pData->nPos==*(short*)(&Dirs[i*2]))

		{

			*(short*)pData->nFood=*(short*)(&Dirs[(i+1)*2]);

			return GOAGetDirectionForDeltaPosition(pData,pData->nFood[0],pData->nFood[1]);

		}

	}



	*(short*)pData->nFood=0; // Nothing found. Go to base.



	return GOAGetDirectionForDeltaPosition(pData,0,0);

}




DefineAnt(GOA,"GOA",GOATeamFunc,struct GOAData)
