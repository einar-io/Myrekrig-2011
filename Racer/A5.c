/* A5, version (0.1.2) Copyright: Andreas Beier aka AB (beier@mip.sdu.dk)
Opkaldt efter størrelsen på det stykke papir source koden kan stå på*/
#include "Myre.h"
#define x mem->xpos
#define y mem->ypos
#define xm mem->xmad
#define ym mem->ymad
#define foo(x,y,a,b) ((abs(x-a))>(abs(y-b)) ? ((x)>(a) ? (3):(1)) : ((y)>(b) ? (2):(4)))
struct A5Brain {u_long rnd;	short int xpos,ypos,xmad,ymad;char antal,lastbar,type;};
int A5(struct SquareData *fields, struct A5Brain *mem){
	int i,bar=0;
	if(mem->type!=1){mem->rnd=mem->rnd*25+123;
		if(mem->rnd%5==1){mem->rnd=mem->rnd*25+123;bar=mem->rnd%5+8;} else bar=mem->lastbar;
		if(((abs(x)+(1==bar || 3==bar))%2) && ((abs(y)+(4==bar || 2==bar))%2))
			bar=mem->lastbar;
		for(i=1;i<=4;i++){if(fields[i].NumFood>(fields[i].NumAnts+5)){
			bar=i;mem->antal=fields[i].NumFood;xm=x;ym=y;}}
		if(x == xm && y == ym && !fields[0].NumFood){ xm=0;ym=0;mem->antal=0;}
		if(!fields[0].NumFood && (xm || ym)){bar=foo(x,y,xm,ym);}
		if(fields[0].NumFood) bar=foo(x,y,0,0)+8;
		for(i=1;i<fields[0].NumAnts;i++){
			if(!xm && !ym){	if(mem[i].antal){xm=mem[i].xmad;ym=mem[i].ymad;mem[i].antal--;}}
			else{if(!mem[i].xmad && !mem[i].ymad && mem->antal){
				mem[i].xmad=xm;mem[i].ymad=ym;	mem->antal--;}}}
		if(fields[0].NumAnts>15){mem->antal=0;}
	}
	for(i=1;i<=4;i++){	if(fields[i].Team > 0){bar=i;mem->type=1;}}
	if(bar%8==1)x++; else if(bar%8==2)y--; else if(bar%8==3)x--; else if(bar%8==4)y++;
	mem->lastbar=bar;return bar;
}
DefineAnt(A5, "A5", A5, struct A5Brain)
