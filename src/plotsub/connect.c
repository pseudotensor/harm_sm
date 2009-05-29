#include "copyright.h"
#include <stdio.h>
#include <math.h>
#include "options.h"
#include "sm.h"
#include "defs.h"
#include "sm_declare.h"
  
extern int sm_verbose;
/*
 * Here's a function for those benighted souls who use callable SM
 */
int
sm_connect(xarray, yarray, npts)
REAL xarray[], yarray[];
int npts;
{
   return(sm_conn(xarray, yarray, npts));
}

int
sm_conn(xarray, yarray, npts)
REAL xarray[], yarray[];
int npts;
{
   int i;
   float epsilon = 1.0;
   int nfan,*karray;
   
   if(npts < 2) return(-1);

   if(*print_var("fan_compress") == '\0') { /* not defined */
      sm_relocate(xarray[0], yarray[0]);
      for(i = 1;i < npts;i++) {
         sm_draw(xarray[i],yarray[i]);
      }
   } else {				/* fan compress data */
      if((karray = (int *)malloc((unsigned)(npts + 1)*sizeof(int))) == NULL) {
	 msg("Can't allocate fan compression array\n");
	 return(-1);
      }
      fan_compress(xarray, yarray, npts, epsilon, karray, &nfan);
      
      if(sm_verbose) {
	 msg_1d("Fan Compression: Plotting %d of ",nfan);
	 msg_1d("%d points ",npts); 
	 msg_1f("with epsilon = %f\n",epsilon);
      }
      
      sm_relocate(xarray[karray[0]],yarray[karray[0]]);
      for(i = 1;i < nfan;i++) {
	 sm_draw(xarray[karray[i]], yarray[karray[i]]);
      }
      free((char *)karray);
   }
   return(0);
}

/********************************************************/
/*
 * Connect points if larray is true; only connect continuously true segments
 */
int
conn_if(xarray, yarray, larray, npts)
REAL xarray[], yarray[],
      larray[];
int npts;
{
    int connecting = 0,			/* are we currently connecting? */
        i;
    
    if(npts < 2) return(-1);
    for(i = 0;i < npts;i++) {
       if(!larray[i]) {
	  connecting = 0;
	  continue;
       }
       
       if(!connecting) {
	  sm_relocate(xarray[i], yarray[i]);
	  connecting = 1;
       } else {
	  sm_draw(xarray[i], yarray[i]);
       }
    }
    return(0);
}

/********************************************************/

int
sm_histogram(xarray, yarray, npts)
REAL xarray[], yarray[];
int npts;
{
   int i ;
   REAL xav;
   
   if(npts < 2) return(-1);
   sm_relocate(0.5*(3*xarray[0] - xarray[1]), yarray[0]);
   xav = 0.5*(xarray[1] + xarray[0] );
   sm_draw(xav, yarray[0]);
   for (i=1; i<npts; i++ ) {
      xav = 0.5*(xarray[i] + xarray[i-1]);
      sm_draw(xav, yarray[i-1]);
      sm_draw(xav, yarray[i]);
   }
   sm_draw(0.5*(3*xarray[npts-1] - xarray[npts-2]), yarray[npts-1]);
   
   return(0);
}

/********************************************************/
/*
 * Draw a histogram, dropping points where the logical array is false
 *
 * First an auxilery function
 */
static float
check_uniform(x,l,npts)
REAL x[],l[];
int npts;
{
   REAL dx;
   int i;
   int found_dx = 0;			/* we've found a valid interval */
   
   for(i = 0;i < npts - 1;i++) {
      if(!l[i] || !l[i + 1]) {
	 continue;
      }

      if(!found_dx) {
	 found_dx = 1;
	 if((dx = x[i + 1] - x[i]) == 0.0) {
	    return(0.0);
	 }
      } else if(fabs((x[i + 1] - x[i])/dx - 1) > 1e-3) {
	 return(0.0);
      }
   }

   return (found_dx ? dx : 0.0);
}
   
/*
 * Draw a histogram, allowing for missing points (where larray[] is false).
 *
 * This is considerably complicated by the sad fact that some people seem
 * to not provide valid x-coordinates for invalid points; if it weren't for
 * this (and boundaries), xav would be simply (x[i] - x[i-1])/2.
 */
int
histogram_if(xarray, yarray, larray, npts)
REAL xarray[], yarray[],
      larray[];
int npts;

{
   int connecting = 0,
       i;
   int checked_uniform = 0;		/* are points uniformly spaced? */
   REAL dx;				/* spacing, if uniform */
   REAL xav;
   
   if(npts < 2) return(-1);
   
   for(i = 0;i < npts && !larray[i];i++) continue;
   
   for(;i < npts;i++) {
      if(i == 0) {
	 xav = 0.5*(3*xarray[0] - xarray[1]);
      } else {
	 if(larray[i]) {		/* this x value is OK */
	    if(larray[i-1]) {		/* previous value was valid */
	       xav = 0.5*(xarray[i] + xarray[i-1]);
	    } else {
	       if(i == npts - 1) {	/* last point; give up now */
		  connecting = 0;
		  break;
	       }
	       if(larray[i+1]) {	/* next point is valid */
		  xav = 0.5*(3*xarray[i] - xarray[i+1]);
	       } else {			/* an isolated valid point */
		  if(!checked_uniform) { /* see if spacing is uniform */
		     checked_uniform = 1;
		     dx = check_uniform(xarray,larray,npts);
		  }
		  if(dx == 0.0) {	/* not uniform; there's nothing to
					   be done with this point */
		     if(sm_verbose > 1) {
			msg_1f("Point at %g is an isolated valid point\n",
			       xarray[i]);
		     }
		     continue;
		  } else {
		     xav = xarray[i] - dx/2;
		  }
	       }
	    }
	 } else {
	    if(!connecting) {		/* no need for it anyway */
	       continue;
	    }
	    if(!larray[i - 1]) {	/* this is impossible, I think */
	       continue;
	    }

	    if(i > 1 && larray[i - 2]) {
	       xav = 0.5*(3*xarray[i - 1] - xarray[i - 2]);
	    } else {
	       if(!checked_uniform) { /* see if spacing is uniform */
		  checked_uniform = 1;
		  dx = check_uniform(xarray,larray,npts);
	       }
	       if(dx == 0.0) {	/* not uniform; there's nothing to
				   be done with this point */
		  if(sm_verbose > 1) {
		     msg_1f("Point at %g is an isolated valid point\n",
			    xarray[i]);
		  }
		  continue;
	       } else {
		  xav = xarray[i - 1] + dx/2;
	       }
	    }
	 }
      }
      if(!larray[i]) {
	 if(connecting) {
	    sm_draw(xav, yarray[i-1]);
	 }
	 connecting = 0;
	 continue;
      }
      
      if(!connecting) {
	 connecting = 1;
	 sm_relocate(xav,yarray[i]);
      } else {
	 sm_draw(xav,yarray[i-1]);
	 sm_draw(xav,yarray[i]);
      }
   }
   if(connecting) {
      sm_draw(0.5*(3*xarray[npts-1] - xarray[npts-2]), yarray[npts-1]);
   }
   
   return(0);
}

/********************************************************/

int
sm_errorbar( xarray, yarray, earray, loc, npts)
REAL xarray[], yarray[], earray[];
int loc, npts;
{
   REAL angsav,
   	ninety = 90.0,
        zero = 0.0;
   float xx, yy;
   int j;

   if (npts < 1) return(-1);
   angsav = aangle;
   if(loc == 1 || loc == 3) {
       set_angle(&ninety,1);
   } else {
       set_angle(&zero,1);
   }
   for (j=0; j<npts; j++) {
       sm_relocate(xarray[j],yarray[j]);
       switch(loc) {
	case 1:
           xx = xarray[j] + earray[j];
           yy = yarray[j];
	   break;
	 case 2:
           xx = xarray[j];
           yy = yarray[j] + earray[j];
	   break;
	 case 3:
           xx = xarray[j] - earray[j];
           yy = yarray[j];
	   break;
	 case 4:
           xx = xarray[j];
           yy = yarray[j] - earray[j];
	   break;
	 default:
           xx = xarray[j] + earray[j];
           yy = yarray[j];
	   msg_1d("Illegal direction for error bar %d\n",loc);
	   break;
       }
       sm_draw( xx, yy );
       sm_draw_point(xx,yy,2,0);
   }
   set_angle(&angsav,1);
   return(0);
}



