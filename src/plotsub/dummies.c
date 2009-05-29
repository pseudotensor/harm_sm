#include "copyright.h"
/*
 * Dummies for interactive SM functions (e.g. msg()), required by
 * programmes that call SM non-interactively
 * 
 */
#include <stdio.h>
#include "options.h"
#include "devices.h"
#include "sm.h"
#include "defs.h"
#include "tree.h"
#include "sm_declare.h"

/*
 * These are really declared in main.c, let's hope that this redeclaration
 * won't hurt.
 */
char cchar = '#';			/* comment character */
char *version_string = "SM Version String";
int sm_interrupt = 0,
    sm_verbose = 0;
int sm_image_cursor = 0;

void
hand_C(i)
int i;
{
   ;
}

void
hand_q(i)
int i;
{
   abort();
}

/*
 * Ask user a question
 */
void
prompt_user(s)
char s[];
{
   printf(s);
}

/*
 * Read a string from input stream
 */
char *
mgets(s,n)
char *s;
int n;
{
   return(fgets(s,n,stdin));
}

int
more(line)
char *line;                     	/* line to print */
{
   if(line != NULL) {
      line[strlen(line) - 1] = '\0';
      (void)puts(line);
   }
   return(0);
}

/*
 * Print messages to stdout
 */
void
msg(f)
char *f;
{
   printf(f);
}

void
msg_1s(f,s)
char *f,*s;
{
   printf(f,s);
}

void
msg_2s(f,s1,s2)
char *f,*s1,*s2;
{
   printf(f,s1,s2);
}

void
msg_1d(f,i)
char *f;
int i;
{
   printf(f,i);
}

void
msg_1f(f,x)
char *f;
double x;
{
   printf(f,x);
}

char *i_print_var();

char *
print_var(name)
char *name;
{
   return(i_print_var(name));
}

void
setvar_internal(name)
char *name;
{
   ;
}

int
sm_parser(str)
char *str;
{
   return(-1);
}	
