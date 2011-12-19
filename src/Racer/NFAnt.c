#include "Myre.h"
#define BD 3*BM
#define BM 27
#define MA MaxSquareAnts
#define c struct SquareData *f,struct NFH *m
#define NA(a) f[a].NumAnts
#define NF(a) f[a].NumFood
#define a(a) ((a)>=0?(a):-(a))
#define l(a,b,c) for(i=a;i<b;i++){c}
#define r(a) return a;
#define s(a) ((a)<0?-1 :(a)>0?1:0)
#define v(x,y) (a(x)+a(y))
#define x m->mx
#define xd(d) (((d)==1)-((d)==3))
#define y m->my
#define yd(d) (((d)==2)-((d)==4))

struct NFH{signed char mx,my;};

int NT(c){
	int i,d;
	if(f->Base)x=y=0;if(y==127)y-=BD;else if(y==-128)y+=BD;if(x==127)x-=BD;else if(x==-128)x+=BD;
	if((v(x,y)==BD)&&!(x&&y)){x=y=0;}
	if(x&&y){
		l(1,NA(0),if(v(m[i].mx,m[i].my)<v(x,y)){x=m[i].mx;y=m[i].my;});
		l(1,NA(0),m[i].mx=x;m[i].my=y;);
	}
	l(1,5,if(f[i].Team)r(i));
	if(!(x||y)){
		if(NF(0)>=NewBaseFood&&!f->Base)r(16);
		l(1,5,if(NF(i))r(i))d=1;
		l(2,5,if(NA(i)<NA(d))d=i;)r(d);
	}if(NF(0)){
		if(NF(0)>1)r(!x?11+s(y):!y?10+s(x):a(x)>a(y)?10+s(x):11+s(y))else{d=0;
		if(!y){if(NF(4))d=4;
		if(NF(2))d=2;
		if(NF(2-s(x)))d=(2-s(x));
		}else if(!x){if(NF(3))d=3;
		if(NF(1))d=1;
		if(NF(3-s(y)))d=(3-s(y));
		} else {if(NF(3-s(y)))d=(3-s(y));
		if(NF(2-s(x)))d=(2-s(x));
		}if(d&&(NA(d)<MA))r(d)}r(!x?11+s(y):!y?10+s(x):a(x)>a(y)?10+s(x):11+s(y));
	}else{
		if((((a(x)<3)&&!y)&&(NA(2+s(x))))||(((a(y)<3)&&!x)&&(NA(3+s(y)))))r(0);
		d=0;
		l(1,5,if(((x+xd(i))||(y+yd(i)))&&NF(i)&&(NA(d)<MA))r(i));
		r(!x?(y%3)||(NA(2+s(y)))||(((a(y)%12)-3)&&(NA(3+s(y))||NA(3-s(y))||NA(0)>1))?3-s(y):2+s(y):!y?(x%3)||(NA(3-s(x)))||(((a(x)%12)-3)&&(NA(2+s(x))||NA(2-s(x))||NA(0)>1))?2-s(x):3-s(x):s(x)==1?s(y)==1?((y-3)*3<(x)*2)?x%3?1:2:(((x)+2*(y))%7==4)&&(y-3<(x)*3)?2:3:(x-3)*3<(-y)*2?y%3?4:1:(((-y)+2*(x))%7==4)&&(x-3<(-y)*3)?1:2:s(y)==1?(-x-3)*3<(y)*2?y%3?2:3:(((y)+2*(-x))%7==4)&&(-x-3<(y)*3)?3:4:(-y-3)*3<(-x)*2?x%3?3:4:(((-x)+2*(-y))%7==4)&&(-y-3<(-x)*3)?4:1);
	}
}

int NFAnt(c){
	int d,t;
	t=NT(f,m);
	if((d=t&7)&&NA(d)<MA){
		x+=xd(d);
		y+=yd(d);
	}
	r(t);
}

DefineAnt(NFAnt,"NFAnt",NFAnt,struct NFH);
