#include "copyright.h"
/*
 * This routine administers the vectors for control.y
 *
 * copy_vector()	copies a vector into the pool
 * free_vector()	frees a vector back to the pool
 * get_vector()		get a predefined vector by value
 * get_vector_ptr()	get the pointer to a predefined vector
 * is_vector()		is name a vector?
 * list_vectors()       list all defined vectors
 * make_vector()	initialise a vector
 * make_anon_vector()   return storage for a vector, but don't name it
 * vec_free(v1)		free storage for v1
 * vec_value(v1,v)	ans = sets v1 to have (scalar) value v
 */
#include <stdio.h>
#include <math.h>
#include "options.h"
#include "tree.h"
#include "vectors.h"
#include "sm_declare.h"

extern int sm_verbose;			/* shall I be verbose ? */
static int stop_list,			/* stop listing vectors? */
	   ddimen,ttype;		/* Used by vmake */
static VECTOR *vvector,			/* Used by vmake */
	      zero_vec;			/* the zero vector */
static void write_vector(),
	    vekill(),
	    velist();
static Void *vemake();
static TREE vv = { NULL, vekill, vemake};
/*
 * Need to initialise zero_vec. ANSI allows us to do this (if I change the
 * order in the union) but it's safer to do it with this call.
 */
void
init_vectors()
{
   zero_vec.name = "";
   (void)strcpy(zero_vec.descrip,"(undefined)");
   zero_vec.vec.f = NULL;
   zero_vec.dimen = 0;
   zero_vec.type = VEC_FLOAT;
}

/*************************************************************/
/*
 * Allocate space for vector->vec
 */
int
vec_malloc(vec,dimen)
VECTOR *vec;
int dimen;
{
   int i;
	   
   if(vec->type == VEC_NULL) {
      vec->vec.f = NULL;
   } else if(vec->type == VEC_FLOAT) {
      if(dimen == 0) {			/* some mallocs don't like malloc(0) */
	 vec->vec.f = (REAL *)malloc(1);
      } else {
	 vec->vec.f = (REAL *)malloc((unsigned)dimen*sizeof(REAL));
      }
      if(vec->vec.f == NULL) {
	 vec->type = VEC_NULL;
	 return(-1);
      }
   } else if(vec->type == VEC_STRING) {
      if(dimen == 0) {			/* some mallocs don't like malloc(0) */
	 vec->vec.s.s = (char **)malloc(1);
	 vec->vec.s.s_s = malloc(1);
      } else {
	 vec->vec.s.s = (char **)malloc((unsigned)dimen*sizeof(char *));
	 vec->vec.s.s_s = malloc((unsigned)dimen*VEC_STR_SIZE);
      }

      if(vec->vec.s.s == NULL || vec->vec.s.s_s == NULL) {
	 if(vec->vec.s.s != NULL) {
	    free((char *)vec->vec.s.s);
	 }
	 if(vec->vec.s.s_s != NULL) {
	    free((char *)vec->vec.s.s_s);
	 }
	 vec->type = VEC_NULL;
	 return(-1);
      }
      for(i = 0;i < dimen;i++) {
	 vec->vec.s.s[i] = vec->vec.s.s_s + i*VEC_STR_SIZE;
      }
   } else {
      msg_1d("unknown vector type %d\n",vec->type);
      return(-1);
   }

   vec->dimen = dimen;
   return(0);
}

/*************************************************************/
/*
 * Reallocate space for vector->vec
 */
int
vec_realloc(vec,dimen)
VECTOR *vec;
int dimen;
{
   int i;

   if(vec == NULL) {
      return(0);
   }
   
   if(vec->type == VEC_NULL) {
      ;
   } else if(vec->type == VEC_FLOAT) {
      if(dimen == 0) {
	 if(vec->vec.f != NULL) {
	    free((char *)vec->vec.f); vec->vec.f = NULL;
	 }
	 return(vec_malloc(vec,dimen));
      }
      if((vec->vec.f = (REAL *)realloc((char *)vec->vec.f,
					(unsigned)dimen*sizeof(REAL)))
	 							== NULL) {
	 vec->type = VEC_NULL;
	 return(-1);
      }
   } else if(vec->type == VEC_STRING) {
      if(dimen == 0) {
	 if(vec->vec.s.s != NULL) {
	    free((char *)vec->vec.s.s);
	    free((char *)vec->vec.s.s_s);
	    vec->vec.s.s = NULL;
	 }
	 return(vec_malloc(vec,dimen));
      }
      (void)free((char *)vec->vec.s.s);
      if((vec->vec.s.s = (char **)malloc((unsigned)dimen*sizeof(char *)))
	 							== NULL) {
	 vec->type = VEC_NULL;
	 return(-1);
      }
      if((vec->vec.s.s_s = realloc(vec->vec.s.s_s,
				   (unsigned)dimen*VEC_STR_SIZE))
	 							== NULL) {
	 free((char *)vec->vec.s.s);
	 vec->type = VEC_NULL;
	 return(-1);
      }
      for(i = 0;i < dimen;i++) {
	 vec->vec.s.s[i] = vec->vec.s.s_s + i*VEC_STR_SIZE;
      }
   } else {
      msg_1s("Vector %s has ",vec->name);
      msg_1d("unknown type: %d\n",vec->type);
      return(-1);
   }

   vec->dimen = dimen;
   return(0);
}

/*************************************************************/
/*
 * free space for vector->vec
 */
void
vec_free(vec)
VECTOR *vec;
{
   if(vec == NULL) return;

   if(vec->type == VEC_FLOAT) {
      if(vec->vec.f != NULL) free((char *)vec->vec.f);
   } else if(vec->type == VEC_STRING) {
      if(vec->vec.s.s != NULL) free((char *)vec->vec.s.s);
      if(vec->vec.s.s_s != NULL) free(vec->vec.s.s_s);
   } else if(vec->type == VEC_NULL) {
      ;
   } else {
      msg_1d("unknown vector type %d\n",vec->type);
   }
   vec->dimen = 0;
   vec->type = VEC_NULL;
   vec->name = "(null)";
}

/*************************************************************/
/*
 * Make a vector of dimension dimen, named name, type type
 */
VECTOR *
make_vector(name,dimen,type)
char name[];				/* name of vector */
int dimen,				/* dimension of vector */
    type;				/* type of vector */
{
   vvector = NULL;
   ddimen = dimen;
   if((ttype = type) != VEC_NULL && ttype != VEC_FLOAT && ttype != VEC_STRING){
      msg_1d("Unknown vector type %d",(int)ttype);
      msg_1s("for vector %s\n",name);
      return(NULL);
   }

   insert_node(name,&vv);
   return(vvector);
}

/*************************************************************/
/*
 * Make a local vector
 */
void
make_vector_local(name)
char name[];				/* name of vector */
{
   vvector = NULL;
   ddimen = 0;
   ttype = VEC_NULL;

   insert_node_local(name,&vv);
}

/**********************************************/
/*
 * make_anon_vector returns storage for a vector, but doesn't name it
 */
int
make_anon_vector(vect,dimen,type)
VECTOR *vect;
int dimen,
    type;
{
   vect->type = type;
   if(vec_malloc(vect,dimen) < 0) {
      msg("Can't get storage for anon vector\n");
      *vect = zero_vec;
      (void)vec_value(vect,0.0);
      return(-1);
   }
   
   vect->name = "(anonymous)";
   vect->descrip[0] = '\0';
   return(0);
}

/**********************************************/
/*
 * copy_vector copies a vector into the pool, and names it
 */
void
copy_vector(name,vector)
char *name;			/* name of vector */
VECTOR vector;			/* vector to be preserved */
{
   if(vector.type == VEC_NULL) return;
   
   vvector = &vector;

   insert_node(name,&vv);
}

static Void *
vemake(name,nn)
char *name;
Void *nn;
{
   VECTOR *vector;
   
   vector = (VECTOR *)nn;

   if(vector != NULL) {			/* free old vector */
      if(sm_verbose > 1) {
	 msg_1s("Redefining %s\n",name);
      }
      if(vvector == NULL && vector->dimen == ddimen &&
						vector->type == ttype) {
	 ;			/* We can re-use the old storage */
      } else {
	 vec_free(vector);
	 vector->type = VEC_NULL;
      }
   }
/*
 * Make a new vector if there isn't one around
 */
   if(vector == NULL) {		/* can't re-use storage */
      if((vector = (VECTOR *)malloc(sizeof(VECTOR))) == NULL) {
	 msg_1s("Can't get storage for vector %s\n",name);
	 return(NULL);
      }
      vector->type = VEC_NULL;
   }
/*
 * There are two alternatives. If vvector is non-NULL use it for the
 * new vector, otherwise make one with dimension ddimen
 */
   if(vvector != NULL) {
      *vector = *vvector;
   } else {
      if(vector->type == VEC_NULL) {
	 vector->type = ttype;
	 if(vec_malloc(vector,ddimen) < 0) {
	    msg_1s("Can't get storage for vector %s\n",name);
	    free((char *)vector);
	    vvector = NULL;
	    return(NULL);
	 }
      }
   }
   
   vector->name = name;		/* storage is in tree structure */
   vector->descrip[0] = '\0';
   vvector = vector;		/* Pass new address back as a global */
   return((Void *)vector);
}

/*************************************************************/
/*
 * free_vector frees a named vector back to the pool
 */
void
free_vector(name)
char *name;			/* name of vector to free */
{
   delete_node(name,&vv);
}

void
free_vector_local(name)
char *name;			/* name of vector to free */
{
   delete_node_local(name,&vv);
}

static void
vekill(nn)
Void *nn;			/* node to kill */
{
   VECTOR *vector;

   vector = (VECTOR *)nn;
   vec_free(vector);
   free((char *)vector);
}

/*************************************************************/
/*
 * get_vector returns a vector by value
 */
int
get_vector(name,vector)
char *name;			/* name of vector to get */
VECTOR *vector;			/* vector to return */
{
   VECTOR *vect;
   
   if((vect = (VECTOR *)get_rest(name,&vv)) == NULL) {
      msg_1s("Vector %s is not defined\n",name);
      *vector = zero_vec;
      (void)vec_value(vector,0.0);
      return(-1);
   } else if(vect->type == VEC_NULL) {
      if((vect = (VECTOR *)get_lnode_scope_rest()) == NULL) {
	 msg_1s("Vector %s is not set\n",name);
	 *vector = zero_vec;
	 (void)vec_value(vector,0.0);
	 return(-1);
      }
   } 

   *vector = *vect;
   if(vec_malloc(vector,vect->dimen) < 0) {
      msg_1s("Can't get storage for %s\n",name);
      *vector = zero_vec;
      (void)vec_value(vector,0.0);
      return(-1);
   }
   if(vector->type == VEC_FLOAT) {
      (void)memcpy((Void *)vector->vec.f,(Const Void *)vect->vec.f,
		   vector->dimen*sizeof(REAL));
   } else if(vector->type == VEC_STRING) {
      (void)memcpy((Void *)vector->vec.s.s_s,(Const Void *)vect->vec.s.s_s,
		   vector->dimen*VEC_STR_SIZE);
   } else {
      msg_1s("Vector %s is of ",vector->name);
      msg_1d("unknown type %d\n",vector->type);
   }
   
   return(vector->type);
}

/*************************************************************/
/*
 * get_vector_ptr returns a pointer to a vector
 */
VECTOR *
get_vector_ptr(name)
char *name;			/* name of vector to get */
{
   VECTOR *vector;

   if((vector = (VECTOR *)get_rest(name,&vv)) == NULL) {
      msg_1s("Vector %s is not defined\n",name);
      return(NULL);
   } else {
      return(vector);
   }
}

/*********************************************************/
/*
 * Return a help message for a vector
 */
int
help_vector(name)
char *name;				/* name of vector to be described */
{
   VECTOR *vector;

   if((vector = (VECTOR *)get_rest(name,&vv)) == NULL) {
      return(-1);
   } else {
      msg_1d("Vector[%d] ",vector->dimen);
      if(sm_verbose) {
	 switch(vector->type) {
	  case VEC_FLOAT:
	    msg("(arithmetic) "); break;
	  case VEC_STRING:
	    msg("(string) "); break;
	  default:
	    msg("(unknown type) "); break;
	 }
      }
      msg_1s(": %s\n",vector->descrip);
      return(0);
   }
}

/*********************************************************/
/*
 * Does vector exist?
 */
int
is_vector(name)
char *name;				/* name of vector to be described */
{
   VECTOR *vec;
   
   if((vec = (VECTOR *)get_rest(name,&vv)) == NULL) {
      return(0);
   } else {
      return(vec->type);
   }
}

/*********************************************************/
/*
 * list all currently defined vectors
 */
void
list_vectors()
{
   stop_list = 0;
   (void)more((char *)NULL);			/* initialise more() */

   list_nodes(&vv,(void (*)())velist);
}

static void
velist(name,nn)
char *name;
Void *nn;
{
   char line[20 + 2*VEC_DESCRIP_SIZE];
   VECTOR *vector;

   if(stop_list) return;

   vector = (VECTOR *)nn;
   (void)sprintf(line,"Vector %s [%d] : %s\n",name,
					       vector->dimen,vector->descrip);
   if(more(line) < 0) stop_list = 1;
}

/*******************************************************/
/*
 * Set the help string for a vector
 */
void
set_help_vector(name,descrip)
char *name,				/* name of vector to be described */
     *descrip;				/* string to be used */
{
   VECTOR *vector;

   if((vector = (VECTOR *)get_rest(name,&vv)) == NULL) {
      msg_1s("Vector %s is not defined\n",name);
   } else {
      (void)strncpy(vector->descrip,descrip,VEC_DESCRIP_SIZE);
   }
   return;
}

/*******************************************************/
/*
 * Save all the vectors to a file
 */
static FILE *fil;

void
save_vectors(ffil)
FILE *ffil;
{
   fil = ffil;
   list_nodes(&vv,(void (*)())write_vector);
}

static void
write_vector(name,nn)
char *name;
Void *nn;
{
   int i;
   VECTOR *vector;

   if(*name == '_') return;		/* don't save vectors starting _ */
   
   vector = (VECTOR *)nn;

   if(vector->type == VEC_FLOAT) {
      fprintf(fil,"%s : dimension = %d\n",name,vector->dimen);
      fprintf(fil,"help = %s\n",vector->descrip);
      for(i = 0;i < vector->dimen;i++) {
	 fprintf(fil,"%g%c",vector->vec.f[i],(i%5 == 4)?'\n':'\t');
      }
      if(i%5 != 0) (void)putc('\n',fil);
   } else if(vector->type == VEC_STRING) {
      fprintf(fil,"%s : dimension = %d.s\n",name,vector->dimen);
      fprintf(fil,"help = %s\n",vector->descrip);
      for(i = 0;i < vector->dimen;i++) {
	 fprintf(fil,"%s\n",vector->vec.s.s[i]);
      }
   } else {
      msg_1s("Vector %s has ",vector->name);
      msg_1d("unknown type %d\n",vector->type);
   }
}

/**********************************************************/
/*
 * Set a vector to have a scalar value
 */
int
vec_value(v1,val)
VECTOR *v1;				/* pointer to vector in question */
double val;				/* value desired */
{
   if((v1->vec.f = (REAL *)malloc(sizeof(REAL))) == NULL) {
      msg("Can't allocate space in vec_value\n");
      v1->dimen = 0;
      v1->type = VEC_NULL;
      return(-1);
   }
   v1->dimen = 1;
   v1->type = VEC_FLOAT;
   v1->vec.f[0] = val;
   v1->name = "(scalar)";
   return(0);
}

/*
 * Now a function to allow a user to define an array as a vector
 */
int
sm_array_to_vector(arr,n,name)
REAL *arr;
int n;
char *name;
{
   VECTOR *vec;

   if((vec = make_vector(name,0,VEC_FLOAT)) == NULL) {
      return(-1);
   }
   vec->dimen = n;
   vec->vec.f = arr;

   return(0);
}
