#include "Myre.h"


enum {Right = 1, Left = 3, Up = 4, Down = 2, UpWithFood = 12, RightWithFood = 9, LeftWithFood = 11, DownWithFood = 10};

int goRandomDirection(u_long random, u_long count);


struct AntBrain {
   u_long random;
   int bx, by;
   u_long count;
   long currentx, currenty;
   short gotobaseflag;
}mem;



int AntFunc(struct SquareData *felter, struct AntBrain *mem) {

    
    int selectedDirection = 0;
   
    mem->count ++;
    
    if (mem->count > 99)
    {
        mem->count = 1;
    }
    //hvis myren står på en base, sæt base koordinater
    if(felter->Base == 1)
    {
        mem->bx = 0;
        mem->by = 0;
        mem->currentx = 0;
        mem->currenty = 0;
        mem->gotobaseflag = 0;
    }
    //Hvis myren står alene på en base, uden mad, gå et random sted hen efter mad.
    if((felter->Base == 1)&& (felter->NumFood == 0))
    {
        mem->bx = 0;
        mem->by = 0;
        mem->gotobaseflag = 0;
        selectedDirection = goRandomDirection(mem->random, mem->count);
        switch(selectedDirection)
        {
                case(Right):
                {
                    mem->currentx += 1;
                }
                case(Left):
                {
                    mem->currentx -= 1;
                }
                case(Up):
                {
                    mem->currenty += 1;
                }
                case(Down):
                {
                    mem->currenty -= 1;
                }
        }
    }
        
    //Hvis myren ikke står på en base, uden mad, gå et random sted hen efter mad.
    if((felter->NumFood == 0) && (felter->Base == 0))
    {
        selectedDirection = goRandomDirection(mem->random, mem->count);
        switch(selectedDirection)
        {
                case(Right):
                {
                    mem->currentx += 1;
                }
                case(Left):
                {
                    mem->currentx -= 1;
                }
                case(Up):
                {
                    mem->currenty += 1;
                }
                case(Down):
                {
                    mem->currenty -= 1;
                }
        }
    }
    
    if((felter->NumFood != 0) && (felter->NumAnts == 1) && (felter->Base == 0))
    {
        mem->gotobaseflag = 1;
        
        if(mem->currentx > mem->bx)
        {
            selectedDirection = Left +8;
            mem->currentx -= 1;
            
        }
        if(mem->currentx < mem->bx)
        {
            selectedDirection = Right +8;
            mem->currentx += 1;
            
        }
        if((mem->currentx == mem->bx)&& (mem->currenty < mem->by))
        {
            selectedDirection = Up + 8;
            mem->currenty += 1;
            
        }
        if((mem->currentx == mem->bx) && (mem->currenty > mem->by))
        {
            selectedDirection = Down + 8;
            mem->currenty -= 1;
            
        }
        
    }
    
    if(mem->gotobaseflag == 1)
    {
        if(mem->currentx > mem->bx)
        {
            selectedDirection = Left;
            mem->currentx -= 1;
            
        }
        if(mem->currentx < mem->bx)
        {
            selectedDirection = Right;
            mem->currentx += 1;
            
        }
        if((mem->currentx == mem->bx)&& (mem->currenty < mem->by))
        {
            selectedDirection = Up;
            mem->currenty += 1;
            
        }
        if((mem->currentx == mem->bx) && (mem->currenty > mem->by))
        {
            selectedDirection = Down;
            mem->currenty -= 1;
            
        }
    }
    
    
   
    
    return selectedDirection;
}

int goRandomDirection(u_long random, u_long count)
{
    u_long randomseed = random;
    u_long counter = count;
  
    for(counter = counter; counter > 1; counter--)
    {
        randomseed = randomseed*random;
    }
    
    
    return (randomseed % 5)+1;
    
}

DefineAnt(ElephAnt, "ElephAnt", AntFunc, struct AntBrain);