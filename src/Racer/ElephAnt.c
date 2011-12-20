#include "Myre.h"


enum {Right = 1, Left = 3, Up = 4, Down = 2, UpWithFood = 12, RightWithFood = 9, LeftWithFood = 11, DownWithFood = 10};

int goRandomDirection(u_long random, u_long count);

struct AntBrain {
   u_long random;
   short x,y;
   short foodx[10],foody[10];
   u_long count;
   short currentx, currenty;
};

struct AntBrain mem;


int AntFunc(struct SquareData *felter, struct AntBrain *mem) {

    
    int selectedDirection = 0;
    mem->count ++;
    
    
    //Hvis myren står alene på en base, gå et random sted hen efter mad.
    if((felter->Base == 1))
    {
        (mem->x) = 0;
        (mem->y) = 0;
        selectedDirection = goRandomDirection(mem->random, mem->count);
    }
    
    //Hvis myren ikke står på en base, gå et random sted hed efter mad.
    if(felter->NumFood == 0)
    {
        selectedDirection = goRandomDirection(mem->random, mem->count);
    }
    
    //Hvis der er mad på pågældende felt, tag det med til basen.
    if(felter->NumFood != 0)
    {
        if(mem->x < mem->currentx)
        {
            selectedDirection = LeftWithFood;
            
        }
        
        if(mem->x > mem->currentx)
        {
            selectedDirection = RightWithFood;
            
        }
        if(mem->y < mem->currenty)
        {
            selectedDirection = DownWithFood;
            
        }
        if(mem->y > mem->currenty)
        {
            selectedDirection = UpWithFood;
         
        }
        
    }
    
    switch(selectedDirection)
    {
        case(Right):
        {
           mem->currentx += 1;
        }
        case(Down):
        {
           mem->currenty -= 1;
        }
        case(Left):
        {
           mem->currentx -= 1;
        }
        case(Up):
        {
           mem->currenty += 1;
        }
        case(RightWithFood):
        {
           mem->currentx += 1;
        }
        case(DownWithFood):
        {
           mem->currenty -= 1;
        }
        case(LeftWithFood):
        {
           mem->currentx -= 1;
        }
        case(UpWithFood):
        {
           mem->currenty += 1;
        }
        default:
            break;
    }
    
    return selectedDirection;
}

int goRandomDirection(u_long random, u_long count)
{
    u_long randomseed;
  

    randomseed = ((((random)^count))%5)+1;
    
    return randomseed;
    
}

DefineAnt(ElephAnt, "ElephAnt", AntFunc, struct AntBrain);