#include "copyright.h"
#include <stdio.h>
#include "options.h"
#include "sm_declare.h"
/*
 * Given a vector, and (maybe an array of indices), return a sorted list
 * (maybe of indices).
 *
 * The sort is a Shell sort, see e.g. Press et.al. Numerical Recipes.
 *
 * sort_flt()			vector is of floats, sort in place
 * sort_flt_ind()		vector is of floats, with indices
 */
#include <math.h>

extern int sm_interrupt;		/* respond to ^C */

#define LN2I 1.442695022		/* 1/ln(e) */
#define TINY 1e-5

void
sort_flt_inds(vec,ind,dimen)
REAL vec[];			/* array of vectors to sort */
int ind[],			/* index array for sorted vectors */
    dimen;			/* dimension of vector */
{
   int i,j,m,n,				/* counters */
       lognb2,				/* (int)(log_2(dimen)) */
       temp;

   if(dimen <= 0) return;
   for(i = 0;i < dimen;i++) ind[i] = i;

   lognb2 = log((double)dimen)*LN2I + TINY;	/* ~ log_2(dimen) */
   m = dimen;
   for(n = 0;n < lognb2;n++) {
      if(sm_interrupt) {
	 break;
      }
      m /= 2;
      for(j = m;j < dimen;j++) {
         i = j - m;
	 temp = ind[j];
	 while(i >= 0 && vec[temp] < vec[ind[i]]) {
	    ind[i + m] = ind[i];
	    i -= m;
	 }
	 ind[i + m] = temp;
      }
   }
}

/*********************************************************/
/*
 * Do the sort directly 
 */
void
sort_flt(vec,dimen)
REAL vec[];			/* array of vectors to sort */
int dimen;			/* dimension of vector */
{
   REAL temp;
   int i,j,m,n,				/* counters */
       lognb2;				/* (int)(log_2(dimen)) */

   if(dimen <= 0) return;
   lognb2 = log((double)dimen)*LN2I + TINY;	/* ~ log_2(dimen) */
   m = dimen;
   for(n = 0;n < lognb2;n++) {
      if(sm_interrupt) {
	 break;
      }
      m /= 2;
      for(j = m;j < dimen;j++) {
         i = j - m;
	 temp = vec[j];
	 while(i >= 0 && temp < vec[i]) {
	    vec[i + m] = vec[i];
	    i -= m;
	 }
	 vec[i + m] = temp;
      }
   }
}

/*****************************************************************************/
/*
 * Sort a set of string vectors
 */
void
sort_str_inds(vec,ind,dimen)
char *vec[];			/* array of vectors to sort */
int ind[],			/* index array for sorted vectors */
    dimen;			/* dimension of vector */
{
   int i,j,m,n,				/* counters */
       lognb2,				/* (int)(log_2(dimen)) */
       temp;

   if(dimen <= 0) return;
   for(i = 0;i < dimen;i++) ind[i] = i;

   lognb2 = log((double)dimen)*LN2I + TINY;	/* ~ log_2(dimen) */
   m = dimen;
   for(n = 0;n < lognb2;n++) {
      if(sm_interrupt) {
	 break;
      }
      m /= 2;
      for(j = m;j < dimen;j++) {
         i = j - m;
	 temp = ind[j];
	 while(i >= 0 && strcmp(vec[temp],vec[ind[i]]) < 0) {
	    ind[i + m] = ind[i];
	    i -= m;
	 }
	 ind[i + m] = temp;
      }
   }
}
