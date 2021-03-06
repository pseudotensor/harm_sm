/*
 * Deal with the PRINT command
 */
#include "copyright.h"
#include "options.h"
#include <stdio.h>
#include <ctype.h>
#include "vectors.h"
#include "sm_declare.h"

#define FSIZE 40			/* max. size of sub-string */

extern int sm_interrupt;
static void parse_format();

void
print_vec(outfile,mode,fmt,nvec,vectors,dimen)
char *outfile,				/* file to print to */
     *mode,				/* opened in this mode */
     *fmt;				/* format statement to use */
int nvec;				/* number of vectors */
VECTOR **vectors;
int dimen;				/* max. dimension of vectors */
{
   char *ftype,				/* type (d, f, or s) of type */
        **formats,			/* formats */
   	*sformats;			/* space for formats */
   FILE *outfil;
   int *fwidth,				/* widths of formats */
       has_newline = 0,			/* format includes a newline */
       i,j;

   if(outfile[0] == '\0') {
      outfil = stdout;
      (void)more((char *)NULL);
   } else if((outfil = fopen(outfile,mode)) == NULL) {
      msg_1s("Can't open %s, using stdout\n",outfile);
      outfil = stdout;
   }

   if(nvec == 0) {			/* just print the format */
      (void)fprintf(outfil,fmt);
      
      if(outfil != stdout) {
	 (void)fclose(outfil);
      }
      return;
   }

   if((formats = (char **)malloc((nvec + 1)*sizeof(char *))) == NULL) {
      msg("Can't allocate storage for formats\n");
      return;
   }
   if((sformats = malloc((nvec + 1)*(FSIZE + 1))) == NULL) {
      msg("Can't allocate storage for sformats\n");
      free((char *)formats);
      return;
   }
   if((fwidth = (int *)malloc((nvec + 1)*sizeof(int))) == NULL) {
      msg("Can't allocate storage for fwidth\n");
      free((char *)formats);
      free(sformats);
      return;
   }
   if((ftype = malloc((nvec + 1))) == NULL) {
      msg("Can't allocate storage for ftype\n");
      free((char *)formats);
      free(sformats);
      free(fwidth);
      return;
   }

   for(i = 0;i <= nvec;i++) {
      formats[i] = sformats + i*(FSIZE + 1);
   }
   
   parse_format(fmt,fwidth,ftype,formats,vectors,nvec,&has_newline);

   if(!atoi(print_var("print_noheader")) && nvec > 0) {
      if(outfil != stdout) (void)putc('#',outfil);
      for(i = 0;i < nvec;i++) {		/* write header */
	 (void)fprintf(outfil,"%*s",(i == 0 ? fwidth[0] - 1 : fwidth[i]),
		       vectors[i]->name);
	 if(i == nvec - 1) {
	    (void)putc('\n',outfil);
	 }
      }
      if(outfil != stdout) (void)putc('#',outfil);
      (void)putc('\n',outfil);
   }
   
   for(i = 0;!sm_interrupt && i < dimen;i++) {	/* write values */
      for(j = 0;j < nvec;j++) {
	 if(i >= vectors[j]->dimen) {
	    (void)fprintf(outfil,"%*s",fwidth[j]," ");
	 } else {
	    if(vectors[j]->type == VEC_FLOAT) {
	       if(vectors[j]->vec.f[i] >= NO_VALUE) {
		  (void)fprintf(outfil,"%*s",fwidth[j],"*");
	       } else {
		  if(ftype[j] == 'd') {
		     REAL val = vectors[j]->vec.f[i];
		     if(val >= 0) {
			(void)fprintf(outfil,formats[j],(int)(val + 0.5));
		     } else {
			(void)fprintf(outfil,formats[j],-(int)(-val + 0.5));
		     }
		  } else {
		     (void)fprintf(outfil,formats[j],vectors[j]->vec.f[i]);
		  }
	       }
	    } else {
	       (void)fprintf(outfil,formats[j],vectors[j]->vec.s.s[i]);
	    }
	 }
	 if(j == nvec - 1) {
	    if(fmt[0] == '\0' || has_newline) {
	       if(outfil != stdout) {
		  (void)putc('\n',outfil);
	       } else {
		  if(more("\n") < 0) {
		     i = dimen;		/* break outer loop */
		     break;
		  }
	       }
	    }
	 }
      }
   }

   if(outfil == stdout) {
      (void)putchar('\n');
   } else {
      (void)fclose(outfil);
   }

   free((char *)formats);
   free(sformats);
   free((char *)fwidth);
   free(ftype);
}

/********************************************************/
/*
 * Parse a format string into sub-strings
 */
static void
parse_format(fmt,fwidth,ftype,formats,vectors,nvec,has_newline)
char *fmt;
int *fwidth;
char *ftype;
char **formats;
VECTOR **vectors;
int nvec,
    *has_newline;			/* format has a newline at the end */
{
   int have_format,			/* a format was provided */
       i,j,
       nfmt = 0,			/* number of formats detected */
       real_fmt = 0,			/* does this sub-string contain %.?  */
       width;				/* printing width of a field */

   have_format = (*fmt != '\0');	/* a format is specified */
/*
 * Take that format string apart
 */
   for(i = 0;*fmt != '\0';i++) {
      if(i > nvec) {
	 msg_1d("You can only specify %d formats\n",nvec);
	 break;
      }
      real_fmt = 0;
      width = 0;
      for(j = 0;*fmt != '\0' && j < FSIZE;) {
	 if(*fmt != '%') {
	    if(*fmt == '\n' && *(fmt + 1) == '\0') { /* a closing newline */
	       *fmt = '\0';
	       *has_newline = 1;
	    }
	    width++;
	    formats[i][j++] = *fmt++;
	 } else {
	    formats[i][j++] = *fmt++;
	    if(*fmt == '%') {
	       if(j < FSIZE) formats[i][j++] = *fmt++;
	       width++;
	       continue;
	    }
	    real_fmt = 1;		/* yes, it will produce output */

	    if(*fmt == '*') {
	       msg("Sorry, %%* formats are not supported for output\n");
	       fmt++;
	    }
	    if(*fmt == '-') {
	       formats[i][j++] = *fmt++;
	    }
	    if(!isdigit(*fmt)) {	/* no length given, assume 10 */
	       width += 10;
	    } else {
	       width += atoi(fmt);
	    }
	    while(j < FSIZE && isdigit(*fmt)) {
	       formats[i][j++] = *fmt++;
	    }
	    if(j < FSIZE && *fmt == '.') {
	       formats[i][j++] = *fmt++;
	    }
	    while(j < FSIZE && isdigit(*fmt)) {
	       formats[i][j++] = *fmt++;
	    }

	    switch(formats[i][j] = *fmt++) { /* deal with the type character */
	     case 'd': case 'o': case 'x':
	       if(i < nvec) {
		  if(vectors[i]->type == VEC_FLOAT) {
		     ftype[i] = 'd';
		  } else {
		     msg_1s("You cannot use a %%d format for vector %s\n",
			    vectors[i]->name);
		     formats[i][j] = 's';
		     ftype[i] = 's';
		     continue;
		  }
	       }
	       break;
	     case 'e': case 'f': case 'g':
	       if(i < nvec) {
		  if(vectors[i]->type != VEC_FLOAT) {
		     msg_1d("You cannot use a %%%c ",formats[i][j]);
		     msg_1s("format for vector %s\n",vectors[i]->name);
		     formats[i][j] = 's';
		     ftype[i] = 's';
		     continue;
		  }
		  ftype[i] = 'f';
	       }
	       break;
	     case 's':
	       if(i < nvec) {
		  if(vectors[i]->type != VEC_STRING) {
		     msg_1s("You cannot use a %%s format for vector %s\n",
			    vectors[i]->name);
		     formats[i][j] = 'f';
		     ftype[i] = 'f';
		     continue;
		  }
	       }
	       break;
	     case '\0':
	       msg_1s("Incomplete format: %s\n",formats[i]);
	       ftype[i] = 'd';
	       break;
	     default:
	       msg_1s("Unknown format: %s\n",formats[i]);
	       ftype[i] = 'd';
	       break;
	    }
	    j++;
	    break;
	 }
      }
      formats[i][j] = '\0';
      fwidth[i] = width;
   }
   nfmt = i;
/*
 * last format (which doesn't correspond to any vector) may have some
 * stuff that we want to print. Append it to the last real one.
 */
   if(nfmt > 1 && !real_fmt) {
      nfmt--;				/* last one really isn't a format */
      for(j = 0;j < FSIZE && formats[nfmt - 1][j] != '\0';j++) continue;
      for(fmt = formats[nfmt];j < FSIZE && *fmt != '\0';) {
	 formats[nfmt - 1][j++] = *fmt++;
      }
      formats[nfmt - 1][j] = '\0';
      fwidth[nfmt - 1] += fwidth[nfmt];
      if(*(fmt - 1) == '\n') fwidth[nfmt - 1]--;
   }

   if(have_format && !(nfmt == nvec || (nvec == 0 && !real_fmt))) {
      msg_1d("Format string specifies %d output fields, ",nfmt);
      msg_1d("for %d vectors\n",nvec);
   }
/*
 * if there are more vectors than formats, invent suitable formats for them
 */
   for(i = nfmt;i < nvec;i++) {
      fwidth[i] = 12;
      switch (vectors[i]->type) {
       case VEC_FLOAT:
	 (void)sprintf(formats[i]," %%%d.4g",fwidth[i] - 1);
	 break;
       case VEC_STRING:
	 (void)sprintf(formats[i]," %%%ds",fwidth[i] - 1);
	 break;
       default:
	 msg_1s("Vector %s has an ",vectors[i]->name);
	 msg_1d("unknown type: %d\n",vectors[i]->type);
	 (void)sprintf(formats[i]," %%%d.4g",fwidth[i] - 1);
	 break;
      }
   }
}
