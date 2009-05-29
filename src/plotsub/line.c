#include "copyright.h"
#include <stdio.h>
#include "options.h"
#include "sm.h"
#include "devices.h"
#include "defs.h"
#include "sm_declare.h"
#define TOP     8
#define BOTTOM  4
#define RIGHT   2
#define LEFT    1
#define IN      0

extern int offscreen;			/* is the current point offscreen? */
/*
 * draw line in user coords
 */
void
sm_line(xa,ya,xb,yb)
double xa,ya,xb,yb;
{
   long ixa, iya, ixb, iyb, xc, yc, xd, yd;

   ixa = ffx + fsx * xa;
   iya = ffy + fsy * ya;
   ixb = ffx + fsx * xb;
   iyb = ffy + fsy * yb;

   if(cross(ixa,iya,ixb,iyb,gx1,gy1,gx2,gy2,&xc,&yc,&xd,&yd) >= 0) {
      draw_dline(xc,yc,xd,yd);
   }
   xp = ixb;
   yp = iyb;
}

void
draw_dline(xa,ya,xb,yb)                      /* draw line in device   */
long xa,ya,xb,yb;
{
   xa = MAX (MIN(xa, SCREEN_SIZE),0);
   ya = MAX (MIN(ya, SCREEN_SIZE),0);
   xb = MAX (MIN(xb, SCREEN_SIZE),0);
   yb = MAX (MIN(yb, SCREEN_SIZE),0);
   
   if(lltype <= 0 && llweight < 2) {	/* line supported in hardware */
      if(offscreen || xa != xp || ya != yp) {
	 LINE((int)xa,(int)ya,(int)xb,(int)yb);
	 offscreen = 0;
      } else {
	 DRAW((int)xb,(int)yb);
      }
   } else {
      chopper(xa,ya,xb,yb);
   }
   xp = xb; yp = yb;
}
