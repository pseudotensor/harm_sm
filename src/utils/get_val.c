#include "copyright.h"
/*
 * Here is a function to get various directories and files needed by SM
 */
#include "options.h"
#include <stdio.h>
#include <ctype.h>
#if defined(__MSDOS__)
#  include <dir.h>
#endif
#include "sm_declare.h"

#define NPATH 5				/* max number of elements on path */
#define SIZE 500

extern char cchar;			/* comment character */
static char *smfile[NPATH + 1] = {NULL}; /* names of .sm files */
extern int sm_verbose;			/* should I be chatty? */

void
set_defaults_file(file,home)
char *file;				/* .sm file */
char *home;				/* home directory */
{
   FILE *fil;
   int i,j,k;
   static char *defaults_file[] = {	/* files to look for defaults */
#if defined(VMS) || defined(__MSDOS__)
      "sm.rc", ".sm", NULL
#else
	".sm", "sm.rc", NULL
#endif
   };
   static char *path[NPATH + 1] = {	/* path to search for .sm files */
      "",
      "~",
      NULL,
   };
   char *ptr;
   static char *s_path = NULL;

   if(file != NULL) {
      defaults_file[0] = file;
      defaults_file[1] = NULL;
   }
   if(*defaults_file[0] == '/') {		/* an absolute specification */
      path[0] = ""; path[1] = NULL;
      return;
   }

   if((ptr = getenv("SMPATH")) == NULL || *ptr == '\0') {
#if defined(DEFAULT_PATH)
      ptr = DEFAULT_PATH;
#endif
   }
   if(ptr == NULL || *ptr == '\0') {
      ptr = ". ~";
   }
/*
 * Split path
 */
   if(s_path != NULL) free(s_path);
   if((s_path = malloc(strlen(ptr) + 1)) == NULL) {
      fprintf(stderr,"Can't allocate storage for s_path\n");
      path[0] = ""; path[1] = NULL;
      return;
   }
   ptr = strcpy(s_path,ptr);
   for(i = 0;i < NPATH;i++) {
      path[i] = ptr;
      if((ptr = strchr(path[i],' ')) == NULL) {
	 break;
      }
      *ptr++ = '\0';
   }
   path[++i] = NULL;
/*
 * Replace ~ by a home directory
 */
   for(i = 0;path[i] != NULL;i++) {
#if defined(VMS)
      if(strcmp(path[i],".") == 0) {
	 path[i] = "";
      } else if(strcmp(path[i],"~") == 0) {
	 if(home == NULL) {
	    path[i] = "sys$login:";
	 }
      }
#else
#  if defined(__MSDOS__)
      if(strcmp(path[i],"~") == 0) {
         if(home == NULL) {
            extern char **_argv;
	    char drive[MAXPATH], dir[MAXDIR];
	    static char s_home[MAXPATH];
	 
	    fnsplit( _argv[0], drive, dir, NULL, NULL );
	    if((path[i] = malloc(strlen(dir)+3)) == NULL) {
	       msg("Can't allocate storage for home directory\n");
	       path[i] = "";
	       continue;
	    }
	    (void)sprintf(s_home,"%s%s",drive,dir);
	    home = s_home;
	 }
	 path[i] = home;
      }
#  else
      if(strcmp(path[i],"~") == 0) {
	 if(home == NULL) {
	    if((home = getenv("HOME")) == NULL) {
	       msg("Can't find home directory\n");
	       path[i] = "";
	       continue;
	    }
	 }
	 if((path[i] = malloc(strlen(home) + 2)) == NULL) {
	    msg("Can't allocate storage for home directory\n");
	    path[i] = "";
	    continue;
	 }
	 (void)sprintf(path[i],"%s/",home);
      }
#  endif
#endif
   }
/*
 * now look to see if we can actually find the file. Look in each directory
 * on the path for files in defaults_file; the search is done for the first
 * defaults_file name in each directory on the path, then the second...
 */
   k = 0;
   for(j = 0;defaults_file[j] != NULL;j++) {
      for(i = 0;path[i] != NULL;i++) {
	 if(strcmp(path[i],".") == 0) {
	    smfile[k] = defaults_file[j];
	 } else {
	    char tmp[SIZE];
	    (void)sprintf(tmp,"%s%s",path[i],defaults_file[j]);
	    if((smfile[k] = malloc(strlen(tmp)+1)) == NULL) {
	       msg_1s("Cannot allocate storage for %s\n",tmp);
	       smfile[k] = NULL;
	    }
	    strcpy(smfile[k],tmp);
	 }
	 if((fil = fopen(smfile[k],"r")) == NULL) {
	    continue;
	 }
	 fclose(fil);
	 if(sm_verbose > 3) {
	    msg_1s(".sm file is %s\n",smfile[k]);
	 }
	 if(++k == NPATH) {
	    break;
	 }
      }
   }

   smfile[k] = NULL;
}

char *
get_val(what)
char what[];			/* desired property */
{
   char in_line[SIZE],*in_ptr,		/* line from file */
	name[SIZE],			/* name of attribute */
	*vptr;				/* pointer to value */
   static char value[SIZE];		/* the desired value */
   FILE *fil;
   int concatenate;			/* concatenate values from .sm files?*/
   int i,j,k;
   static int warned = 0;		/* have we moaned about !found? */

   vptr = value;
   if(smfile[0] == NULL) {
      if(sm_verbose && !warned) {
	 warned = 1;
	 msg("Can't find a .sm file\n");
      }
   }
   for(k = 0;k <= NPATH && smfile[k] != NULL;k++) {
      if((fil = fopen(smfile[k],"r")) == NULL) {
	 if(sm_verbose && !warned) {
	    warned = 1;
	    msg_1s("Can't find file \"%s\"\n",smfile[k]);
	 }
	 continue;
      }

      while((in_ptr = fgets(in_line,SIZE,fil)) != NULL) {
	 while(isspace(*in_ptr)) in_ptr++;
	 if(in_ptr[0] == cchar || in_ptr[0] == '\0') { /* blank or comment */
	    continue;
	 } else if(in_ptr[0] == '@' || in_ptr[0] == '+') {
	    (void)str_scanf(&in_ptr[1],"%s",name);
	 } else {
	    (void)str_scanf(in_ptr,"%s",name);
	 }
	 if(strcmp(name,what) == 0) {
	    if(in_ptr[0] == '@') {	/* this entry is explicitly missing */
	       (void)fclose(fil);
	       return(vptr > value ? value : NULL);
	    } else if(in_ptr[0] == '+') {
	       concatenate = 1;
	    } else {
	       concatenate = 0;
	    }
	 
	    i = strlen(in_ptr) - 1;
	    if(in_ptr[i] == '\n') in_ptr[i] = '\0'; /* remove \n */
	 
	    for(i = 0;i < SIZE && in_ptr[i] != '\0' &&
		!isspace(in_ptr[i]);i++) continue; /* skip name */
	    for(;i < SIZE && isspace(in_ptr[i]);i++) continue; /* find value */
	    for(j = i;in_ptr[j] != '\0';j++) {
	       if(in_ptr[j] == cchar) {
		  j--;
		  if(j > i) {
		     while(isspace(in_ptr[j])) j--;
		  }
		  in_ptr[++j] = '\0';
		  break;
	       } else if(in_ptr[j] == '\\') {
		  if(in_ptr[j+1] == '\\' || in_ptr[j+1] == cchar) {
		     if(in_ptr[j + 2] != '\0') {
			j++;		/* skip escaped characters */
		     }
		  }
	       }
	    }
	    if(in_ptr[i] == '\0' || i == SIZE || i == j) {
	       if(concatenate) {
		  msg("You must specify a value if you use +name in .sm\n");
	       }
	       (void)fclose(fil);
	       return("1");
	    } else {
	       if(vptr > value) *vptr++ = ' ';
	       j = strlen(&in_ptr[i]);
	       if(vptr + j > value + SIZE) {
		  msg_1s("Total value for %s in .sm is too long\n",what);
		  (void)fclose(fil);
		  return(vptr > value ? value : NULL);
	       }
	       strcpy(vptr,&in_ptr[i]);
	       vptr += j;
	       if(!concatenate) {
		  (void)fclose(fil);
		  return(value);
	       }
	    }
	 }
      }
      (void)fclose(fil);
   }

   if(vptr > value) return(value);

   if(isupper(what[0])) {		/* Try the environment */
      return(getenv(what));
   }
   return(NULL);			/* not found */
}

/****************************************************/
/*
 * Add an entry to the defaults_file file
 */
int
put_val(name,value)
char name[],
     value[];
{
   char file[SIZE];
   FILE *fil;

   if(smfile[0] == NULL || (fil = fopen(smfile[0],"r+")) == NULL) {
      if((fil = fopen(smfile[0],"r+")) == NULL) {
#if defined(VMS)
	 (void)sprintf(file,"sys$login:%s",smfile[0]);
#else
#  if defined(__MSDOS__)
	 (void)sprintf(file,"./%s",smfile[0]);
#  else
	 (void)sprintf(file,"%s/%s",getenv("HOME"),smfile[0]);
#  endif
#endif
	 if((fil = fopen(file,"r+")) == NULL) {
	    if((fil = fopen(file,"w")) == NULL) {
	       msg_1s("I'm sorry, but I can't create a %s file for you\n",
		    smfile[0]);
	       return(-1);
	    }
	 }
      }
   }
   fseek(fil,0L,2);
   fprintf(fil,"%s\t\t%s",name,value);
   (void)fclose(fil);
   return(0);
}
