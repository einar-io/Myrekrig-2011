#include "Myre.h"
// typer: 0 imod mål, 1 søgemønster, 2 cirkelekspansion

#define abv(a) ((a) >= 0 ? (a) : -(a))
#define  xp  m->xpos
#define  yp  m->ypos
#define  xt  m->xtar
#define  yt  m->ytar
#define S 200
#define T 40

struct hjerne{
	u_long	rdm;
	short  xpos,ypos,xtar,ytar;
	short  turn,foodx[6],foody[6];
	u_char foodn[6],c,e,f,g,h,type;
};

int myresyre(struct SquareData *f,struct hjerne *m){
	
int a, b, d=0;
short madx=0, bufx=0, mady=0, bufy=0;
u_char madn=0, bufn=0;


m->turn ++;

if(m->turn == 1)m->type = 11;




if(f->Base == 1)
{	
	m->type = 11;
	
	if(m->foodn[0]>0)
	{
		yt = m->foody[0];
		xt = m->foodx[0];
		m->type = 2;
	}
	else if (f->NumAnts>1)
	for (b=1;b<=f->NumAnts-1;b++)
	{
		if (m->foodn[0]<m[b].foodn[0])
		{
			for(a=0;a<=5;a++)
			{
				m->foodx[a]=m[b].foodx[a];
				m->foody[a]=m[b].foody[a];
				m->foodn[a]=m[b].foodn[a];
				
				if(m->foodn[0]>0)
				{
					yt = m->foody[0];
					xt = m->foodx[0];
					m->type = 2;
				}
			}
		}
	}
}


// START Fjender-------------------------------------------------

for (d=1;d<=4;d++)
{
	if(f[d].Team >=1)
	{
		m->type = 6;
		return d;
	}
}
if(m->type == 6)return 0;
// START Fjender-------------------------------------------------*/

// START Sprede sig----------------------------------------------------


if (m->type == 11)
{
yt = yp + (m->rdm %S) -(S/2);
xt = xp + (m->rdm %(S + 3)) - (S/2)+1;
m->type = 1;
}

if ((m->type == 12)&&(yp!=0)&&(xp!=0))
{
yt = yp + ((yp/abv(yp))*(m->rdm %T + 20));
xt = xp + ((xp/abv(xp))*(m->rdm %(T+3) + 20));
m->type = 1;
}


// SLUT Sprede sig---------------------------------------------------*/
	

// START Check for mad-------------------------------------

	//START venner

if((f->NumAnts == 2)&&((m->type == 10)||(m[1].type == 10)))
{	
	if ((m->type == 10)&&(m[1].foodn[0]>0))
	{
		for(a=0;a<=5;a++)
		{
			m->foodx[a]=m[1].foodx[a];
			m->foody[a]=m[1].foody[a];
			m->foodn[a]=m[1].foodn[a];
			m->type = 2;
		}
	}
	else if ((m[1].type == 10)&&(m->foodn[0]>0))
	{
		for(a=0;a<=5;a++)
		{
			m[a].foodx[a]=m->foodx[a];
			m[a].foody[a]=m->foody[a];
			m[a].foodn[a]=m->foodn[a];
			m[1].type = 2;
		}
	}
if(m->type == 10)m->type = 0;
if(m[1].type == 10)m->type = 0;
//m->c--;
}



	//SLUT venner

	//START felter
for (a=1;a<=4;a++)
{
	if(f[a].NumFood>f[a].NumAnts)
	{
		madn = f[a].NumFood-f[a].NumAnts;
		madx = xp;
		mady = yp;
		switch (a)
		{
			case 1: madx++;break;
			case 2: mady--;break;
			case 3: madx--;break;
			case 4: mady++;break;
		}
	}
	
}
	//SLUT felter*/
// START Check for mad-------------------------------------*/



// START Føde rækkefølge--------------------------------------------
b=0;
for (a=0;a<=5;a++)
{
	if ((m->foodx[a] == madx) && (m->foody[a] == mady))
	{
		m->foodn[a] = madn;
		b=1;
	}
}
if (b == 0)
{
	for(a=0;a<=5;a++)
	{
		if(madn > m->foodn[a])
		{
			bufn = m->foodn[a];
			bufx = m->foodx[a];
			bufy = m->foody[a];
			m->foodn[a] = madn;
			m->foodx[a] = madx;
			m->foody[a] = mady;
			madn = bufn;
			madx = bufx;
			mady = bufy;
		}
	}
}
if (b == 1)
{
	for (a=0;a<=4;a++)
	{
		if (m->foodn[a]<m->foodn[a+1])
		{
			bufx = m->foodx[a+1];
			bufy = m->foody[a+1];
			bufn = m->foodn[a+1];
			m->foodn[a+1] = m->foodn[a];
			m->foodx[a+1] = m->foodx[a];
			m->foody[a+1] = m->foody[a];
			m->foodn[a] = bufn;
			m->foodx[a] = bufx;
			m->foody[a] = bufy;
		}
	}
}
// SLUT Føde rækkefølge--------------------------------------------*/
/*

0 : Rundt
1 : På vej imod rundt
2 : På vej imod F[0]
3 : På vej imod base med mad
6 : forsvarssoldat
10 : Ligeglad
11 : Tjekker ny position for rundt fra base
12 : Tjekker ny position for rundt fra f[0]
*/


// START Hvad gør jeg nu?---------------------------
{short xd = xp - xt;
short yd = yp - yt;
	//START Er der nogen??
if((m->type == 0)&&(m->c=0))
{
	for(a=1;a<=4;a++)
	{
		if (f[a].NumAnts == 1)
		{
//			m->c = m->type;
			m->type = 12;
//			m->c = 1;
			d = a;
		}
	}
}
	//SLUT Er der nogen??*/

if((xd==0) && (yd==0) && (m->type == 1)) m->type = 0;

if((xd==0) && (yd==0) && (m->type == 2))
{
	if (f->NumFood < f->NumAnts)
	{
		m->type = m->rdm %2 +11;
		m->foodn[0]=0;
		
		if(m->foodn[0] > 0)
		{
			m->type = 2;
			xt = m->foodx[0];
			yt = m->foody[0];
		}
	}
	else
	{
		m->type = 3;
		xt = 0;
		yt = 0;
	}
}


if((f->Base == 1)&&(m->type == 3))
{
	if(m->foodn[0] > 0)
	{
		m->type = 2;
		xt = m->foodx[0];
		yt = m->foody[0];
	}
	else m->type = 0;
}

if(((m->type == 0) || (m->type == 1)) && (m->foodn[0]>0))
{
	m->type = 2;
	xt = m->foodx[0];
	yt = m->foody[0];
}
	
xd = xp - xt;
yd = yp - yt;


// SLUT Hvad gør jeg nu?---------------------------*/

	
// Skal jeg gå efter mad eller hvad?------------------------------

// SLUT Skal jeg gå efter mad eller hvad?------------------------------*/


// START madsøgning--------------------------------------------
if(m->type == 0)
{
	if (m->e < m->f)
	{
		m->e ++;
		d = ((m->g %4) + 1);
	}
	else if (m->h == 1)
	{
		m->f = m->f + 2;
		m->e = 0;
		m->h = 0;
		m->g ++;
		d = ((m->g %4) + 1);
	}
	else
	{
		m->f ++;
		m->e = 0;
		m->h = 1;
		m->g ++;
		d = ((m->g %4) + 1);
	}
}
// SLUT madsøgning--------------------------------------------*/	

// START målsøgning ----------------------------------------------------------
if ((m->type == 1)||(m->type == 2)||(m->type == 3))
{
	if((m->type == 3)||(m->type == 4))
	{
		a = 3+8;
		b = 2+8; 
		if(xd < 0) a = 1+8;
		if(yd < 0) b = 4+8;
	}
	else
	{
		a = 3;
		b = 2; 
		if(xd < 0) a = 1;
		if(yd < 0) b = 4;
	}
	if(xd == 0) d = b;
	else if(yd == 0) d = a;
		else if(abv(xd / yd) >= 1)
		{		
			if(m->c <= abv(xd/yd)) d = a;
			else
			{
				m->c = 0;
				d = b;
			}
		}
			else if(m->c <= abv(yd/xd)) d = b;
			else
			{
				m->c = 0;
				d = a;
			}
			
	m->c ++;
}
// SLUT målsøgning ----------------------------------------------------------*/

//printf("x:%d y:%d xt:%d yt:%d xp:%d yp:%d xd:%d yd:%d type:%d NumFood:%d \n",m->xpos,m->ypos,xt,yt,xp,yp,xd,yd,m->type,f->NumFood);
//printf("food1:%d x:%d y:%d food2:%d x:%d y:%d \n",m->foodn[0],m->foodx[0],m->foody[0],m->foodn[1],m->foodx[1],m->foody[1]);	
// START Hvor er jeg nu?------------------------------------------------------------
	switch (d)
	{
		case 1: m->xpos ++;break;
		case 2: m->ypos --;break;
		case 3: m->xpos --;break;
		case 4: m->ypos ++;break;
		case 9: m->xpos ++;break;
		case 10: m->ypos --;break;
		case 11: m->xpos --;break;
		case 12: m->ypos ++;break;
	}
// SLUT Hvor er jeg nu?------------------------------------------------------------*/
}

	return d;

}

DefineAnt(myresyre,"Myresyre#11FFFF",myresyre,struct hjerne);
