#include "copyright.h"
/*
 * This does logical operations on (pointers to) vectors.
 *
 * vec_and(v1,v2,ans)	ans = v1 && v2
 * vec_or(v1,v2,ans)	ans = v1 || v2
 * vec_eq(v1,v2,ans)	ans = v1 == v2
 * vec_ne(v1,v2,ans)	ans = v1 != v2
 * vec_ge(v1,v2,ans)	ans = v1 >= v2
 * vec_gt(v1,v2,ans)	ans = v1 >  v2
 * vec_ternary(v1,v2,v3,ans)	ans = v1 ? v2 : v3
 */
#include <stdio.h>
#include "options.h"
#include "vectors.h"
#include "sm_declare.h"

extern int sm_verbose;			/* be chatty? */
  
/***********************************************************/

void
vec_and(v1,v2,ans)
VECTOR *v1,*v2,			/* vectors to be anded */
       *ans;			/* the answer */
{
   int i;
   VECTOR *temp;		/* used in switching v1 and v2 */

   if(check_vec(v1,v2,ans) < 0) return;

   if(v1->dimen < v2->dimen) {	/* make v1 >= v2 */
      temp = v1;
      v1 = v2;
      v2 = temp;
   }

   if(v2->dimen == 1) {		/* scalar and vector */
      int dimen = v1->dimen;		/* unalias */
      REAL *f = v1->vec.f;
      REAL val = v2->vec.f[0];
      if(!val) {
	 for(i = 0;i < dimen;i++) {
	    f[i] = 0;
	 }
      } else {
	 for(i = 0;i < dimen;i++) {
	    f[i] = f[i] && 1;
	 }
      }
   } else {			/* both vectors */
      int dimen = v2->dimen;		/* unalias */
      REAL *f1 = v1->vec.f, *f2 = v2->vec.f;
      
      if(v1->dimen != v2->dimen) {
	 msg_2s("%s && %s: vectors' dimensions differ",v1->name,v2->name);
	 yyerror("show where");
      }
      for(i = 0;i < dimen;i++) {
	 f1[i] = f1[i] && f2[i];
      }
   }
   vec_free(v2);
   ans->dimen = v1->dimen;
   ans->type = VEC_FLOAT;
   ans->name = v1->name;
   ans->vec.f = v1->vec.f;
   return;
}

/***********************************************************/

void
vec_or(v1,v2,ans)
VECTOR *v1,*v2,			/* vectors to be ored */
       *ans;			/* the answer */
{
   int i;
   VECTOR *temp;		/* used in switching v1 and v2 */

   if(check_vec(v1,v2,ans) < 0) return;

   if(v1->dimen < v2->dimen) {	/* make v1 >= v2 */
      temp = v1;
      v1 = v2;
      v2 = temp;
   }

   if(v2->dimen == 1) {		/* scalar and vector */
      int dimen = v1->dimen;		/* unalias */
      REAL *f = v1->vec.f;
      REAL val = v2->vec.f[0];
      if(val) {
	 for(i = 0;i < dimen;i++) {
	    f[i] = 1;
	 }
      } else {
	 for(i = 0;i < dimen;i++) {
	    f[i] = f[i] || 0;
	 }
      }
   } else {			/* both vectors */
      int dimen = v2->dimen;		/* unalias */
      REAL *f1 = v1->vec.f, *f2 = v2->vec.f;

      if(v1->dimen != v2->dimen) {
	 msg_2s("%s || %s: vectors' dimensions differ",v1->name,v2->name);
	 yyerror("show where");
      }
      for(i = 0;i < dimen;i++) {
	 f1[i] = f1[i] || f2[i];
      }
   }
   vec_free(v2);
   ans->dimen = v1->dimen;
   ans->type = VEC_FLOAT;
   ans->name = v1->name;
   ans->vec.f = v1->vec.f;
   return;
}

/***********************************************************/

void
vec_eq(v1,v2,ans)
VECTOR *v1,*v2,			/* vectors to be compared in equality */
       *ans;			/* the answer */
{
   int i,
       made_ans_vec;		/* have I made ans->vec.f yet? */
   VECTOR *temp;		/* used in switching v1 and v2 */

   if(v1->dimen < v2->dimen) {	/* make v1 >= v2 */
      temp = v1;
      v1 = v2;
      v2 = temp;
   }

   ans->dimen = v1->dimen;
   ans->type = VEC_FLOAT;
   ans->name = v1->name;
   if(v1->type != VEC_FLOAT) {
      if(vec_malloc(ans,v1->dimen) < 0) {
	 msg_2s("Can't make logical vector comparing %s and %s",
		v1->name,v2->name);
	 vec_free(v2);
	 ans->vec.f = v1->vec.f;	/* best chance */
	 return;
      }
      made_ans_vec = 1;
   } else {
      made_ans_vec = 0;
      ans->vec = v1->vec;
   }

   if(v2->dimen == 1) {		/* scalar and vector */
      if(v1->type != v2->type) {	/* different types can't be equal */
	 for(i = 0;i < v1->dimen;i++) {
	    ans->vec.f[i] = 0.0;
	 }
      } else if(v1->type == VEC_FLOAT) {
	 int dimen = v1->dimen;		/* unalias */
	 REAL *f = v1->vec.f;
	 REAL val = v2->vec.f[0];
	 for(i = 0;i < dimen;i++) {
	    f[i] = f[i] == val;
	 }
      } else {
	 int dimen = v1->dimen;		/* unalias */
	 REAL *f = ans->vec.f;
	 char **s = v1->vec.s.s;
	 char *val = v2->vec.s.s[0];
	 for(i = 0;i < dimen;i++) {
	    f[i] = strcmp(s[i],val) == 0;
	 }
      }
   } else {			/* both vectors */
      if(v1->dimen != v2->dimen) {
	 msg_2s("%s == %s: vectors' dimensions differ",v1->name,v2->name);
	 yyerror("show where");
      }
      if(v1->type != v2->type) {	/* different types can't be equal */
	 for(i = 0;i < v1->dimen;i++) {
	    ans->vec.f[i] = 0.0;
	 }
      } else if(v1->type == VEC_FLOAT) {
	 int dimen = v2->dimen;		/* unalias */
	 REAL *f1 = v1->vec.f, *f2 = v2->vec.f;
	 for(i = 0;i < dimen;i++) {
	    f1[i] = f1[i] == f2[i];
	 }
      } else {
	 int dimen = v2->dimen;		/* unalias */
	 REAL *f = ans->vec.f;
	 char **s1 = v1->vec.s.s, **s2 = v2->vec.s.s;
	 for(i = 0;i < dimen;i++) {
	    f[i] = strcmp(s1[i],s2[i]) == 0;
	 }
      }
   }
   if(made_ans_vec) {
      vec_free(v1);
   }
   vec_free(v2);
   return;
}

/***********************************************************/

void
vec_ne(v1,v2,ans)
VECTOR *v1,*v2,			/* vectors to be compared in inequality */
       *ans;			/* the answer */
{
   int i,
       made_ans_vec;		/* have I made ans->vec.f yet? */
   VECTOR *temp;		/* used in switching v1 and v2 */

   if(v1->dimen < v2->dimen) {	/* make v1 >= v2 */
      temp = v1;
      v1 = v2;
      v2 = temp;
   }

   ans->dimen = v1->dimen;
   ans->type = VEC_FLOAT;
   ans->name = v1->name;
   if(v1->type != VEC_FLOAT) {
      if(vec_malloc(ans,v1->dimen) < 0) {
	 msg_2s("Can't make logical vector comparing %s and %s",
		v1->name,v2->name);
	 vec_free(v2);
	 ans->vec.f = v1->vec.f;	/* best chance */
	 return;
      }
      made_ans_vec = 1;
   } else {
      made_ans_vec = 0;
      ans->vec = v1->vec;
   }

   if(v2->dimen == 1) {		/* scalar and vector */
      if(v1->type != v2->type) {	/* different types can't be equal */
	 for(i = 0;i < v1->dimen;i++) {
	    ans->vec.f[i] = 1.0;
	 }
      } else if(v1->type == VEC_FLOAT) {
	 int dimen = v1->dimen;		/* unalias */
	 REAL *f = v1->vec.f;
	 REAL val = v2->vec.f[0];
	 for(i = 0;i < dimen;i++) {
	    f[i] = f[i] != val;
	 }
      } else {
	 int dimen = v1->dimen;		/* unalias */
	 REAL *f = ans->vec.f;
	 char **s = v1->vec.s.s;
	 char *val = v2->vec.s.s[0];
	 for(i = 0;i < dimen;i++) {
	    f[i] = strcmp(s[i],val) != 0;
	 }
      }
   } else {			/* both vectors */
      if(v1->dimen != v2->dimen) {
	 msg_2s("%s != %s: vectors' dimensions differ",v1->name,v2->name);
	 yyerror("show where");
      }
      if(v1->type != v2->type) {	/* different types can't be equal */
	 for(i = 0;i < v2->dimen;i++) {
	    ans->vec.f[i] = 1.0;
	 }
      } else if(v1->type == VEC_FLOAT) {
	 int dimen = v2->dimen;		/* unalias */
	 REAL *f1 = v1->vec.f, *f2 = v2->vec.f;
	 for(i = 0;i < dimen;i++) {
	    f1[i] = f1[i] != f2[i];
	 }
      } else {
	 int dimen = v2->dimen;		/* unalias */
	 REAL *f = ans->vec.f;
	 char **s1 = v1->vec.s.s, **s2 = v2->vec.s.s;
	 for(i = 0;i < dimen;i++) {
	    f[i] = strcmp(s1[i],s2[i]) != 0;
	 }
      }
   }
   if(made_ans_vec) {
      vec_free(v1);
   }
   vec_free(v2);
   return;
}

/***********************************************************/

void
vec_ge(v1,v2,ans)
VECTOR *v1,*v2,			/* vectors to be compared */
       *ans;			/* the answer */
{
   int i,
       exchange = 0;		/* have vectors been exchanged ? */
   VECTOR *temp;		/* used in exchanging v1 and v2 */

   if(check_vec(v1,v2,ans) < 0) return;

   if(v1->dimen < v2->dimen) {	/* make v1 >= v2 */
      temp = v1;
      v1 = v2;
      v2 = temp;
      exchange = 1;
   }

   if(v2->dimen == 1) {	/* scalar and vector */
      int dimen = v1->dimen;		/* unalias */
      REAL *f = v1->vec.f;
      REAL val = v2->vec.f[0];
      if(exchange) {
	 for(i = 0;i < dimen;i++) {
	    f[i] = val >= f[i];
	 }
      } else {
	 for(i = 0;i < dimen;i++) {
	    f[i] = f[i] >= val;
	 }
      }
   } else {			/* both vectors */
      int dimen = v2->dimen;		/* unalias */
      REAL *f1 = v1->vec.f, *f2 = v2->vec.f;
      if(v1->dimen != v2->dimen) {
	 msg_2s("%s >= or <= %s: vectors' dimensions differ",v1->name,v2->name);
	 yyerror("show where");
      }
      if(exchange) {
	 for(i = 0;i < dimen;i++) {
	    f1[i] = f2[i] >= f1[i];
	 }
      } else {
	 for(i = 0;i < dimen;i++) {
	    f1[i] = f1[i] >= f2[i];
	 }
      }
   }
   vec_free(v2);
   ans->dimen = v1->dimen;
   ans->type = VEC_FLOAT;
   ans->name = v1->name;
   ans->vec.f = v1->vec.f;
   return;
}

/***********************************************************/

void
vec_gt(v1,v2,ans)
VECTOR *v1,*v2,			/* vectors to be compared */
       *ans;			/* the answer */
{
   int i,
       exchange = 0;		/* have vectors been exchanged ? */
   VECTOR *temp;			/* used in exchanging v1 and v2 */

   if(check_vec(v1,v2,ans) < 0) return;

   if(v1->dimen < v2->dimen) {	/* make v1 >= v2 */
      temp = v1;
      v1 = v2;
      v2 = temp;
      exchange = 1;
   }

   if(v2->dimen == 1) {	/* scalar and vector */
      int dimen = v1->dimen;		/* unalias */
      REAL *f = v1->vec.f;
      REAL val = v2->vec.f[0];
      if(exchange) {
	 for(i = 0;i < dimen;i++) {
	    f[i] = val > f[i];
	 }
      } else {
	 for(i = 0;i < dimen;i++) {
	    f[i] = f[i] > val;
	 }
      }
   } else {			/* both vectors */
      int dimen = v2->dimen;		/* unalias */
      REAL *f1 = v1->vec.f, *f2 = v2->vec.f;

      if(v1->dimen != v2->dimen) {
	 msg_2s("%s > or < %s: vectors' dimensions differ",v1->name,v2->name);
	 yyerror("show where");
      }
      if(exchange) {
	 for(i = 0;i < dimen;i++) {
	    f1[i] = f2[i] > f1[i];
	 }
      } else {
	 for(i = 0;i < dimen;i++) {
	    f1[i] = f1[i] > f2[i];
	 }
      }
   }
   vec_free(v2);
   ans->dimen = v1->dimen;
   ans->type = VEC_FLOAT;
   ans->name = v1->name;
   ans->vec.f = v1->vec.f;
   return;
}

/***********************************************************/

void
vec_ternary(log,v1,v2,ans)
VECTOR *log,			/* logical vector */
       *v1,*v2,			/* vectors to be compared */
       *ans;			/* the answer */
{
   int i,
       exchange = 0;		/* have vectors been exchanged ? */
   VECTOR *temp;		/* used in exchanging v1 and v2 */

   if(v1->type != v2->type || log->type != VEC_FLOAT) {
      if(v1->type != v2->type) {
	 msg_2s("Vectors %s and %s have different types in ?: statement",
	     v1->name,v2->name);
      } else {
	 msg_1s("Logical vector %s is not arithmetic",log->name);
      }
      yyerror("show where");
      vec_free(log);
      vec_free(v1);
      vec_free(v2);
      (void)vec_value(ans,0.0);
      return;
   }

   if(v1->dimen < v2->dimen) {	/* make v1 >= v2 */
      temp = v1;
      v1 = v2;
      v2 = temp;
      exchange = 1;
   }

   if(v1->dimen != v2->dimen && v2->dimen != 1) {
      msg_1s("%s ? ",log->name);
      msg_2s("%s : %s: vectors' dimensions differ",v1->name,v2->name);
      yyerror("show where");
      v1->dimen = v2->dimen;
   }
/*
 * Prepare answer vector
 */
   ans->type = v1->type;
   ans->name = v1->name;

   if(log->dimen == 1) {	/* This is a trivial case */
      if((log->vec.f[0] && exchange) || (!log->vec.f[0] && !exchange)) {
	 if(v2->dimen == 1) {
	    if(ans->type == VEC_FLOAT) {
	       int dimen = v1->dimen;		/* unalias */
	       REAL *f = v1->vec.f;
	       REAL val = v2->vec.f[0];
	       for(i = 0;i < dimen;i++) {
		  f[i] = val;
	       }
	    } else {
	       int dimen = v1->dimen;		/* unalias */
	       char **s = v1->vec.s.s;
	       char *val = v2->vec.s.s[0];
	       for(i = 0;i < dimen;i++) {
		  (void)strcpy(s[i],val);
	       }
	    }
	 } else {
	    temp = v1;		/* v1 will be returned as ans */
	    v1 = v2;
	    v2 = temp;		/* must be able to free v2 */
	 }
      }
      vec_free(log);
      vec_free(v2);
      if(ans->type == VEC_FLOAT) {
	 ans->vec.f = v1->vec.f;
      } else {
	 ans->vec.s = v1->vec.s;
      }
      ans->dimen = v1->dimen;
      return;
   }
/*
 * logical is a vector
 */
   if(log->dimen != v1->dimen) {
      if(v1->dimen == 1) {		/* both v1 and v2 are scalars */
	 ans->dimen = log->dimen;
	 if(ans->type == VEC_FLOAT) {
	    int dimen = log->dimen;	/* unalias */
	    REAL *f, val1 = v1->vec.f[0], val2 = v2->vec.f[0];
	    
	    f = ans->vec.f = log->vec.f; /* put answer in log */
	    if(exchange) {
	       for(i = 0;i < dimen;i++) {
		  f[i] = f[i] ? val2 : val1;
	       }
	    } else {
	       for(i = 0;i < dimen;i++) {
		  f[i] = f[i] ? val1 : val2;
	       }
	    }
	 } else {
	    int dimen = log->dimen;	/* unalias */
	    char **a;
	    REAL *f = log->vec.f;
	    char *val1 = v1->vec.s.s[0], *val2 = v2->vec.s.s[0];

	    if(vec_malloc(ans,log->dimen) < 0) { /* make a vector for answer */
	       msg("Can't get storage in ?: statement\n");
	       ans = v1;
	       return;
	    }
	    a = ans->vec.s.s;
	    if(exchange) {
	       for(i = 0;i < dimen;i++) {
		  (void)strcpy(a[i], f[i] ? val2 : val1);
	       }
	    } else {
	       for(i = 0;i < dimen;i++) {
		  (void)strcpy(a[i],f[i] ? val1 : val2);
	       }
	    }
	      
	    vec_free(log);
	 }
	 vec_free(v1);
	 vec_free(v2);
	 return;
      } else {
	 msg("Logical has wrong dimension in ? : statement");
	 yyerror("show where");
	 if(log->dimen > v1->dimen) {
	    log->dimen = v1->dimen;
	 }
      }
   }
/*
 * We now know that we are going to return the answer in v1
 */
   ans->vec = v1->vec;
   ans->dimen = v1->dimen;
   
   if(v1->type == VEC_FLOAT) {
      int dimen = log->dimen;		/* unalias */
      REAL *l = log->vec.f, *f1 = v1->vec.f;
      if(v2->dimen == 1) {
	 REAL val2 = v2->vec.f[0];
	 if(exchange) {
	    for(i = 0;i < dimen;i++) {
	       if(l[i]) {
		  f1[i] = val2;
	       }
	    }
	 } else {
	    for(i = 0;i < dimen;i++) {
	       if(!l[i]) {
		  f1[i] = val2;
	       }
	    }
	 }
      } else {
	 REAL *f2 = v2->vec.f;
	 if(exchange) {
	    for(i = 0;i < dimen;i++) {
	       if(l[i]) {
		  f1[i] = f2[i];
	       }
	    }
	 } else {
	    for(i = 0;i < dimen;i++) {
	       if(!l[i]) {
		  f1[i] = f2[i];
	       }
	    }
	 }
      }
   } else {
      int dimen = log->dimen;		/* unalias */
      REAL *l = log->vec.f;
      char **f1 = v1->vec.s.s;
      
      if(v2->dimen == 1) {
	 char *val = v2->vec.s.s[0];
	 if(exchange) {
	    for(i = 0;i < dimen;i++) {
	       if(l[i]) {
		  (void)strcpy(f1[i],val);
	       }
	    }
	 } else {
	    for(i = 0;i < dimen;i++) {
	       if(!l[i]) {
		  (void)strcpy(f1[i],val);
	       }
	    }
	 }
      } else {
	 char **f2 = v2->vec.s.s;
	 if(exchange) {
	    for(i = 0;i < dimen;i++) {
	       if(l[i]) {
		  (void)strcpy(f1[i],f2[i]);
	       }
	    }
	 } else {
	    for(i = 0;i < dimen;i++) {
	       if(!l[i]) {
		  (void)strcpy(f1[i],f2[i]);
	       }
	    }
	 }
      }
   }
   vec_free(log);
   vec_free(v2);
}
