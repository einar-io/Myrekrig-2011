// "Inkal" af Steffen Dyhr-Nielsen    NOONTZ

#include "Myre.h"


#define nytype m->T=(m->T & 0xe0)+
#define xp m->A
#define yp m->B
#define xd (xp - xt)
#define yd (yp - yt)
#define navi ((m->T & 0xe0)>>5)
#define type (m->T & 0x1f)
#define abv(a) ((a) >= 0 ? (a) : -(a))
#define ft(a) ((a) >= 0 ? 1 : -1)
#define ft2(a) ((a) == 0 ? -1 : 1)
#define Qt(x,y) (((2*ft(x)+ft(y) == 3)&&(x!=0)) ? 1 : 2*ft(x)+ft(y) == 1 ? 2 : ((2*ft(x)+ft(y) == -3)||(y==0)) ? 3 : 4)
#define W 60 // Delta baser
#define W1 90 // St›rrelse  b›lge
#define W1H 4 // Hyppighed f›rste b›lge
#define indre_1 3
#define indre_2 -1
#define ydre_1 11
#define ydre_2 3

enum {baby,barn,unge,afsted,afstedbid,afstedbase,sulten,
	inkalen,hjemad,hjemadmad,hjemadbid,mad,spiral,kvadrant,basecamp,
	drone,vagt,suppe,stafet,afstedbidfarvel,dronepuppe,kok,
	afstedbid_3_4,afstedbid_2_2,afstedbid_1_1,afstedbid_4_1,afstedbid_3_3,afstedbid_2_3,afstedbid_1_2,afstedbid_4_4,};

struct hjerne
{
	signed char A,B,C,D;
	char	E,T;	
};

int Inkal(struct SquareData *f,struct hjerne *m)

{
	unsigned char a,b=0,d=0,X=0;
	char xt=0,yt=0;
	if((type==mad)||(type==inkalen)||(type==kvadrant))return 0;

// VAGT
	for (a=1;a<=4;a++)
	{
		if(f[a].Team>=1)
		{
			nytype vagt;
			m->E=0;
			switch (a)
			{
				case 1: xp ++;break;
				case 2: yp --;break;
				case 3: xp --;break;
				case 4: yp ++;break;
			}
			return a;
		}
	}
	if(type==vagt)
	{
		for (a=0;a<=4;a++)
		{
			if((f[a].NumFood>0)&&(f[a].NumAnts==0))
			{
				nytype sulten;
				switch (a)
				{
					case 1: xp ++;break;
					case 2: yp --;break;
					case 3: xp --;break;
					case 4: yp ++;break;
				}
			return a;
			}
		}
		return 0;
	}
// DRONEPUPPE
	if(type==dronepuppe)
	{
			if (f->NumAnts<40)
			{
			 	nytype vagt;
			 	return 0;
			}
			else
			{
				for(a=1;a<=f->NumAnts-1;a++)
				{
					if((m[a].T & 0x1f)==drone)X=1;
				}
				
				if(X==0)m->T=(m->T & 0xe0)+drone;
				else nytype barn;
			}
	}
	
	if(type==drone)
	{
		if((f->NumAnts>=40)&&(f->NumFood>=50))return 16;
		else return 0;
	}


// KOK
	if(type==kok)
	{
		m->E++;

		if(f->NumFood==1)
		{
			nytype hjemadbid;
			m->C=0;
			m->D=0;
			m->E=0;
			m->T = (m->T & 0x1f)+(1<<5);
		}
		else if(f->NumFood==0)
		{
			nytype hjemad;
			m->C=0;
			m->D=0;
			m->E=0;
			m->T = (m->T & 0x1f)+(1<<5);
		}
		else if(m->E==125)
		{
			nytype hjemadmad;
			m->C=xp;
			m->D=yp;
			m->E=f->NumFood;
			m->T = (m->T & 0x1f)+(1<<5);
		}
		else return 0;
	}			




// SUPPE	
	if(type==suppe)
	{
		
		if(f->Base==1)
		{
			switch(Qt(m->C,m->D))
			{
				{
					case 1: return 1;
					case 2: return 2;
					case 3: return 3;
					case 4: return 4;
				}
			}
		}
		else
		{
			m->B++;
			if(m->B>=100)
			{
				m->A++;
				m->B=0;
			}
			
			if(m->A>2)
			{	
				X=0;
				for(a=1;a<=f->NumAnts-1;a++)
				{
					switch(m[a].T & 0x1f)
					{
						case kvadrant:X++;break;
					}
				}
				
				if((m->T & 0xe0)>>5!=0)X++;
				
				if(X==0)
				{
					
					switch(Qt(m[b].C,m[b].D))
					{
						
						case 1: m->C=W;m->D=W;break;
						case 2: m->C=W;m->D=-W;break;
						case 3: m->C=-W;m->D=-W;break;
						case 4: m->C=-W;m->D=W;break;
					}
					m->B=0;
					m->A=0;
					m->E=0;
					
					m->T=(m->T & 0xe0)+kvadrant;
					return 0;
				}					
				else
				{
					switch(Qt(m->C,m->D))
					{
						case 1: nytype barn;m->A=0;m->B=0;return 3;
						case 2: nytype barn;m->A=0;m->B=0;return 4;
						case 3: nytype barn;m->A=0;m->B=0;return 1;
						case 4: nytype barn;m->A=0;m->B=0;return 2;
					}
				}
			}
		}
	}
	
// HJEMADMAD
	if(type==hjemadmad)
	{
		if(abv(xp)+abv(yp)>1)
		{
			switch(Qt(m->C,m->D))
			{
				case 1: xt=1;yt=0;break;
				case 2: xt=0;yt=-1;break;
				case 3: xt=-1;yt=0;break;
				case 4: xt=0;yt=1;break;
			}
		}
		else if(abv(xp)+abv(yp)==1)
		{
			for(a=1;a<=f->NumAnts-1;a++)
			{
				if(((m[a].T & 0x1f)==suppe)&&(m[a].C==m->C)&&(m[a].D==m->D))nytype hjemadbid;
			}
		}			
		else if(type == hjemadmad)
		{
			nytype mad;
			
			for(a=1;a<=f->NumAnts-1;a++)
			{
				if((((m[a].T & 0x1f)==mad)||((m[a].T & 0x1f)==suppe))&&(m[a].C==m->C)&&(m[a].D==m->D))nytype barn;
			}
			if(type==mad)return 0;
		}
	}
// HJEMADBID
	if(type==hjemadbid)
	{
		if(abv(xp)+abv(yp)>1)
		{
			for (a=1;a<=4;a++)
			{
				if((f[a].NumFood>0)&&(f[a].NumAnts==0))
				{
					nytype sulten;
					switch (a)
					{
						case 1: xp ++;break;
						case 2: yp --;break;
						case 3: xp --;break;
						case 4: yp ++;break;
					}
				return a+8;
				}
			}
			switch(Qt(xp,yp))
			{
				case 1: xt=1;yt=0;break;
				case 2: xt=0;yt=-1;break;
				case 3: xt=-1;yt=0;break;
				case 4: xt=0;yt=1;break;
			}
		}
		else if(abv(xp)+abv(yp)==1)
		{
			xt=0;
			yt=0;
			for(a=1;a<=f->NumAnts-1;a++)
			{
				if((m[a].T & 0x1f)==kvadrant)
				{
					X=0;
					m[a].A++;
					if(m[a].A %W1H ==0)
					{
						if(((m[a].T & 0xe0)>>5)<W1H)d=1;
					}
					if(m[a].A==W1)
					{
						m[a].A=0;
						m[a].T=m[a].T+(1<<5);
					}

					
					if(d == 1)
					{
						if((m[a].E & 0x80)>>7==0)m[a].E=(m[a].E & 0x7f)+1;
						if((m[a].E & 0x3f)>20)m[a].E=(m[a].E & 0x40);
						m[a].E=m[a].E+(1<<7);
						m->E=m[a].E+(X<<6);
						m->C=m[a].C;
						m->D=m[a].D;
						m->T=basecamp+(1<<5);
						switch(Qt(xp,yp))
						{
							
							case 1: xp++;return 9;
							case 2: yp--;return 10;
							case 3: xp--;return 11;
							case 4: yp++;return 12;
						}
					}
					else
					{
						switch(Qt(xp,yp))
						{
							case 1: xp=0;yp=0;return 11;
							case 2: xp=0;yp=0;return 12;
							case 3: xp=0;yp=0;return 9;
							case 4: xp=0;yp=0;return 10;
						}
					}						
				}
			}
		}
		else if((xp==0)&&(yp==0))
		{
			if(f->Base==1)nytype barn;
			else
			{
				m->T = dronepuppe+((m->E)<<5);
				m->A=-30;
				m->B=-10;
				m->C=0;
				m->D=-1;
				m->E=1;
			}
		}			
			

		
		
	}

// BASECAMP
	if(type==basecamp)
	{
		if(f->NumAnts>=W1)
		{

			for(a=1;a<=f->NumAnts-1;a++)
			{
				{
					m[a].T=afstedbase+(1<<5);
					m[a].E=(m[a].E & 0xbf);
				}
			}

		
		m->T=stafet+(Qt(m->C,m->D)<<5);
		
		switch(Qt(m->C,m->D))
			{
				{
					case 1: return 11;
					case 2: return 12;
					case 3: return 9;
					case 4: return 10;
				}
			}
		}
	}

// STAFET
		
if(type==stafet)
{
	for(a=1;a<=4;a++)
	{
		if(f[a].Base==1)return a;
	}
	
	if(f->Base==1)
	{
		for(a=1;a<=f->NumAnts-1;a++)
		{

			if((m[a].T & 0x1f)==inkalen)
			{
				m[a].E=m[a].E+(1<<((Qt(m->C,m->D)+3)));
				nytype barn;
				
			}
		}
	}
}

// BABY	
	if(type==baby)
	{
		X=0;
		
		nytype barn;

		for(a=1;a<=f->NumAnts-1;a++)
		{
			switch(m[a].T & 0x1f)
			{
				case drone: 
				case inkalen: X++;a=f->NumAnts;break;
			}
		}
			
		if(X==0)
		{

			m->A=-30;
			m->B=-10;
			m->C=(1<<7);
			m->D=-1;
			m->E=0;
			m->T = inkalen;
			return 0;
		}
					
	}
	
// SULTEN	
	if(type==sulten)
	
	{
		nytype hjemadmad;
		m->C=xp;
		m->D=yp;
		m->E=f->NumFood;
		m->T = (m->T & 0x1f)+(1<<5);
	}
	
// HJEMAD
	if(type==hjemad)
	{

		for (a=1;a<=4;a++)
		{
			if((f[a].NumFood>0)&&(f[a].NumAnts==0))
			{
				nytype sulten;
				switch (a)
				{
					case 1: xp ++;break;
					case 2: yp --;break;
					case 3: xp --;break;
					case 4: yp ++;break;
				}
			return a;
			}
		}
		if((xp==0)&&(yp==0))
		{
			if(f->Base==1)nytype barn;
			else
			{
				m->T = dronepuppe+((m->E)<<5);
				m->A=-30;
				m->B=-10;
				m->C=0;
				m->D=-1;
				m->E=1;
			}
		}
	}


// BARN
	if(type==barn)
	{
		
		char Ex = 0;
		char Dx = 0;

		nytype unge;
		
		
		for(a=1;a<=f->NumAnts-1;a++)
		{
			if((m[a].T & 0x1f)==mad)
			{

				d=1;
				b=a;
				a=f->NumAnts;
			}
			if((m[a].T & 0x1f)==inkalen)Ex=(m[a].E & 0xf0);
	
			if((m[a].T & 0x1f)==drone)Dx=((m[a].T & 0xe0)>>5);
			
		}		

		if(d==1)
		{
			
			for(a=b+1;a<=f->NumAnts-1;a++)
			{
				if(((m[a].T & 0x1f)==mad)&&(abv(m[a].C)+abv(m[a].D)<abv(m[b].C)+abv(m[b].D)))b=a;
				if((m[a].T & 0x1f)==inkalen)Ex=(m[a].E & 0xf0);
				if((m[a].T & 0x1f)==drone)Dx=((m[a].T & 0xe0)>>5);
			}
			if(m[b].E>0)
			{
				nytype afstedbid;
				
				if((m[b].A & 0x80)>>7==0)
				{
					m[b].A=(m[b].A & 0x7f)+1;
					if((abv(m[b].C)<20)&&(abv(m[b].D)<20))m[b].A=(2<<5);
				}
				m[b].A=m[b].A + (1<<7);
				m[b].E--;
				if(Dx!=0)
				{
					switch(Dx)
					{
						case 1: if(m[b].C<-W)nytype afstedbid_3_4;if(m[b].D<-W)nytype afstedbid_2_2;break;
						case 2: if(m[b].C<-W)nytype afstedbid_3_3;if(m[b].D>W)nytype afstedbid_4_1;break;
						case 3: if(m[b].C>W)nytype afstedbid_1_2;if(m[b].D>W)nytype afstedbid_4_4;break;
						case 4: if(m[b].C>W)nytype afstedbid_1_1;if(m[b].D<-W)nytype afstedbid_2_3;break;
					}
				}
				else if((((Ex & 0x10)>>4)==1)&&(Qt(m[b].C,m[b].D)==1))nytype afstedbidfarvel;
				else if((((Ex & 0x20)>>5)==1)&&(Qt(m[b].C,m[b].D)==2))nytype afstedbidfarvel;
				else if((((Ex & 0x40)>>6)==1)&&(Qt(m[b].C,m[b].D)==3))nytype afstedbidfarvel;
				else if((((Ex & 0x80)>>7)==1)&&(Qt(m[b].C,m[b].D)==4))nytype afstedbidfarvel;
				xp=0;
				yp=0;
				m->E=m[b].A;
				m->C=m[b].C;
				m->D=m[b].D;
				m->T = (m->T & 0x1f)+(1<<5);
				X=0;
			}
			else 
			{

				m[b].E=0;
				m[b].A=0;
				m[b].B=0;
				
				m[b].T = suppe+(Dx<<5);
			}
		}
	}


// UNGE	
	if(type==unge)
	{
		

		char Indre=indre_1;
		char Ydre=ydre_1;

		for(a=1;a<=f->NumAnts-1;a++)
		{
			X=0;
			d=0;
			if((m[a].T & 0x1f)==drone)
			{
				d=1;
				switch((m[a].T & 0xe0)>>5)
				{
					case 1:if((m[a].A<0)&&(m[a].B<0))X=1;break;
					case 2:if((m[a].A<0)&&(m[a].B>0))X=1;break; 
					case 3:if((m[a].A>0)&&(m[a].B>0))X=1;break; 
					case 4:if((m[a].A>0)&&(m[a].B<0))X=1;break;  
				}
			}
			else if((m[a].T & 0x1f)==inkalen)
			{
				d=1;
				if((m[a].E & 0xf0)>>4!=15)
				{
					switch(Qt(m[a].A,m[a].B))
					{
						case 1:if(((m[a].E & 0x10)>>4)==1)X=1;break;
						case 2:if(((m[a].E & 0x20)>>5)==1)X=1;break;
						case 3:if(((m[a].E & 0x40)>>6)==1)X=1;break;
						case 4:if(((m[a].E & 0x80)>>7)==1)X=1;break;
					}
				}
				else
				{
					Indre=indre_2;
					Ydre=ydre_2;
				}
			}
			
			if(d==1)
			{
				unsigned char C=m[a].C & 0x7f;
				unsigned char E=(m[a].C & 0x80)>>7;
				{
					if(m[a].D>Ydre)
					{
						E=1;
						m[a].D=Indre;
						C=0;
						m[a].A=-30;
						m[a].B=-10;
					}
					if(C>=m[a].D*4)
					{
						C=1;
						E=0;
						m[a].A=m[a].A+40;
						m[a].B=m[a].B+20;
						m[a].D=m[a].D+2;
					}
					else
					{
						C++;
						if(((C-2) %(m[a].D)+2)==((C-1) %(m[a].D*2)+1))m[a].B=m[a].B+ft2(E)*20;
						else m[a].A=m[a].A+ft2(E)*20;		
						if(((C-2) %(m[a].D*2)+1)==m[a].D*2)E=1;
					}
	
					m[a].C= C + (E<<7);
				}
				if(X==0)
				{
					nytype afsted;
					xp=0;
					yp=0;
					m->C=m[a].A;
					m->D=m[a].B;
					m->E=0;
					m->T = (m->T & 0x1f)+(1<<5);
					a=f->NumAnts;
				}
				else a--;
			}
		}

	}

// AFSTED
	if(type==afsted)
	{
		xt=m->C;
		yt=m->D;
		for (a=1;a<=4;a++)
		{
			if((f[a].NumFood>0)&&(f[a].NumAnts==0))
			{
				nytype sulten;
				switch (a)
				{
					case 1: xp ++;break;
					case 2: yp --;break;
					case 3: xp --;break;
					case 4: yp ++;break;
				}
			return a;
			}
		}
		if((xd==0)&&(yd==0))
		{
			nytype spiral;
			m->C=-1;
			m->D=1;
			m->E=0;
			m->T = (m->T & 0x1f);
		}
	}

	
// AFSTEDBID && AFSTEDBASE
	if((type==afstedbid)||(type==afstedbidfarvel)||(type==afstedbase)
	||(type==afstedbid_3_4)||(type==afstedbid_2_2)||(type==afstedbid_1_1)||(type==afstedbid_4_1)||(type==afstedbid_3_3)||(type==afstedbid_2_3)||(type==afstedbid_1_2)||(type==afstedbid_4_4))
	{


		if((m->E & 0x60)>>5==0)
		{
			if((m->E & 0x80)>>7==0)
			{
				xt=(m->E & 0x1f)*ft(m->C);
				yt=0;
			}
			else
			{
				yt=(m->E & 0x1f)*ft(m->D);
				xt=0;
			}
			if((yd==0)&&(xd==0))m->E=m->E+(1<<5);
		}
		if((m->E & 0x60)>>5==1)
		{
			if(type!=afstedbase)
			for (a=1;a<=4;a++)
			{
				if((f[a].NumFood>0)&&(f[a].NumAnts==0))
				{
					nytype sulten;
					switch (a)
					{
						case 1: xp ++;break;
						case 2: yp --;break;
						case 3: xp --;break;
						case 4: yp ++;break;
					}
					return a;
				}
			}
			if((m->E & 0x80)>>7==0)
			{
				xt=(m->C);
				yt=m->D-(m->E & 0x1f)*ft(m->D);
			}
			else
			{
				yt=(m->D);
				xt=m->C-(m->E & 0x1f)*ft(m->C);
			}
			if((abv(xd)<5)&&(abv(yd)<5))m->E=m->E+(1<<5);
		}
		if((m->E & 0x60)>>5==2)
		{
			xt=m->C;
			yt=m->D;
					
			// AFSTEDBID
			if((yd==0)&&(xd==0))
			{
	
				if(type!=afstedbase)
				{
					if(type==afstedbidfarvel)
					{
						if(abv(xp)+abv(yp)>=W)
						{
				
							xp=xp-ft(xp)*W;
							yp=yp-ft(yp)*W;
							m->E=Qt(xp,yp);
						}
	
					}
					else
					{
						switch(m->T & 0x1f)
						{
							case afstedbid: m->E=0; break;
							case afstedbid_1_1: xp=xp-2*W;m->E=1;break;
							case afstedbid_1_2: xp=xp-2*W;m->E=2;break;
							case afstedbid_2_2: yp=yp+2*W;m->E=2;break;
							case afstedbid_2_3: yp=yp+2*W;m->E=3;break;
							case afstedbid_3_3: xp=xp+2*W;m->E=3;break;
							case afstedbid_3_4: xp=xp+2*W;m->E=4;break;
							case afstedbid_4_4: yp=yp-2*W;m->E=4;break;
							case afstedbid_4_1: yp=yp-2*W;m->E=1;break;
						}
					}
					if(f->NumFood>0)
					{
						if(f->NumAnts>2)
						{
							nytype hjemadbid;
							m->C=0;
							m->D=0;
							m->T = (m->T & 0x1f)+(1<<5);
						}
						else
						{
							nytype kok;
							return 0;
						}
					}
					else
					{
						nytype hjemad;
						m->C=0;
						m->D=0;
						m->T = (m->T & 0x1f)+(1<<5);
					}
				}
				else   // AFSTEDBASE
				{
					if(f->Base==1)nytype barn;
					else
					{
						m->T = dronepuppe+((Qt(xp,yp))<<5);
						m->A=-30;
						m->B=-10;
						m->C=0;
						m->D=-1;
						m->E=1;
					}
				}
			}
		}
	}
	

// START Spiral------------------------------------------------------------------------------------
if(type == spiral)
{

	char Cs=(m->C & 0x3f);
	char Ds=(m->D & 0x3f);

	for (a=1;a<=4;a++)
	{
		if((f[a].NumFood>0)&&(f[a].NumAnts==0))
		{
			nytype sulten;
			switch (a)
			{
				case 1: xp ++;break;
				case 2: yp --;break;
				case 3: xp --;break;
				case 4: yp ++;break;
			}
			return a;
		}
	}
	if((type!=sulten)||(type!=hjemad))
	{
		
		if (Cs< Ds)
		{
			Cs=Cs+1;
			d = ((m->E %4) + 1);
		}
		else if (navi == 1)
		{
			Ds = Ds + 2;
			Cs = 0;
			m->T = m->T & 0x1f;
			m->E ++;
			d = ((m->E %4) + 1);
		}
		else
		{
			Ds = Ds+1;
			Cs = 0;
			m->T = (m->T & 0x1f)+(1<<5);
			m->E ++;
			d = ((m->E %4) + 1);
		}
	}
	if((Ds==60)&&(type==spiral))
	{
		nytype hjemad;
		m->E=0;
		m->C=0;
		m->D=0;
		m->T = (m->T & 0x1f)+(1<<5);
	}
	m->C=(m->C & 0xc0)+Cs;
	m->D=(m->D & 0xc0)+Ds;
	if((abv(xp)>125)||(abv(yp)>125))
	{
		nytype vagt;
		if(f->NumAnts==1)return 0;
	}
			
}
// SLUT Spiral-----------------------------------------------------------------------------------*/
	
// START M†l---------------------------------------------------------------------------------

if ((type==hjemad)||(type==hjemadmad)||(type==hjemadbid)||(type==afsted)||(type==afstedbid)
	||(type==afstedbase)||(type==afstedbidfarvel)
	||(type==afstedbid_3_4)||(type==afstedbid_2_2)||(type==afstedbid_1_1)||(type==afstedbid_4_1)||(type==afstedbid_3_3)||(type==afstedbid_2_3)||(type==afstedbid_1_2)||(type==afstedbid_4_4))
{
	
	if((type==hjemadmad)||(type==hjemadbid)||(type==afstedbase))
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
			if(navi <= abv(xd/yd)) d = a;
			else
			{
				m->T = (m->T & 0x1f);
				d = b;
			}
		}
		else if(navi <= abv(yd/xd)) d = b;
		else
		{
			m->T = (m->T & 0x1f);
			d = a;
		}
	if(navi<8)m->T =m->T +(1<<5);

}
// SLUT M†l------------------------------------------------------------------------------------*/

// START Hvor er jeg nu?-------------------------------------------------------------------------------

	switch (d)
	{
		case 0: break;
		case 1: xp ++;break;
		case 2: yp --;break;
		case 3: xp --;break;
		case 4: yp ++;break;
		case 9: xp ++;break;
		case 10: yp --;break;
		case 11: xp --;break;
		case 12: yp ++;break;
		case 16: break;
	}
// SLUT Hvor er jeg nu?--------------------------------------------------------------------------------*/
	
	return d;
}

DefineAnt(Inkal,"Inkal#00ff55",Inkal,struct hjerne);
