/* MyreKrig XWindows System-functions version 0.12.3 (24.09.03) */
/* Copyright (c) 1998-2002 Aske Simon Christensen */

/* 0.1.0 (17.02.98): Første version, uden farver. */
/* 0.2.0 (18.02.98): Farver! */
/* 0.2.1 (18.02.98): Sletter skærm melem kampe. */
/* 0.3.0 (19.02.98): Tur-tæller. */
/* 0.3.1 (19.02.98): Kamp-tæller. */
/* 0.3.2 (19.02.98): Antal kampe. */
/* 0.4.0 (10.03.98): Hold-navne og -grafer. */
/* 0.5.0 (10.03.98): Opdaterer vinduet korrekt og quittes med midterste knap.*/
/* 0.5.1 (11.03.98): Baser hvide og hold-grafer præcise. */
/* 0.5.2 (22.03.98): SysCheck() returnerer hvilken museknap. */
/* 0.5.3 (03.04.98): Holdgraf-placerings-bug rettet. */
/* 0.5.4 (07.04.98): SquareChanged() optimeret. */
/* 0.5.5 (14.05.98): Slem myre-med-mad-farve-bug rettet. */
/* 0.5.6 (06.06.98): Benytter NumTeamsVar. */
/* 0.5.7 (08.07.98): Understøtter F-taster. */
/* 0.6.0 (18.06.99): Understøtter variation af vinduestørrelser. (MyreKrig 1.6.1) */
/* 0.6.1 (26.06.99): Tilpasset til holdnummerering fra 1. (MyreKrig 1.6.4) */
/* 0.6.2 (22.10.00): Base- og myretællere. Clearer rigeligt til højre. */
/* 0.7.0 (14.11.00): Tegner med XPutImage. */
/* 0.7.1 (22.11.00): Kan køre i baggrunden. Korrekte farver ved mere end 8 hold. */
/* 0.8.0 (01.12.00): Viser myrernes territorier. */
/* 0.9.0 (20.12.00): Tager imod argumenter. */
/* 0.9.1 (23.02.01): Smårettelser med henblik på kompatibilitet. */
/* 0.9.2 (11.06.01): Autoskalering af grafer med skala og win-line. */
/* 0.9.3 (09.10.01): Sjældnere opdatering af vinduestitlen. */
/* 0.9.4 (23.01.02): Variabel team stats opdatering. */
/* 0.10.0(09.04.02): Eksplicitte holdfarver. */
/* 0.11.0(19.04.02): Skalerer vinduet løbende udfra størrelse og antal hold. */
/* 0.11.1(19.08.02): Argument til at skjule myrer uden mad. */
/* 0.12.0(27.06.03): Tidslinie-grafer. */
/* 0.12.1(13.07.03): Smårettelser af opdatering. */
/* 0.12.2(12.09.03): Flere opdateringsrettelser. */
/* 0.12.3(24.09.03): Ingen huller i grafen, graf-vinduestitel. Root-rettelser. */

/* Skal snart laves: Visning af FoodOwn og FoodTouch. */

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "MyreKrig.h"

static int update = 0;
static int statupd = 1;
static int root = 0;
static int terri = 1;
static int showants = 1;
static int graph = 1;

static clock_t title_time = 0;

static Display *xdisplay = 0;
static Window xwindow = 0;
static int xscreen;
static GC xgc = 0;
static Colormap cmap;
static XWindowAttributes xwinattr;
static int depth,pixelsize;
static XImage *image;
static char *imagedata;

static int root_width;
static int root_height;

static unsigned long *colpix;

static Window gwindow;
static GC ggc = 0;
static int g_width = 500;
static int g_height = 400;
static int g_turns = 1000;
static int g_ants = 1000;
static int g_origo_x = 0;
static int g_origo_y = 400;
static int **g_data;
static int g_last_x = -1;

void drawGraph(int turns);
int drawGraph_x(int turns, int x);

int SysGameInit(int argc, char *argv[]) {
  int i;

  /* Kig argumenterne igennem. */
  for(i = 0 ; i < argc ; i++) {
    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 'u':
	update = atoi(&argv[i][2]);
	break;
      case 's':
	statupd = atoi(&argv[i][2]);
	break;
      case 'r':
	if (argv[i][2] == 'v') {
	  root = 2;
	} else {
	  root = 1;
	}
	break;
      case 't':
	terri = 0;
	break;
      case 'a' :
	showants = 0;
	break;
      case 'g':
	graph = 0;
	break;
      default:
	printf("X Commandline help:\n"
	       "\n"
	       "Arg  Variable\n"
	       "--------------------\n"
	       "-u<num>  Update map every <num> turns.\n"
	       "         If <num> is 0, update each pixel as it changes.\n"
	       "-s<num>  Update team stats every <num> turns.\n"
	       "         If <num> is 0, draw no team stats.\n"
	       "-r       Run in the root window.\n"
	       "-rv      Run in the root window, with vertical layout.\n"
	       "-t       Do not show team territories.\n"
	       "-a       Do not show ants without food.\n"
	       "-g       Do not draw timeline graphs.\n"
	       "\n"
	       );
	return 0;
	break;
      }
    }
  }

  /* Alloker graph data */
  if (graph) {
    int i;
    g_data = ((int **)malloc((Max.BattleSize+1)*sizeof(int *)))+1;
    for (i = -1 ; i < (int)Max.BattleSize ; i++) {
      g_data[i] = (int *)malloc(Max.TimeOutTurn*sizeof(int));
      g_data[i][0] = 0;
    }
  }

  /* Åbn vindue på default display. */
  if((xdisplay = XOpenDisplay(""))) {
    if(root) {
      xwindow = DefaultRootWindow(xdisplay);
      XGetWindowAttributes(xdisplay,xwindow,&xwinattr);
      root_width = xwinattr.width;
      root_height = xwinattr.height;
      if (graph) {
	gwindow = xwindow;
      }
    } else {
      xwindow = XCreateSimpleWindow (xdisplay, DefaultRootWindow(xdisplay),
                                     42, 42, 42, 42,
                                     2,0,0);
      if (graph) {
	gwindow = XCreateSimpleWindow (xdisplay, DefaultRootWindow(xdisplay),
				       42, 42, g_width, g_height,
				       2,0,0);
      }
    }
      
    /* Find ud af ting */
    XGetWindowAttributes(xdisplay,xwindow,&xwinattr);
    depth = xwinattr.depth;
    pixelsize = depth <= 8 ? 1 : 4;

    if(!root) {
      /* Vi vil høre om museknapper, taster og exposure. */
      XSelectInput(xdisplay, xwindow, ButtonPressMask | KeyPressMask | ExposureMask);
      XMapRaised (xdisplay, xwindow);
      
      if (graph) {
      /* Hiv vinduet forest. */
	XSelectInput(xdisplay, gwindow, ExposureMask);
	XMapRaised (xdisplay, gwindow);
      }
    } else {
      /* Vi vil kun høre om exposure. */
      XSelectInput(xdisplay, xwindow, ExposureMask);
    }
    
    /* Sæt diverse X-ting op. */
    xscreen = DefaultScreen (xdisplay);
    xgc = XCreateGC (xdisplay, xwindow, 0, 0);
    if (graph) {
      ggc = XCreateGC (xdisplay, gwindow, 0, 0);
    }
    cmap = DefaultColormap (xdisplay,xscreen);
      
    return 1;
  }
  return 0;
}

int SysBattleInit() {
  int c;
  int w_width,w_height;

  /* Alloker alle de farver, vi skal bruge. */
  colpix = (unsigned long *)malloc((9+Used.BattleSize*3)*sizeof(unsigned long));
  for(c = 0 ; c < 9+Used.BattleSize*3 ; c++) {
    XColor color;
    if(c == 0) {
      color.red   = 0x0000;
      color.green = 0x0000;
      color.blue  = 0x0000;
    } else if(c <= 8) {
      color.red   = 0x2700 + 0x1800*c;
      color.green = 0x2700 + 0x1800*c;
      color.blue  = 0x2700 + 0x1800*c;
    } else {
      int t = BattleTeams[c/3-3];
      u_long col = TeamDatas[t].Color;
      int r = (col >> 16) & 0xff;
      int g = (col >>  8) & 0xff;
      int b = (col >>  0) & 0xff;
      switch(c%3) {
      case 0:
	/* Dark version */
	color.red   = r << 6;
	color.green = g << 6;
	color.blue  = b << 6;
	break;
      case 1:
	/* Normal version */
	color.red   = r << 8;
	color.green = g << 8;
	color.blue  = b << 8;
	break;
      case 2:
	/* Light version */
	color.red   = (r << 7) + 0x8000;
	color.green = (g << 7) + 0x8000;
	color.blue  = (b << 7) + 0x8000;
	break;
      }
    }
    XAllocColor(xdisplay, cmap, &color);
    colpix[c] = color.pixel;
  }
 
  imagedata = (char *)malloc(Used.MapWidth*Used.MapHeight*pixelsize);
  memset(imagedata,colpix[0],Used.MapWidth*Used.MapHeight*pixelsize);
  image = XCreateImage(xdisplay,DefaultVisual(xdisplay,xscreen),
			 depth,ZPixmap,0,imagedata,Used.MapWidth,Used.MapHeight,32,Used.MapWidth*pixelsize);
  image->byte_order = LSBFirst;

  w_width = Used.MapWidth;
  w_height = Used.MapHeight+(statupd ? Used.BattleSize*10 : 0);
  XResizeWindow(xdisplay, xwindow, w_width, w_height);
  XSetForeground(xdisplay, xgc, colpix[0]);
  if (root) {
    XFillRectangle(xdisplay, xwindow, xgc, 0,0, root_width,root_height);
  } else {
    XFillRectangle(xdisplay, xwindow, xgc, 0,0, Used.MapWidth,Used.MapHeight);
  }

  if (graph) {
    if (root) {
      switch(root) {
      case 1:
	g_origo_x = w_width;
	g_origo_y = root_height;
	g_width = root_width-w_width;
	g_height = root_height;
	break;
      case 2:
	g_origo_x = 0;
	g_origo_y = root_height;
	g_width = root_width;
	g_height = root_height-w_height;
	break;
      }
    }
    g_turns = 1000;
    g_ants = 1000;
    drawGraph(0);
  }

  return 1;
}

void SysSquareChanged(int x, int y) {
  struct SquareData sd = MapData(x,y);
  int col = colpix[(sd.Base ? 8 :
		    sd.NumAnts && sd.NumFood ? TeamIndex[sd.Team]*3+11 :
		    sd.NumAnts && showants ? TeamIndex[sd.Team]*3+10 :
		    terri && sd.Team && !sd.NumFood ? TeamIndex[sd.Team]*3+9 :
		    sd.NumFood < 29 ? (sd.NumFood+3)/4 : 8)];

  if(pixelsize == 1) {
    if (update == 0 && imagedata[x+y*Used.MapWidth] != col) {
      XSetForeground(xdisplay, xgc, col);
      XDrawPoint(xdisplay,xwindow,xgc,x,y);
    }
    imagedata[x+y*Used.MapWidth] = col;
  } else {
    if (update == 0 && ((int *)imagedata)[x+y*Used.MapWidth] != col) {
      XSetForeground(xdisplay, xgc, col);
      XDrawPoint(xdisplay,xwindow,xgc,x,y);
    }
    ((int *)imagedata)[x+y*Used.MapWidth] = col;
  }

}

void SysDrawMap() {
  char title[100];
  char *titleptr = title;
  XTextItem xti = {0,0,0,0};
  XTextProperty titleTP;
  int i;

  int max_value;
  int max_terri;

  int bar_start = 162;
  int bar_width = Used.MapWidth-bar_start;

  int winpercent;
  int winline;
  int bar_winline;

  max_value = 0;
  max_terri = bar_width;
  for (i = 0 ; i < Used.BattleSize ; i++) {
    int t = BattleTeams[i];
    if (TeamDatas[t].NumBases*BaseValue+TeamDatas[t].NumAnts > max_value)
      max_value = TeamDatas[t].NumBases*BaseValue+TeamDatas[t].NumAnts;
    if (TeamDatas[t].SquareOwn/Used.NewFoodSpace > max_terri)
      max_terri = TeamDatas[t].SquareOwn/Used.NewFoodSpace;
  }
  if (max_value > max_terri)
    max_terri = max_value;

  winpercent = CurrentTurn >= Used.HalfTimeTurn ?
    Used.HalfTimePercent : Used.WinPercent;
  winline = max_value*100/winpercent-max_value;
  bar_winline = winline*bar_width/max_terri;

  if (update != 0 && CurrentTurn%update == 0) {
    XPutImage(xdisplay,xwindow,xgc,image,0,0,0,0,Used.MapWidth,Used.MapHeight);
  }

  if (statupd && CurrentTurn%statupd == 0) {
    for (i = 0 ; i < Used.BattleSize ; i++) {
      int t = BattleTeams[i];
      int basesize = TeamDatas[t].NumBases*BaseValue;
      int popsize = TeamDatas[t].NumAnts;
      int areasize = TeamDatas[t].SquareOwn/Used.NewFoodSpace;

      int bar_basesize = basesize*bar_width/max_terri;
      int bar_popsize = popsize*bar_width/max_terri;
      int bar_areasize = areasize*bar_width/max_terri;

      char numbuf[10] = "";

      int x;

      /* Sort */
      XSetForeground(xdisplay,xgc,colpix[0]);
      XFillRectangle(xdisplay,xwindow,xgc,0,Used.MapHeight+i*10,Used.MapWidth,10);

      /* Skala */
      for (x = 100 ; x < max_terri ; x += 100) {
	int bar_x = x*bar_width/max_terri;
	XSetForeground(xdisplay,xgc,colpix[x%1000 == 0 ? 4 : 2]);
	XFillRectangle(xdisplay,xwindow,xgc,bar_start+bar_x,Used.MapHeight+i*10,1,10);
      }

      /* Vinder-linie */
      XSetForeground(xdisplay,xgc,colpix[6]);
      XFillRectangle(xdisplay,xwindow,xgc,bar_start+bar_winline,Used.MapHeight+i*10,1,10);

      /* Territoriefarve */
      XSetForeground(xdisplay,xgc,colpix[i*3+9]);
      XFillRectangle(xdisplay,xwindow,xgc,bar_start,Used.MapHeight+i*10+3,bar_areasize,5);

      /* Hvid */
      XSetForeground(xdisplay,xgc,colpix[8]);
      XFillRectangle(xdisplay,xwindow,xgc,bar_start,Used.MapHeight+i*10+3,bar_basesize,5);
      sprintf(numbuf,"%2ld",TeamDatas[t].NumBases);
      /* Base-antal */
      xti.chars = numbuf;
      xti.nchars = 2;
      XDrawText(xdisplay,xwindow,xgc,60,Used.MapHeight+i*10+9,&xti,1);

      /* Farve */
      XSetForeground(xdisplay,xgc,colpix[i*3+10]);
      sprintf(numbuf,"%7ld",TeamDatas[t].SquareOwn);
      /* Territorie-tal */
      xti.chars = numbuf;
      xti.nchars = 7;
      XDrawText(xdisplay,xwindow,xgc,108,Used.MapHeight+i*10+9,&xti,1);

      /* Navn */
      xti.chars = TeamDatas[t].Title;
      xti.nchars = strlen(xti.chars);
      XDrawText(xdisplay,xwindow,xgc,0,Used.MapHeight+i*10+9,&xti,1);
      XFillRectangle(xdisplay,xwindow,xgc,bar_start+bar_basesize,Used.MapHeight+i*10+3,bar_popsize,5);

      /* Lys farve */
      XSetForeground(xdisplay,xgc,colpix[i*3+11]);
      sprintf(numbuf,"%6ld",TeamDatas[t].NumAnts);
      /* Myre-antal */
      xti.chars = numbuf;
      xti.nchars = 6;
      XDrawText(xdisplay,xwindow,xgc,72,Used.MapHeight+i*10+9,&xti,1);
    }

  }

  /* Vinuestitel */
  if (clock()-title_time >= CLOCKS_PER_SEC/10) {
    title_time = clock();
    sprintf(title,"Kamp: %d / %ld  Tur: %ld",BattleCount+1,Args.NumBattles,CurrentTurn);
    XStringListToTextProperty(&titleptr,1, &titleTP);
    XSetWMName(xdisplay,xwindow,&titleTP);
  }

  /* Graph */
  if (graph) {
    int i;
    int new_x;
    g_data[-1][CurrentTurn] = winline;
    for (i = 0 ; i < Used.BattleSize ; i++) {
      int team = BattleTeams[i];
      g_data[i][CurrentTurn] = TeamDatas[team].NumAnts+TeamDatas[team].NumBases*BaseValue;
    }
    if (CurrentTurn >= g_turns) {
      g_turns += 1000;
      drawGraph(CurrentTurn);
    }
    if (max_value >= g_ants) {
      g_ants += 1000;
      drawGraph(CurrentTurn);
    }
    new_x = (CurrentTurn-1)*g_width/g_turns;
    while (g_last_x < new_x-1) {
      drawGraph_x(CurrentTurn, ++g_last_x);
    }
    drawGraph_x(CurrentTurn, new_x);
  }

  XFlush(xdisplay);
}

void drawGraph(int turns) {
  char title[100];
  char *titleptr = title;
  XTextProperty titleTP;
  int i,x;

  sprintf(title,"Graf - %d ture - %d point", g_turns, g_ants);
  XStringListToTextProperty(&titleptr,1, &titleTP);
  XSetWMName(xdisplay,gwindow,&titleTP);

  XSetForeground(xdisplay, ggc, colpix[0]);
  XFillRectangle(xdisplay, gwindow, ggc, g_origo_x,g_origo_y-g_height, g_width,g_height);

  for (i = 1 ; i < g_turns/100 ; i++) {
    XSetForeground(xdisplay, ggc, colpix[i%10 == 0 ? 4 : 2]);
    XFillRectangle(xdisplay, gwindow, ggc, g_origo_x+i*100*g_width/g_turns,g_origo_y-g_height, 1,g_height);
  }
  for (i = 1 ; i < g_ants/100 ; i++) {
    XSetForeground(xdisplay, ggc, colpix[i%10 == 0 ? 4 : 2]);
    XFillRectangle(xdisplay, gwindow, ggc, g_origo_x,g_origo_y-i*100*g_height/g_ants, g_width,1);
  }

  for (x = 0 ; x < g_width ; x++) {
    if (drawGraph_x(turns, x)) break;
    g_last_x = x;
  }
}

int drawGraph_x(int turns, int x) {
  int i,turn;
  int turn_min = x*g_turns/g_width;
  int turn_max = (x+1)*g_turns/g_width;
  if (turn_max > turns) return 1;
  for (i = -1 ; i < (int)Used.BattleSize ; i++) {
    int ants_min = MaxAnts;
    int ants_max = 0;
    int y_top, y_bottom;
    for (turn = turn_min ; turn <= turn_max ; turn++) {
      int ants = g_data[i][turn];
      if (ants < ants_min) ants_min = ants;
      if (ants > ants_max) ants_max = ants;
    }
    y_top = -ants_max*g_height/g_ants;
    y_bottom = -ants_min*g_height/g_ants;
    
    XSetForeground(xdisplay,ggc,colpix[i == -1 ? x&8 ? 6 : 3 : i*3+10]);
    XFillRectangle(xdisplay,gwindow,ggc, g_origo_x+x,g_origo_y+y_top, 1,y_bottom-y_top+1);
  }
  return 0;
}


int SysCheck() {
  XEvent xevent;
  int key;
  int redraw = 0;
  while(XCheckWindowEvent(xdisplay,xwindow,ButtonPressMask | KeyPressMask | ExposureMask, &xevent)) {
    switch(xevent.type) {
    case Expose:
      redraw = 1;
      break;
    case ButtonPress:
      return xevent.xbutton.button; /* Museknap nummer! */
    case KeyPress:
      key = xevent.xkey.keycode;
      if(key >= 15 && key <= 87 && key%8 == 7) {
        return key/8; /* F-tast! */
      }
      break;
    }
  }
  if(redraw) {
    if (root) {
      XSetForeground(xdisplay, xgc, colpix[0]);
      XFillRectangle(xdisplay, xwindow, xgc, 0,0, root_width,root_height);
    }
    XPutImage(xdisplay,xwindow,xgc,image,0,0,0,0,Used.MapWidth,Used.MapHeight);
  }
  if (graph) {
    if (!root) redraw = 0;
    while (XCheckWindowEvent(xdisplay,gwindow, ExposureMask, &xevent)) {
      redraw = 1;
    }
    if (redraw) {
      if (!root) {
	XGetWindowAttributes(xdisplay,gwindow,&xwinattr);
	g_width = xwinattr.width;
	g_height = xwinattr.height;
	g_origo_x = 0;
	g_origo_y = g_height;
      }
      drawGraph(CurrentTurn);
    }
  }

  return 0;
}

void SysBattleExit() {
  if(imagedata) { free(imagedata); imagedata = 0; }
  XFreeColors(xdisplay, cmap, colpix, 9+Used.BattleSize*3, 0);
  if(colpix) { free(colpix); colpix = 0; }
}

void SysGameExit() {
  if(xgc) XFreeGC(xdisplay, xgc);
  if(xwindow) XDestroyWindow(xdisplay,xwindow);
}
