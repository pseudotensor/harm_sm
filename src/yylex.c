#include "copyright.h"
/*
 * yylex takes the output from lexan(), and
 * recognises keywords for yacc.
 *
 * lexan() partitions the input stream into integers
 * (LEXINT), floating point numbers (LEXFLOAT) and
 * words (LEXWORD). A word is any sequence of non-white
 * characters which isn't a float or integer.
 * An end-of-input marker (ENDMARK), carriage return (\n)
 * and the characters +-/(){}[]<>&|!=,?:% are also recognised
 */
#include "options.h"
#include <stdio.h>
#include <ctype.h>
#include "stack.h"
#include "vectors.h"
#include "yaccun.h"
#include "control.h"
#include "lex.h"
#include "sm_declare.h"

extern int noexpand,	/* shall I recognise tokens ? */
           sm_verbose;	/* Shall I be verbose ? */

static char overload_tab[] = {	/* Is token overloaded? */
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', 
};
static char nooverload_tab[] = {	/* no overloaded tokens */
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
   '\0', '\0', 
};

static char *overloaded = overload_tab;

int
#if defined(YYBISON)		/* a bison later than 1.14 */
yylex(yylvalp)
YYSTYPE *yylvalp;
#else
yylex(yylvalp,yyllocp)
YYSTYPE *yylvalp;
Void *yyllocp;			/* not used */
#endif
{
  int ll;

  switch (((ll = lexan(yylvalp)) > 0) ? ll : ENDMARK) {
  case LEXINT :
     if(sm_verbose > 3)
        fprintf(stderr,"%d> INTEGER  %d\n",noexpand,
                                       yylvalp->intval);
     save_int(yylvalp->intval);
     return(INTEGER);
  case LEXFLOAT :
     if(sm_verbose > 3)
        fprintf(stderr,"%d> FLOAT   %g\n",noexpand,
                                      yylvalp->floatval);
     save_float(yylvalp->floatval);
     return(FLOAT);
  case LEXWORD :
     if(noexpand > 0) {	/* recognise no keywords */
        if(noexpand == 1 &&
                  !strcmp(yylvalp->charval,"{")) {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> WORD     '{'\n",noexpand);
           else {;}
           save_str("{");
           return('{');
        } else {
           if(sm_verbose > 3)
              if(!strcmp(yylvalp->charval,"\n"))
              fprintf(stderr,"%d> WORD     \\n\n",noexpand);
              else
              fprintf(stderr,"%d> WORD     %s\n",noexpand,
           		                          yylvalp->charval);
           else {;}
           save_str(yylvalp->charval);
           return(WORD);
        }
     }

     switch (yylvalp->charval[0]) {
     case '\n' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '\\n'     \\n\n",noexpand);
           else {;}
           save_str("\n");
           return('\n');
        }
        break;
     case '*' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '*'      *\n",noexpand);
           else {;}
           save_str("*");
           return('*');
        } else if(!strcmp(yylvalp->charval,"**")) {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '**'    **\n",noexpand);
           else {;}
           save_str("**");
           return(POWER_SYMBOL);
        }
        break;
     case '+' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '+'     +\n",noexpand);
           else {;}
           save_str("+");
           return('+');
        } else
           break;
     case '-' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '-'     -\n",noexpand);
           else {;}
           save_str("-");
           return('-');
        } else
           break;
     case '/' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '/'     /\n",noexpand);
           else {;}
           save_str("/");
           return('/');
        } else
           break;
     case '(' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '('     (\n",noexpand);
           else {;}
           save_str("(");
           return('(');
        } else
           break;
     case ')' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> ')'     )\n",noexpand);
           else {;}
           save_str(")");
           return(')');
        } else
           break;
     case '{' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '{'     {\n",noexpand);
           else {;}
           save_str("{");
           return('{');
        } else
           break;
     case '}' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '}'     }\n",noexpand);
           else {;}
           save_str("}");
           return('}');
        } else
           break;
     case '[' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '['     [\n",noexpand);
           else {;}
           save_str("[");
           return('[');
        } else
           break;
     case ']' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> ']'     ]\n",noexpand);
           else {;}
           save_str("]");
           return(']');
        } else
           break;
     case '<' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '<'     <\n",noexpand);
           else {;}
           save_str("<");
           return('<');
        } else
           break;
     case '>' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '>'     >\n",noexpand);
           else {;}
           save_str(">");
           return('>');
        } else
           break;
     case '&' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '&'     &\n",noexpand);
           else {;}
           save_str("&");
           return('&');
        } else
           break;
     case '|' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '|'     |\n",noexpand);
           else {;}
           save_str("|");
           return('|');
        } else
           break;
     case '!' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '!'     !\n",noexpand);
           else {;}
           save_str("!");
           return('!');
        } else
           break;
     case '=' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '='     =\n",noexpand);
           else {;}
           save_str("=");
           return('=');
        } else
           break;
     case ',' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> ','     ,\n",noexpand);
           else {;}
           save_str(",");
           return(',');
        } else
           break;
     case '?' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '?'     ?\n",noexpand);
           else {;}
           save_str("?");
           return('?');
        } else
           break;
     case ':' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> ':'     :\n",noexpand);
           else {;}
           save_str(":");
           return(':');
        } else
           break;
     case '%' :
        if(yylvalp->charval[1] == '\0') {
           if(sm_verbose > 3)
              fprintf(stderr,"%d> '%%'     %%\n",noexpand);
           else {;}
           save_str("%");
           return('%');
        } else
           break;
     case 'A' :
        if(!strcmp(yylvalp->charval,"ABORT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ABORT    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(262);
       } else
        if(!strcmp(yylvalp->charval,"ABS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ABS      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(344);
       } else
        if(!strcmp(yylvalp->charval,"ACOS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ACOS     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(345);
       } else
        if(!strcmp(yylvalp->charval,"ANGLE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ANGLE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(293);
       } else
        if(!strcmp(yylvalp->charval,"APROPOS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> APROPOS  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(263);
       } else
        if(!strcmp(yylvalp->charval,"ASIN")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ASIN     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(346);
       } else
        if(!strcmp(yylvalp->charval,"ASPECT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ASPECT   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(294);
       } else
        if(!strcmp(yylvalp->charval,"ATAN")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ATAN     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(347);
       } else
        if(!strcmp(yylvalp->charval,"ATAN2")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ATAN2    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(348);
       } else
        if(!strcmp(yylvalp->charval,"ATOF")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ATOF     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(365);
       } else
        if(!strcmp(yylvalp->charval,"AXIS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> AXIS     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(295);
       } else
			break;
     case 'a' :
        if(!overloaded[262] && !strcmp(yylvalp->charval,"abort")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> abort    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(262);
       } else
        if(!overloaded[344] && !strcmp(yylvalp->charval,"abs")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> abs      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(344);
       } else
        if(!overloaded[345] && !strcmp(yylvalp->charval,"acos")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> acos     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(345);
       } else
        if(!overloaded[293] && !strcmp(yylvalp->charval,"angle")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> angle    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(293);
       } else
        if(!overloaded[263] && !strcmp(yylvalp->charval,"apropos")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> apropos  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(263);
       } else
        if(!overloaded[346] && !strcmp(yylvalp->charval,"asin")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> asin     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(346);
       } else
        if(!overloaded[294] && !strcmp(yylvalp->charval,"aspect")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> aspect   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(294);
       } else
        if(!overloaded[347] && !strcmp(yylvalp->charval,"atan")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> atan     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(347);
       } else
        if(!overloaded[348] && !strcmp(yylvalp->charval,"atan2")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> atan2    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(348);
       } else
        if(!overloaded[365] && !strcmp(yylvalp->charval,"atof")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> atof     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(365);
       } else
        if(!overloaded[295] && !strcmp(yylvalp->charval,"axis")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> axis     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(295);
       } else
			break;
     case 'B' :
        if(!strcmp(yylvalp->charval,"BOX")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> BOX      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(296);
       } else
        if(!strcmp(yylvalp->charval,"BREAK")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> BREAK    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(264);
       } else
			break;
     case 'b' :
        if(!overloaded[296] && !strcmp(yylvalp->charval,"box")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> box      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(296);
       } else
        if(!overloaded[264] && !strcmp(yylvalp->charval,"break")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> break    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(264);
       } else
			break;
     case 'C' :
        if(!strcmp(yylvalp->charval,"CHDIR")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> CHDIR    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(265);
       } else
        if(!strcmp(yylvalp->charval,"CONCAT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> CONCAT   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(349);
       } else
        if(!strcmp(yylvalp->charval,"CONNECT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> CONNECT  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(297);
       } else
        if(!strcmp(yylvalp->charval,"CONTOUR")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> CONTOUR  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(298);
       } else
        if(!strcmp(yylvalp->charval,"COS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> COS      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(350);
       } else
        if(!strcmp(yylvalp->charval,"CTYPE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> CTYPE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(299);
       } else
        if(!strcmp(yylvalp->charval,"CURSOR")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> CURSOR   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(300);
       } else
			break;
     case 'c' :
        if(!overloaded[265] && !strcmp(yylvalp->charval,"chdir")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> chdir    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(265);
       } else
        if(!overloaded[349] && !strcmp(yylvalp->charval,"concat")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> concat   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(349);
       } else
        if(!overloaded[297] && !strcmp(yylvalp->charval,"connect")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> connect  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(297);
       } else
        if(!overloaded[298] && !strcmp(yylvalp->charval,"contour")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> contour  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(298);
       } else
        if(!overloaded[350] && !strcmp(yylvalp->charval,"cos")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> cos      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(350);
       } else
        if(!overloaded[299] && !strcmp(yylvalp->charval,"ctype")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ctype    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(299);
       } else
        if(!overloaded[300] && !strcmp(yylvalp->charval,"cursor")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> cursor   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(300);
       } else
			break;
     case 'D' :
        if(!strcmp(yylvalp->charval,"DATA")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DATA     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(301);
       } else
        if(!strcmp(yylvalp->charval,"DEFINE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DEFINE   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(266);
       } else
        if(!strcmp(yylvalp->charval,"DELETE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DELETE   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(267);
       } else
        if(!strcmp(yylvalp->charval,"DEVICE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DEVICE   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(302);
       } else
        if(!strcmp(yylvalp->charval,"DIMEN")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DIMEN    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(351);
       } else
        if(!strcmp(yylvalp->charval,"DITHER")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DITHER   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(305);
       } else
        if(!strcmp(yylvalp->charval,"DO")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DO       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(268);
       } else
        if(!strcmp(yylvalp->charval,"DOT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DOT      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(303);
       } else
        if(!strcmp(yylvalp->charval,"DRAW")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> DRAW     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(304);
       } else
			break;
     case 'd' :
        if(!overloaded[301] && !strcmp(yylvalp->charval,"data")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> data     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(301);
       } else
        if(!overloaded[266] && !strcmp(yylvalp->charval,"define")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> define   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(266);
       } else
        if(!overloaded[267] && !strcmp(yylvalp->charval,"delete")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> delete   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(267);
       } else
        if(!overloaded[302] && !strcmp(yylvalp->charval,"device")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> device   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(302);
       } else
        if(!overloaded[351] && !strcmp(yylvalp->charval,"dimen")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> dimen    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(351);
       } else
        if(!overloaded[305] && !strcmp(yylvalp->charval,"dither")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> dither   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(305);
       } else
        if(!overloaded[268] && !strcmp(yylvalp->charval,"do")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> do       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(268);
       } else
        if(!overloaded[303] && !strcmp(yylvalp->charval,"dot")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> dot      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(303);
       } else
        if(!overloaded[304] && !strcmp(yylvalp->charval,"draw")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> draw     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(304);
       } else
			break;
     case 'E' :
        if(!strcmp(yylvalp->charval,"EDIT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> EDIT     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(269);
       } else
        if(!strcmp(yylvalp->charval,"ELSE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ELSE     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(270);
       } else
        if(!strcmp(yylvalp->charval,"ERASE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ERASE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(306);
       } else
        if(!strcmp(yylvalp->charval,"ERRORBAR")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ERRORBAR %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(307);
       } else
        if(!strcmp(yylvalp->charval,"EXP")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> EXP      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(352);
       } else
        if(!strcmp(yylvalp->charval,"EXPAND")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> EXPAND   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(308);
       } else
			break;
     case 'e' :
        if(!overloaded[269] && !strcmp(yylvalp->charval,"edit")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> edit     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(269);
       } else
        if(!overloaded[270] && !strcmp(yylvalp->charval,"else")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> else     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(270);
       } else
        if(!overloaded[306] && !strcmp(yylvalp->charval,"erase")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> erase    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(306);
       } else
        if(!overloaded[307] && !strcmp(yylvalp->charval,"errorbar")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> errorbar %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(307);
       } else
        if(!overloaded[352] && !strcmp(yylvalp->charval,"exp")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> exp      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(352);
       } else
        if(!overloaded[308] && !strcmp(yylvalp->charval,"expand")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> expand   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(308);
       } else
			break;
     case 'F' :
        if(!strcmp(yylvalp->charval,"FFT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> FFT      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(353);
       } else
        if(!strcmp(yylvalp->charval,"FLOAT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> FLOAT    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(260);
       } else
        if(!strcmp(yylvalp->charval,"FOREACH")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> FOREACH  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(271);
       } else
        if(!strcmp(yylvalp->charval,"FORMAT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> FORMAT   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(309);
       } else
			break;
     case 'f' :
        if(!overloaded[353] && !strcmp(yylvalp->charval,"fft")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> fft      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(353);
       } else
        if(!overloaded[260] && !strcmp(yylvalp->charval,"float")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> float    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(260);
       } else
        if(!overloaded[271] && !strcmp(yylvalp->charval,"foreach")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> foreach  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(271);
       } else
        if(!overloaded[309] && !strcmp(yylvalp->charval,"format")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> format   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(309);
       } else
			break;
     case 'G' :
        if(!strcmp(yylvalp->charval,"GAMMA")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> GAMMA    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(354);
       } else
        if(!strcmp(yylvalp->charval,"GRID")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> GRID     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(310);
       } else
			break;
     case 'g' :
        if(!overloaded[354] && !strcmp(yylvalp->charval,"gamma")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> gamma    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(354);
       } else
        if(!overloaded[310] && !strcmp(yylvalp->charval,"grid")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> grid     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(310);
       } else
			break;
     case 'H' :
        if(!strcmp(yylvalp->charval,"HELP")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> HELP     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(272);
       } else
        if(!strcmp(yylvalp->charval,"HISTOGRAM")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> HISTOGRAM %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(311);
       } else
        if(!strcmp(yylvalp->charval,"HISTORY")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> HISTORY  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(273);
       } else
			break;
     case 'h' :
        if(!overloaded[272] && !strcmp(yylvalp->charval,"help")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> help     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(272);
       } else
        if(!overloaded[311] && !strcmp(yylvalp->charval,"histogram")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> histogram %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(311);
       } else
        if(!overloaded[273] && !strcmp(yylvalp->charval,"history")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> history  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(273);
       } else
			break;
     case 'I' :
        if(!strcmp(yylvalp->charval,"IF")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> IF       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(274);
       } else
        if(!strcmp(yylvalp->charval,"IMAGE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> IMAGE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(312);
       } else
        if(!strcmp(yylvalp->charval,"INDEX")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> INDEX    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(366);
       } else
        if(!strcmp(yylvalp->charval,"INT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> INT      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(355);
       } else
        if(!strcmp(yylvalp->charval,"INTEGER")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> INTEGER  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(261);
       } else
			break;
     case 'i' :
        if(!overloaded[274] && !strcmp(yylvalp->charval,"if")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> if       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(274);
       } else
        if(!overloaded[312] && !strcmp(yylvalp->charval,"image")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> image    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(312);
       } else
        if(!overloaded[366] && !strcmp(yylvalp->charval,"index")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> index    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(366);
       } else
        if(!overloaded[355] && !strcmp(yylvalp->charval,"int")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> int      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(355);
       } else
        if(!overloaded[261] && !strcmp(yylvalp->charval,"integer")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> integer  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(261);
       } else
			break;
     case 'J' :
			break;
     case 'j' :
			break;
     case 'K' :
        if(!strcmp(yylvalp->charval,"KEY")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> KEY      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(275);
       } else
			break;
     case 'k' :
        if(!overloaded[275] && !strcmp(yylvalp->charval,"key")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> key      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(275);
       } else
			break;
     case 'L' :
        if(!strcmp(yylvalp->charval,"LABEL")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LABEL    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(313);
       } else
        if(!strcmp(yylvalp->charval,"LENGTH")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LENGTH   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(367);
       } else
        if(!strcmp(yylvalp->charval,"LEVELS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LEVELS   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(314);
       } else
        if(!strcmp(yylvalp->charval,"LG")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LG       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(356);
       } else
        if(!strcmp(yylvalp->charval,"LIMITS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LIMITS   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(315);
       } else
        if(!strcmp(yylvalp->charval,"LINES")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LINES    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(316);
       } else
        if(!strcmp(yylvalp->charval,"LIST")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LIST     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(276);
       } else
        if(!strcmp(yylvalp->charval,"LN")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LN       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(357);
       } else
        if(!strcmp(yylvalp->charval,"LOCAL")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LOCAL    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(277);
       } else
        if(!strcmp(yylvalp->charval,"LOCATION")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LOCATION %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(317);
       } else
        if(!strcmp(yylvalp->charval,"LTYPE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LTYPE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(318);
       } else
        if(!strcmp(yylvalp->charval,"LWEIGHT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> LWEIGHT  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(319);
       } else
			break;
     case 'l' :
        if(!overloaded[313] && !strcmp(yylvalp->charval,"label")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> label    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(313);
       } else
        if(!overloaded[367] && !strcmp(yylvalp->charval,"length")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> length   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(367);
       } else
        if(!overloaded[314] && !strcmp(yylvalp->charval,"levels")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> levels   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(314);
       } else
        if(!overloaded[356] && !strcmp(yylvalp->charval,"lg")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> lg       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(356);
       } else
        if(!overloaded[315] && !strcmp(yylvalp->charval,"limits")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> limits   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(315);
       } else
        if(!overloaded[316] && !strcmp(yylvalp->charval,"lines")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> lines    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(316);
       } else
        if(!overloaded[276] && !strcmp(yylvalp->charval,"list")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> list     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(276);
       } else
        if(!overloaded[357] && !strcmp(yylvalp->charval,"ln")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ln       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(357);
       } else
        if(!overloaded[277] && !strcmp(yylvalp->charval,"local")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> local    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(277);
       } else
        if(!overloaded[317] && !strcmp(yylvalp->charval,"location")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> location %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(317);
       } else
        if(!overloaded[318] && !strcmp(yylvalp->charval,"ltype")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ltype    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(318);
       } else
        if(!overloaded[319] && !strcmp(yylvalp->charval,"lweight")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> lweight  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(319);
       } else
			break;
     case 'M' :
        if(!strcmp(yylvalp->charval,"MACRO")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> MACRO    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(278);
       } else
        if(!strcmp(yylvalp->charval,"META")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> META     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(341);
       } else
        if(!strcmp(yylvalp->charval,"MINMAX")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> MINMAX   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(320);
       } else
			break;
     case 'm' :
        if(!overloaded[278] && !strcmp(yylvalp->charval,"macro")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> macro    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(278);
       } else
        if(!overloaded[341] && !strcmp(yylvalp->charval,"meta")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> meta     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(341);
       } else
        if(!overloaded[320] && !strcmp(yylvalp->charval,"minmax")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> minmax   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(320);
       } else
			break;
     case 'N' :
        if(!strcmp(yylvalp->charval,"NOTATION")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> NOTATION %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(321);
       } else
			break;
     case 'n' :
        if(!overloaded[321] && !strcmp(yylvalp->charval,"notation")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> notation %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(321);
       } else
			break;
     case 'O' :
        if(!strcmp(yylvalp->charval,"OLD")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> OLD      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(342);
       } else
        if(!strcmp(yylvalp->charval,"OVERLOAD")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> OVERLOAD %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(279);
       } else
			break;
     case 'o' :
        if(!overloaded[342] && !strcmp(yylvalp->charval,"old")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> old      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(342);
       } else
        if(!overloaded[279] && !strcmp(yylvalp->charval,"overload")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> overload %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(279);
       } else
			break;
     case 'P' :
        if(!strcmp(yylvalp->charval,"PAGE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> PAGE     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(322);
       } else
        if(!strcmp(yylvalp->charval,"PI")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> PI       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(358);
       } else
        if(!strcmp(yylvalp->charval,"POINTS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> POINTS   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(323);
       } else
        if(!strcmp(yylvalp->charval,"PRINT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> PRINT    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(280);
       } else
        if(!strcmp(yylvalp->charval,"PROMPT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> PROMPT   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(281);
       } else
        if(!strcmp(yylvalp->charval,"PTYPE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> PTYPE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(324);
       } else
        if(!strcmp(yylvalp->charval,"PUTLABEL")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> PUTLABEL %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(325);
       } else
			break;
     case 'p' :
        if(!overloaded[322] && !strcmp(yylvalp->charval,"page")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> page     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(322);
       } else
        if(!overloaded[358] && !strcmp(yylvalp->charval,"pi")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> pi       %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(358);
       } else
        if(!overloaded[323] && !strcmp(yylvalp->charval,"points")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> points   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(323);
       } else
        if(!overloaded[280] && !strcmp(yylvalp->charval,"print")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> print    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(280);
       } else
        if(!overloaded[281] && !strcmp(yylvalp->charval,"prompt")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> prompt   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(281);
       } else
        if(!overloaded[324] && !strcmp(yylvalp->charval,"ptype")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ptype    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(324);
       } else
        if(!overloaded[325] && !strcmp(yylvalp->charval,"putlabel")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> putlabel %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(325);
       } else
			break;
     case 'Q' :
        if(!strcmp(yylvalp->charval,"QUIT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> QUIT     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(282);
       } else
			break;
     case 'q' :
        if(!overloaded[282] && !strcmp(yylvalp->charval,"quit")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> quit     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(282);
       } else
			break;
     case 'R' :
        if(!strcmp(yylvalp->charval,"RANDOM")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> RANDOM   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(359);
       } else
        if(!strcmp(yylvalp->charval,"RANGE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> RANGE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(326);
       } else
        if(!strcmp(yylvalp->charval,"READ")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> READ     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(283);
       } else
        if(!strcmp(yylvalp->charval,"RELOCATE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> RELOCATE %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(327);
       } else
        if(!strcmp(yylvalp->charval,"RESTORE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> RESTORE  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(284);
       } else
        if(!strcmp(yylvalp->charval,"RETURN")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> RETURN   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(328);
       } else
        if(!strcmp(yylvalp->charval,"ROW")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ROW      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(343);
       } else
			break;
     case 'r' :
        if(!overloaded[359] && !strcmp(yylvalp->charval,"random")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> random   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(359);
       } else
        if(!overloaded[326] && !strcmp(yylvalp->charval,"range")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> range    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(326);
       } else
        if(!overloaded[283] && !strcmp(yylvalp->charval,"read")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> read     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(283);
       } else
        if(!overloaded[327] && !strcmp(yylvalp->charval,"relocate")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> relocate %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(327);
       } else
        if(!overloaded[284] && !strcmp(yylvalp->charval,"restore")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> restore  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(284);
       } else
        if(!overloaded[328] && !strcmp(yylvalp->charval,"return")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> return   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(328);
       } else
        if(!overloaded[343] && !strcmp(yylvalp->charval,"row")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> row      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(343);
       } else
			break;
     case 'S' :
        if(!strcmp(yylvalp->charval,"SAVE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SAVE     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(285);
       } else
        if(!strcmp(yylvalp->charval,"SET")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SET      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(286);
       } else
        if(!strcmp(yylvalp->charval,"SHADE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SHADE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(329);
       } else
        if(!strcmp(yylvalp->charval,"SIN")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SIN      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(360);
       } else
        if(!strcmp(yylvalp->charval,"SNARK")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SNARK    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(287);
       } else
        if(!strcmp(yylvalp->charval,"SORT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SORT     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(330);
       } else
        if(!strcmp(yylvalp->charval,"SPLINE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SPLINE   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(331);
       } else
        if(!strcmp(yylvalp->charval,"SPRINTF")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SPRINTF  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(368);
       } else
        if(!strcmp(yylvalp->charval,"SQRT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SQRT     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(361);
       } else
        if(!strcmp(yylvalp->charval,"STANDARD")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> STANDARD %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(288);
       } else
        if(!strcmp(yylvalp->charval,"STRING")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> STRING   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(259);
       } else
        if(!strcmp(yylvalp->charval,"STRLEN")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> STRLEN   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(369);
       } else
        if(!strcmp(yylvalp->charval,"SUBSTR")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SUBSTR   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(370);
       } else
        if(!strcmp(yylvalp->charval,"SUM")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SUM      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(362);
       } else
        if(!strcmp(yylvalp->charval,"SURFACE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> SURFACE  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(332);
       } else
			break;
     case 's' :
        if(!overloaded[285] && !strcmp(yylvalp->charval,"save")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> save     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(285);
       } else
        if(!overloaded[286] && !strcmp(yylvalp->charval,"set")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> set      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(286);
       } else
        if(!overloaded[329] && !strcmp(yylvalp->charval,"shade")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> shade    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(329);
       } else
        if(!overloaded[360] && !strcmp(yylvalp->charval,"sin")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> sin      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(360);
       } else
        if(!overloaded[287] && !strcmp(yylvalp->charval,"snark")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> snark    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(287);
       } else
        if(!overloaded[330] && !strcmp(yylvalp->charval,"sort")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> sort     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(330);
       } else
        if(!overloaded[331] && !strcmp(yylvalp->charval,"spline")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> spline   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(331);
       } else
        if(!overloaded[368] && !strcmp(yylvalp->charval,"sprintf")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> sprintf  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(368);
       } else
        if(!overloaded[361] && !strcmp(yylvalp->charval,"sqrt")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> sqrt     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(361);
       } else
        if(!overloaded[288] && !strcmp(yylvalp->charval,"standard")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> standard %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(288);
       } else
        if(!overloaded[259] && !strcmp(yylvalp->charval,"string")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> string   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(259);
       } else
        if(!overloaded[369] && !strcmp(yylvalp->charval,"strlen")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> strlen   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(369);
       } else
        if(!overloaded[370] && !strcmp(yylvalp->charval,"substr")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> substr   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(370);
       } else
        if(!overloaded[362] && !strcmp(yylvalp->charval,"sum")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> sum      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(362);
       } else
        if(!overloaded[332] && !strcmp(yylvalp->charval,"surface")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> surface  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(332);
       } else
			break;
     case 'T' :
        if(!strcmp(yylvalp->charval,"TABLE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> TABLE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(333);
       } else
        if(!strcmp(yylvalp->charval,"TAN")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> TAN      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(363);
       } else
        if(!strcmp(yylvalp->charval,"TERMTYPE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> TERMTYPE %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(289);
       } else
        if(!strcmp(yylvalp->charval,"TICKSIZE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> TICKSIZE %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(334);
       } else
			break;
     case 't' :
        if(!overloaded[333] && !strcmp(yylvalp->charval,"table")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> table    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(333);
       } else
        if(!overloaded[363] && !strcmp(yylvalp->charval,"tan")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> tan      %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(363);
       } else
        if(!overloaded[289] && !strcmp(yylvalp->charval,"termtype")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> termtype %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(289);
       } else
        if(!overloaded[334] && !strcmp(yylvalp->charval,"ticksize")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ticksize %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(334);
       } else
			break;
     case 'U' :
        if(!strcmp(yylvalp->charval,"UNARY")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> UNARY    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(371);
       } else
        if(!strcmp(yylvalp->charval,"USER")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> USER     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(335);
       } else
			break;
     case 'u' :
        if(!overloaded[371] && !strcmp(yylvalp->charval,"unary")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> unary    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(371);
       } else
        if(!overloaded[335] && !strcmp(yylvalp->charval,"user")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> user     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(335);
       } else
			break;
     case 'V' :
        if(!strcmp(yylvalp->charval,"VERBOSE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> VERBOSE  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(290);
       } else
        if(!strcmp(yylvalp->charval,"VIEWPOINT")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> VIEWPOINT %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(336);
       } else
			break;
     case 'v' :
        if(!overloaded[290] && !strcmp(yylvalp->charval,"verbose")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> verbose  %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(290);
       } else
        if(!overloaded[336] && !strcmp(yylvalp->charval,"viewpoint")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> viewpoint %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(336);
       } else
			break;
     case 'W' :
        if(!strcmp(yylvalp->charval,"WHATIS")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> WHATIS   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(337);
       } else
        if(!strcmp(yylvalp->charval,"WHILE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> WHILE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(291);
       } else
        if(!strcmp(yylvalp->charval,"WINDOW")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> WINDOW   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(338);
       } else
        if(!strcmp(yylvalp->charval,"WORD")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> WORD     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(258);
       } else
        if(!strcmp(yylvalp->charval,"WRITE")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> WRITE    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(292);
       } else
			break;
     case 'w' :
        if(!overloaded[337] && !strcmp(yylvalp->charval,"whatis")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> whatis   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(337);
       } else
        if(!overloaded[291] && !strcmp(yylvalp->charval,"while")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> while    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(291);
       } else
        if(!overloaded[338] && !strcmp(yylvalp->charval,"window")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> window   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(338);
       } else
        if(!overloaded[258] && !strcmp(yylvalp->charval,"word")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> word     %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(258);
       } else
        if(!overloaded[292] && !strcmp(yylvalp->charval,"write")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> write    %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(292);
       } else
			break;
     case 'X' :
        if(!strcmp(yylvalp->charval,"XLABEL")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> XLABEL   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(339);
       } else
			break;
     case 'x' :
        if(!overloaded[339] && !strcmp(yylvalp->charval,"xlabel")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> xlabel   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(339);
       } else
			break;
     case 'Y' :
        if(!strcmp(yylvalp->charval,"YLABEL")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> YLABEL   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(340);
       } else
			break;
     case 'y' :
        if(!overloaded[340] && !strcmp(yylvalp->charval,"ylabel")) {
           if(sm_verbose > 3) {
              fprintf(stderr,"%d> ylabel   %s\n",
                 		noexpand,yylvalp->charval);
           }
           save_str(yylvalp->charval);
           return(340);
       } else
			break;
     default : break;
     }
     save_str(yylvalp->charval);

     if(sm_verbose > 3) {
        fprintf(stderr,"%d> WORD     %s\n",noexpand,
                                       yylvalp->charval);
     }
     return(WORD);

  case LEXSTRING:
     save_str(yylvalp->charval);

     if(sm_verbose > 3) {
        fprintf(stderr,"%d> STRING     %s\n",noexpand,
                                       yylvalp->charval);
     }
     if(noexpand) {
        return(WORD);
     } else {
        return(STRING);
     }

  case ENDMARK :
     if(sm_verbose > 3)
        fprintf(stderr,"%d> ENDMARK\n",noexpand);
     else {;}
     save_str("ENDMARK");
     return(ENDMARK);
  default :
     printf("bad code from lexan in yylex\n");
     (void)strcpy(yylvalp->charval,"unknown_code");
     save_str(yylvalp->charval);
     return(WORD);
  }
}


void
allow_overload()
{
   overloaded = overload_tab;
}


void
disable_overload()
{
   overloaded = nooverload_tab;
}


void
overload(str,set)
char *str;		/* symbol to overload */
int set;		/* true to overload it */
{
   char *ptr;
   int tok;
   YYSTYPE lval;

   for(ptr = str;islower(*ptr);ptr++) *ptr=toupper(*ptr);

   push(str,S_NORMAL);
   tok = yylex(&lval,(Void *)NULL);
   unsave_str();			/* yylex will have saved it */

   if(tok == WORD) {
      msg_1s("You can't overload %s -- it isn't special\n",str);
      return;
   }
   if(tok < 0 || tok > 371) {
      msg_1d("Illegal token to overload: %d\n",tok);
      return;
   }

   overloaded[tok] = (set) ? '1' : '\0';
}
