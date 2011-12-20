#include "Myre.h"
struct FH {u_long r;signed char x,y;unsigned short s;};DefineAnt(FormAnt,
"FormAnt",FormAnt,struct FH);u_long FAR(int t,u_long *r){*r=*r*89423309+89449237
;return((*r<<13)+(*r>>19))%t;}int PornAnt(struct SquareData *f,struct FH *m){int
i,d,tx,ty;if(m->s==21)m->s=1;if(m->s>=2)m->s--;if(!(m->x%64||m->y%64))m->x=m->y=
0;for(i=1;i<f->NumAnts;i++)if((m[i].x>=0?m[i].x:-m[i].x)+(m[i].y>=0?m[i].y:-m[i]
.y)<(m->x>=0?m->x:-m->x)+(m->y>=0?m->y:-m->y)){m->x=m[i].x;m->y=m[i].y;}for(i=1;
i<5;i++)if(f[i].Team){m->s=263;return i;}if(m->s>13)return 0;if(!m->x&&!m->y){m
->s=0;if(!f->Base&&f->NumFood>=NewBaseFood)return 16;for(i=1;i<5;i++)if(f[i].
NumFood&&m->s<2)return i;return FAR(4,&m->r)+1;}tx=((int)m->x)*((int)m->x);ty=((
int)m->y)*((int)m->y);if(tx+ty>8192)m->s=1;if(tx+ty<2457)m->s=0;if(f->NumFood&&m
->s<2){if((m->x>=0?m->x:-m->x)<=2&&!m->y)return 10+(m->x<0?-1:m->x>0?1:0);if((m
->y>=0?m->y:-m->y)<=2&&!m->x)return 11+(m->y<0?-1:m->y>0?1:0);if(f->NumFood>1)
return !m->x?11+(m->y<0?-1:m->y>0?1:0):!m->y?10+(m->x<0?-1:m->x>0?1:0):(m->x>=0?
m->x:-m->x)<(m->y>=0?m->y:-m->y)?10+(m->x<0?-1:m->x>0?1:0):11+(m->y<0?-1:m->y>0?
1:0);else{d=!m->y?f[4].NumFood?4:f[2].NumFood?2:f[2-(m->x<0?-1:m->x>0?1:0)].
NumFood?2-(m->x<0?-1:m->x>0?1:0):0:!m->x?f[3].NumFood?3:f[1].NumFood?1:f[3-(m->y
<0?-1:m->y>0?1:0)].NumFood?3-(m->y<0?-1:m->y>0?1:0):0:f[3-(m->y<0?-1:m->y>0?1:0)
].NumFood?3-(m->y<0?-1:m->y>0?1:0):f[2-(m->x<0?-1:m->x>0?1:0)].NumFood?d=2-(m->x
<0?-1:m->x>0?1:0):0;if(d&&f[d].NumAnts<MaxSquareAnts)return d;}return !m->x?11+(
m->y<0?-1:m->y>0?1:0):!m->y?10+(m->x<0?-1:m->x>0?1:0):(m->x>=0?m->x:-m->x)<(m->y
>=0?m->y:-m->y)?10+(m->x<0?-1:m->x>0?1:0):11+(m->y<0?-1:m->y>0?1:0);}else{d=0;
for(i=1;i<5;i++)if((m->x+(i==1)-(i==3)||m->y+(i==2)-(i==4))&&f[i].NumFood&&m->s<
2&&f[d].NumAnts<MaxSquareAnts)return i;return (m->x>=0?m->x:-m->x)<=3&&(m->y>=0?
m->y:-m->y)<=3&&m->s<2?(m->x>=0?m->x:-m->x)>(m->y>=0?m->y:-m->y)?2-(m->x<0?-1:m
->x>0?1:0):3-(m->y<0?-1:m->y>0?1:0):!m->x?f[2+(m->y<0?-1:m->y>0?1:0)].NumAnts?3-
(m->y<0?-1:m->y>0?1:0):2+(m->y<0?-1:m->y>0?1:0):!m->y?f[3-(m->x<0?-1:m->x>0?1:0)
].NumAnts?2-(m->x<0?-1:m->x>0?1:0):3-(m->x<0?-1:m->x>0?1:0):m->x>0?m->y>0?m->x>m
->y?m->s==1?2+FAR(2,&m->r):2:m->s==1?3:2+FAR(2,&m->r):m->x>-m->y?m->s==1?2:1+FAR
(2,&m->r):m->s==1?1+FAR(2,&m->r):1:m->y>0?-m->x>m->y?m->s==1?4:3+FAR(2,&m->r):m
->s==1?3+FAR(2,&m->r):3:-m->x>-m->y?m->s==1?FAR(2,&m->r)?4:1:4:m->s==1?1:FAR(2,&
m->r)?4:1;}}int FormAnt(struct SquareData *f,struct FH *m){int d,r=PornAnt(f,m);
if(d=r&7){if(f[d].NumAnts<MaxSquareAnts){m->x+=(d==1)-(d==3);m->y+=(d==2)-(d==4)
;}else m->s=12;}return r;}
