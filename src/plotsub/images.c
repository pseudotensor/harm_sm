#include "copyright.h"
/*
 * These are subroutines to read unformatted files, given a
 * description of their vaguaries in a `filecap' file. The files are
 * permitted to have records which are padded either at the front or
 * the back (or both) with extraneous bytes, to have a padding in front
 * of the entire file, and to have a header. Various fields are extracted
 * from this header, if requested in the filecap file.
 *
 * Filecap capabilities used are :
 *
 * DA           DAta type       char fits float(D) int long short (string)
 * FS           File Start      Unwanted bytes at start of file
 * HS           Header Size     Size of header (alias: HH)
 * NS           No Swap         Don't swap bytes, even on (e.g.) a vax
 * RE           Record End      Unwanted bytes at end of record
 * RL           Record Length   Number of useful bytes per record
 * RS           Record Start    Unwanted bytes at start of record
 * SW		SWap		Swap bytes (ab --> ba, abcd --> dcba)
 * nx,ny        Number X, Y     byte offsets of X and Y sizes of data
 *                              within the first record, sizes are ints
 *
 * Most parameters are optional, and will default to 0. If RE or RS is
 * specified, you must give RL as well. If you specify it as -ve, then
 * we'll look for it from the O/S.
 * Nx and ny must be present. Note that HH excludes FS, so HH will usually
 * be 2*sizeof(int) irrespective of the value of FS, and nx will usually be 0.
 */
#include "options.h"
#include <stdio.h>
#include <ctype.h>
#include "sm.h"
#include "image.h"
#ifdef vms
#  include <stat.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#endif
#include "tty.h"
#include "tree.h"
#include "vectors.h"
#include "sm_declare.h"

#define FILECAP_SIZE 200		/* buffer for list of filecaps */
#define FITS_SIZE 2880
#define SSIZE 40                        /* size for small buffers */
extern int sm_interrupt,                   /* respond to ^C */
	   sm_verbose;                     /* control volubility */

#define CONVERT                 /* convert cbuff to float in zz */      \
   { register int jj;                                                   \
     switch(dtype) {                                                    \
       case CHAR:                                                       \
	 for(jj = 0;jj < rsize;jj++) {                                  \
	    *((float *)&zz[zi] + jj) = *(cbuff+jj); } break;            \
       case INT:                                                        \
	 for(jj = 0;jj < rsize/sizeof(int);jj++) {                      \
	    *((float *)&zz[zi] + jj) = *((int *)cbuff + jj); } break;   \
       case LONG:                                                       \
	 for(jj = 0;jj < rsize/sizeof(long);jj++) {                     \
	    *((float *)&zz[zi] + jj) = *((long *)cbuff + jj); } break;  \
       case SHORT:                                                      \
	 for(jj = 0;jj < rsize/sizeof(short);jj++) {                    \
	    *((float *)&zz[zi] + jj) = *((short *)cbuff + jj); } break; \
       default:                                                         \
	 msg("Impossible dtype in get_image!\n"); abort();                \
     }}
#define BITS_IN_CHAR 8                  /* convert FITS BITPIX to sizeof */
#define CHAR 0                          /* symbolic names for dtype */
#define FLOAT 1
#define INT 2
#define LONG 3
#define SHORT 4
#define DOUBLE 5
#define STRING 6
#define HEAP 7
#define SIZE 10240              /* size of buffer to use in simple cases */

static char *get_capstr(),
	    *get_keyword(),
	    *header = NULL,		/* storage for header entry */
            i_table_fmt[2*SSIZE];	/* table format from filecap */
static IMAGE *read_image_data P_ARGS(( char *, char *, int, int, int, char *, VECTOR **, int *, int, int *, int *, int * ));

extern int sm_interrupt,                   /* respond to ^C */
	   sm_verbose;                     /* control volubility */
static int find_entry(),
	   get_capint(),
	   get_recl(),
	   header_size = 0,		/* size of header */
	   is_capval(),
           parse_table_format P_ARGS(( char *, VECTOR **, int, int, int, \
				      int *, int *, int * )),
           parse_table_names P_ARGS(( char *, VECTOR **, int, int, \
				     int *, int *, int * )),
	   read_card(),
	   read_fits_header(),
	   read_fits_image_header(),
	   read_fits_table_header(),
	   swap_bytes = 0;			/* do we want to byte swap? */
static void swap_2(),
	    swap_4();

/*****************************************************************************/

void
read_table_header(filename, subtable)
char *filename;				/* file to read */
int subtable;				/* which table (the PDU is 0) */
{
   kill_keywords();			/* kill old keywords from header */
/*
 * Read the table
 */
   if(subtable > 0 && sm_verbose) {
      msg_1d("skipping to table %d\n",subtable);
   }
   (void)read_image_data(filename,"",-1,-1,subtable,
						"",NULL,NULL,0,NULL,NULL,NULL);
}

int
read_table(filename, subtable, vecs, nvec, nextra,
	   line_1, line_2, how, table_fmt)
char *filename;				/* file to read */
int subtable;				/* which table (the PDU is 0) */
VECTOR **vecs;				/* array of vectors to fill */
int nvec;				/* how many vectors there are */
int nextra;				/* number of extra vectors allocated */
int line_1, line_2;			/* which lines to read */
char *how;				/* how to map names to columns */
char *table_fmt;			/* format to use for table */
{
   char *cptr;				/* char pointer to data */
   IMAGE *image;			/* IMAGE of the table's data */
   int i,j;
   int nrow;				/* number of rows read */
   int *offset;				/* offsets of data in a row */
   int *size;				/* size of element of data */
   int *type;				/* type of data element */
   char tmp;				/* used in byte swapping */
/*
 * values to memcpy data to, to avoid alignment problems
 */
   char cvalue;
   short svalue;
   int ivalue;
   long lvalue;
   double dvalue;
   float fvalue;

   kill_keywords();			/* kill old keywords from header */
/*
 * Figure out where the data is in the rows read from the table
 */
   if((offset = (int *)malloc(3*(nvec + nextra)*sizeof(int))) == NULL) {
      msg("Can't allocate storage for offset\n");
      return(0);
   }
   size = offset + nvec + nextra;
   type = size + nvec + nextra;
/*
 * look for a table format
 */
   if(strcmp(how,"byname") == 0) {
      ;					/* we don't need one */
   } else {
      if(how[0] != '\0' &&
	 strcmp(how,"bycolumn") != 0) {	/* it's a table format */
	 table_fmt = how;
      } else if(*table_fmt == '\0') {	/* none was given with TABLE */
	 table_fmt = i_table_fmt;	/* so check the filecap file */
	 if(*table_fmt == '\0') {	/* no; use a default */
	    table_fmt = "f*";
	    msg_2s("I cannot locate a format for file %s, using \"%s\"\n",
		   filename,table_fmt);
	 }
      }
   }
/*
 * Read the table
 */
   if(subtable > 0 && sm_verbose) {
      msg_1d("skipping to table %d\n",subtable);
   }
   if((image = read_image_data(filename,how,line_1,line_2,subtable,table_fmt,
			       vecs,&nvec,nextra,offset,size,type)) == NULL) {
      free((char *)offset);
      return(0);
   }
/*
 * we've read the data, now allocate the vectors to receive it
 */
   if(line_1 > 0) line_1--;		/* it isn't 0-indexed */
   nrow = image->ny - line_1;

   for(j = 0;j < nvec;j++) {
      if(vec_malloc(vecs[j],nrow) < 0) {
	 msg_1d("Can't get space for vector %d in read_table\n",i);
	 nvec = i;
	 break;
      }
   }
/*
 * and transfer the data from the image to the vectors
 */
   for(i = 0;i < nrow;i++) {
      for(j = 0;j < nvec;j++) {
	 cptr = (char *)image->ptr[line_1 + i] + offset[j];

	 if(swap_bytes) {
	    switch (type[j]) {
	     case CHAR:
	     case STRING:
	       break;
	     case SHORT:
	       tmp = cptr[0]; cptr[0] = cptr[1]; cptr[1] = tmp;
	       break;
	     case INT:
	     case LONG:
	     case FLOAT:
	       tmp = cptr[0]; cptr[0] = cptr[3]; cptr[3] = tmp;
	       tmp = cptr[1]; cptr[1] = cptr[2]; cptr[2] = tmp;
	       break;
	     case DOUBLE:
	       tmp = cptr[0]; cptr[0] = cptr[7]; cptr[7] = tmp;
	       tmp = cptr[1]; cptr[1] = cptr[6]; cptr[6] = tmp;
	       tmp = cptr[2]; cptr[2] = cptr[5]; cptr[5] = tmp;
	       tmp = cptr[3]; cptr[3] = cptr[4]; cptr[4] = tmp;
	       break;
	     default:
	       msg_1d("unknown value of type in read_table: %d\n",type[j]);
	       type[j] = CHAR;
	       break;
	    }
	 }

	 switch (type[j]) {
	  case STRING:
	    strncpy(vecs[j]->vec.s.s[i],cptr,size[j]);
	    vecs[j]->vec.s.s[i][size[j]] = '\0';
	    break;
	  case CHAR:
	    memcpy(&cvalue,cptr,sizeof(char));
	    vecs[j]->vec.f[i] = cvalue;
	    break;
	  case SHORT:
	    memcpy(&svalue,cptr,sizeof(short));
	    vecs[j]->vec.f[i] = svalue;
	    break;
	  case INT:
	    memcpy(&ivalue,cptr,sizeof(int));
	    vecs[j]->vec.f[i] = ivalue;
	    break;
	  case LONG:
	    memcpy(&lvalue,cptr,sizeof(long));
	    vecs[j]->vec.f[i] = lvalue;
	    break;
	  case FLOAT:
	    memcpy(&fvalue,cptr,sizeof(float));
	    vecs[j]->vec.f[i] = fvalue;
	    break;
	  case DOUBLE:
	    memcpy(&dvalue,cptr,sizeof(double));
	    vecs[j]->vec.f[i] = dvalue;
	    break;
	  default:
	    msg_1d("impossible value of type in read_table: %d\n",type[j]);
	    abort();
	    break;
	 }
      }
   }

   free((char *)offset);
   freeimage(&image);

   return(nvec);
}

/*****************************************************************************/
/*
 * Read an image from a file
 */
IMAGE *
get_image(filename)
char *filename;
{
   IMAGE *image;
   int i,j;
   float deltx,delty,			/* point spacing in user coords */
	 max_val;			/* maximum allowed float */
   
   kill_keywords();			/* kill old keywords from header */

   if((image = read_image_data(filename,NULL,0,0,0,
			       "",NULL,NULL,0,NULL,NULL,NULL)) == NULL) {
      return(NULL);
   }
/*
 * check that all the values in the image are valid
 */
   max_val = pow(10.0,(float)(MAX_POW));
   deltx = image->nx <= 1 ? 1 : (image->xmax - image->xmin)/(image->nx - 1);
   delty = image->ny <= 1 ? 1 : (image->ymax - image->ymin)/(image->ny - 1);

   for(j = 0;j < image->ny;j++) {
      for(i = 0;i < image->nx;i++) {
	 if(image->ptr[j][i] != image->ptr[j][i]) { /* must be a NaN */
	    if(sm_verbose > 1) {
	       msg_1f("Image value of %g ",image->ptr[j][i]);
	       msg_1f("at (%g,",image->xmin + i*deltx);
	       msg_1f("%g) is a NaN\n",image->ymin + j*delty);
	    }
	    image->ptr[j][i] = max_val;
	 } else if(image->ptr[j][i] > max_val) { /* could be inf */
	    image->ptr[j][i] = max_val;
	 }
      }
   }
   
   return(image);
}

/*****************************************************************************/
/*
 * Actually process a binary file, read as an IMAGE or a TABLE
 *
 * IMAGEs convert all their inputs to float, whereas TABLEs want the
 * raw bytes from the file; this is done by suitable casts controlled
 * by is_table
 *
 * If line_1 is negative, only read the header
 */
static IMAGE *
read_image_data(filename, how, line_1, line_2, which,
		table_fmt,vecs,nvec,nextra,offset,elsize,type)
char *filename;				/* file to read */
char *how;				/* how to read table, or NULL */
int line_1, line_2;			/* range of desired rows */
int which;				/* desired data unit */
char *table_fmt;			/* format to read */
VECTOR **vecs;				/* vectors that we'll be reading */
int *nvec;				/* number of vectors */
int nextra;				/* number of extra vectors allocated */
int *offset;				/* offsets of elements in a row */
int *elsize;				/* sizes of elements in a row */
int *type;				/* type of an element */
{
   char arr[SIZE],                      /* array for header */
   	buff[30],			/* temp. buffer */
	*cbuff = NULL,			/* buffer for converting to floats
					   (initialised to NULL to placate
					   flow-analysing compilers) */
	*data_type,                     /* type of data */
	filecap[FILECAP_SIZE],		/* name of filecap files */
	*file_type,                     /* name of entry in filecap */
	*ptr,
	*zz;                            /* storage for z */
   float bscale =  0.0,                 /* if non-zero, */
	 bbzero = 0.0,                  /*  data = bbzero + bscale*data */
	 xmin,xmax,                     /* range of x and */
	 ymin,ymax,                     /*        y coordinates */
	 **z;                           /* pointers to image */
   IMAGE *image;                        /* the image to create */
   int dsize,                           /* size of each datum */
       dtype,                           /* type of data in file */
       du,				/* the "data unit" that we're reading*/
       fd = -1,                         /* file descriptor for data */
       fits = 0,                        /* is it a FITS image? */
       i,				/* index, esp. to data as read */
       file_start,                      /* dead space at start of file */
       is_table = (how == NULL) ? 0 : 1, /* we are reading a table */
       head_size,                       /* size of header */
       head_start,			/* offset of start of header */
       have_xlims = 0,have_ylims = 0,   /* were x/y limits on header? */
       n_read,				/* number of bytes still to read */
       nx,ny,                           /* dimension of image */
       rec_start,                       /* dead space at start of record */
       rec_len,                         /* length of data per record */
       rec_end,                         /* dead space at end of record */
       row0,				/* first row of data to read */
       rsize,                           /* number of bytes to read at a time */
       size,                            /* space needed for data */
       zi;                              /* index into z; != i if not floats */

   if((fd = open(filename,0)) < 0) {
      msg_1s("Can't open %s\n",filename);
      return(NULL);
   }
/*
 * Find the `filecap' file, then the characteristics of file,
 * then read the header and then the data itself
 * Note that the filecap entries will usually be in the graphcap file,
 * and this will be assumed if we can't find filecap.
 */
   if(*(ptr = print_var("filecap")) == '\0') {
      if(*(ptr = print_var("graphcap")) == '\0') {
	 msg("Can't find filecap/graphcap in environment file\n");
	 (void)close(fd);
	 return(NULL);
      }
   }
   (void)strncpy(filecap,ptr,FILECAP_SIZE);
   if(is_table) {
      file_type = print_var("table_type");
      if(file_type[0] == '\0') {
	 msg("No variable table_type is defined, ");
	 msg("using value of \"file_type\" instead\n");
	 file_type = print_var("file_type");
      }
   } else {
      file_type = print_var("file_type");
   }
   if(file_type[0] == '\0') {
      msg("No variable file_type is defined, assuming \"C\"\n");
      find_entry("C",filecap);
   } else {
      find_entry(file_type,filecap);
   }

   head_start = file_start = get_capint("FS");
   if((head_size = get_capint("HS")) == 0) {
      head_size = get_capint("HH");     /* old name for it */
   }
   rec_start = get_capint("RS");
   rec_len = get_capint("RL");
   rec_end = get_capint("RE");
   data_type = get_capstr("DA");
   if(*data_type == '\0') data_type = "float";  /* as a default */
/*
 * process values, looking for errors and special values.
 *
 * If HS is -ve, assume that the file has a record structure
 * that even applies to the header. If RL is specified, use it for HS.
 * Otherwise get it from the O/S.
 *
 * If RL is -ve, get it from the O/S
 */
   if(sm_verbose > 0) {
      msg_1s("Data type is %s\n",data_type);
   }
   if(!strcmp(data_type,"char")) {
      dtype = CHAR; dsize = sizeof(char);
   } else if(!strcmp(data_type,"fits")) {
      fits = 1;
#if 0					/* VMS disk fits have recl != 2880 */
      if(rec_len != 0 && rec_len != FITS_SIZE) {
	 msg_1d("FITS files must have a record length of %d\n",FITS_SIZE);
      }
      head_size = rec_len = FITS_SIZE;
#endif
      head_size += rec_start;           /* fake up the header */
      head_size += rec_end;
      head_start += rec_start;
      dtype = INT;			/* we'll check in read_fits_header() */
   } else if(!strcmp(data_type,"float")) {
      dtype = FLOAT; dsize = sizeof(float);
   } else if(!strcmp(data_type,"int")) {
      dtype = INT; dsize = sizeof(int);
   } else if(!strcmp(data_type,"long")) {
      dtype = LONG; dsize = sizeof(long);
   } else if(!strcmp(data_type,"short")) {
      dtype = SHORT; dsize = sizeof(short);
   } else {
      msg_1s("Data type %s not supported\n",data_type);
      (void)close(fd);
      return(NULL);
   }

   swap_bytes = 0;			/* the default */
#ifdef NEED_SWAP			/* we need to byte swap FITS */
   if(fits) swap_bytes = !is_capval("NS"); /* unless explicitly forbidden */
#endif
   if(is_capval("SW")) swap_bytes = 1;
   if(swap_bytes) {
      if(dtype == SHORT || dtype == INT || dtype == LONG) {
	 ;				/* OK */
      } else {
	 msg_1s("You cannot request byte swapping for data type %s\n",
		data_type);
	 swap_bytes = 0;
      }
   }

   if(head_size < 0) {
      if(rec_len > 0) {
	 head_size = rec_len;
      } else {
	 if((head_size = get_recl(fd)) < 0) {
	    msg_1s("Can't get record length for %s\n",filename);
	    close(fd);
	    return(NULL);
	 }
      }
      head_size += rec_start;           /* fake up the header */
      head_size += rec_end;
      head_start += rec_start;
   }
   if(rec_start > SSIZE) {
      msg_1d("rec_start is too large, > %d\n",SSIZE);
      (void)close(fd);
      return(NULL);
   }
   if((rec_start || rec_end) && !rec_len) {
      msg("You must specify RL in filecap if you use RS or RE; Assuming -1\n");
      rec_len = -1;
   }
/*
 * Loop through the file, reading headers and (maybe) skipping over the
 * data unit
 */
   for(du = 0;;du++) {
      if(fits) {                   /* we already have rec_len */
	 if((size = file_start + head_size) > SIZE) {
	    msg("Array in get_image is too small to read header\n");
	    (void)close(fd);
	    return(NULL);
	 }
	 ptr = arr; 
	 for(i = rec_len;i < size;i += rec_len) {
	    if(read(fd,ptr,rec_len) != rec_len) {
	       msg_1d("Error reading header for data unit %d\n",du);
	       (void)close(fd);
	       return(NULL);
	    }
	    ptr += rec_len;     
	 } 
	 n_read = size - (ptr - arr);	/* this many by of size still unread */
	 if(read(fd, ptr, n_read) != n_read) {
	    msg("Error reading end of header\n");
	    (void)close(fd);
	    return(NULL);
	 }
/*
 * Process the header. Note that the first header of a fits binary table
 * is a regular SIMPLE=T header
 */
	 nx = -1;			/* this is the first header record */
	 for(;(i = read_fits_header(is_table,du,&arr[head_start],
				    &nx,&ny,&dtype,&dsize,&bscale,&bbzero,
				    &xmin,&xmax,&ymin,&ymax)) != 0;){
	    if(i < 0) {			/* error on header */
	       (void)close(fd);
	       return(NULL);
	    } else {			/* not seen END card yet */
	       size -= file_start;
	       file_start = 0;		/* we've read file_start already */
	       head_start = 0;		/* and skipped the header start */
	       n_read = rec_len - n_read; /* How many bytes we must still read 
					     from current record */
	       ptr = arr;
	       for(i = n_read; i < size ;i += rec_len) {
		  if(read(fd, ptr, n_read) != n_read) {
		     msg("Error reading header\n");
		     (void)close(fd);
		     return(NULL);
		  }
		  ptr += n_read;
		  n_read = rec_len;
	       } 
	       n_read = size - (ptr -arr);
	       if((i = read(fd, ptr, n_read)) != n_read) {
		  msg("Error reading header\n");
		  (void)close(fd);
		  return(NULL);
	       }
	       ptr += n_read;
	    }
	    if(xmin != 0 || xmax != nx - 1) have_xlims = 1;
	    if(ymin != 0 || ymax != ny - 1) have_ylims = 1;
	 }
      } else {                             /* Not FITS */
	 if((size = file_start + head_size) > SIZE) {
	    msg("Array in get_image is too small to read header\n");
	    (void)close(fd);
	    return(NULL);
	 }
	 ptr = arr;
	 if(rec_len <= 0) {
	    if(read(fd, ptr, size) != size) {
	       msg("Error reading header\n");
	       (void)close(fd);
	       return(NULL);
	    }
	    n_read = 0;			/* num of bytes of 1st record unread */
	 } else {
	    for(i = 0;i < size - rec_len;i += rec_len) {
	       if(read(fd, ptr, rec_len) != rec_len) {
		  msg("Error reading header\n");
		  (void)close(fd);
		  return(NULL);
	       }
	       ptr += rec_len;
	    }
	    n_read = size - (ptr - arr); /* num of bytes of 1st record unread*/
	 }
	 if((i = read(fd, ptr, n_read)) != n_read) {
	    msg("Error reading header\n");
	    (void)close(fd);
	    return(NULL);
	 }
	 
	 if(head_size < 200) {		/* save the header */
	    if(header != NULL) free(header);
	    if((header = (char *)malloc(head_size)) != NULL) {
	       memcpy(header,arr + file_start,head_size);
	       header_size = head_size;
	    }
	 }
	 if(!is_capval("nx") && !is_capval("ny")) {
	    prompt_user("Enter nx and ny ");
	    if(mgets(arr,SIZE) == NULL || str_scanf(arr,"%d %d",&nx,&ny) != 2){
	       msg("Error reading size of image interactively\n");
	       (void)close(fd);
	       return(NULL);
	    }
	 } else {
	    if(!is_capval("nx") || !is_capval("ny")) {
	       msg("Size of image is not present in header\n");
	       (void)close(fd);
	       return(NULL);
	    }
/*
 * I hope that the values in filecap won't give an alignment problem
 */
	    if(*(ptr = get_capstr("nx")) != '\0' && *ptr != '#') {
	       nx = atoi(ptr);		/* the value of nx */
	       if(sm_verbose > 1) msg_1d("nx is %d\n",nx);
	    } else {
	       nx = get_capint("nx");
	       if(nx < 0) {
		  if(sm_verbose > 1) msg_1d("nx is %d, ",-nx);
		  nx = -nx;
	       } else {
		  if(sm_verbose > 1) {
		     msg_1d("nx is at offset %d, ",head_start + nx);
		  }
		  nx = *(int *)&arr[head_start + nx];
	       }
	    }
	    
	    if(*(ptr = get_capstr("ny")) != '\0' && *ptr != '#') {
	       ny = atoi(ptr);		/* the value of ny */
	       if(sm_verbose > 1) msg_1d("ny is %d\n",ny);
	    } else {
	       ny =  get_capint("ny");
	       if(ny < 0) {
		  if(sm_verbose > 1) msg_1d("ny is %d\n",-ny);
		  ny = -ny;
	       } else {
		  if(sm_verbose > 1) {
		     msg_1d("ny is at offset %d\n",head_start + ny);
		  }
		  ny = *(int *)&arr[head_start + ny];
	       }
	    }
	 }
/*
 * check that nx and ny are reasonable. Say, >= 0 && size <= 1e8
 */
	 if(nx <= 0 || ny <= 0) {
	    msg_1d("You have nx = %d, ",nx);
	    msg_1d("ny = %d\n",ny);
	    (void)close(fd);
	    return(NULL);
	 }
	 if((unsigned)nx*ny > 1e8) {    /* probably junk */
	    msg_1d("You have nx = %d, ",nx);
	    msg_1d("ny = %d, is this correct?\n",ny);
	    msg("enter 1 to continue  ");
	    (void)scanf("%d%*c",&i);
	    if(!i) {
	       (void)close(fd);
	       return(NULL);
	    }
	 }
      }
      (void)sprintf(buff,"%d",nx); set_image_variable("NX",buff);
      (void)sprintf(buff,"%d",ny); set_image_variable("NY",buff);
/*
 * look for x/y limits of image,
 * and hope that the values in filecap won't give an alignment problem
 */
      if(is_capval("x0")) {
	 if(!is_capval("x1")) {
	    msg("You must specify neither or both of x0 and x1 in a file\n");
	 } else {
	    xmin = *(float *)&arr[file_start + rec_start + get_capint("x0")];
	    xmax = *(float *)&arr[file_start + rec_start + get_capint("x1")];
	    (void)sprintf(buff,"%g",xmin); set_image_variable("X0",buff);
	    (void)sprintf(buff,"%g",xmax); set_image_variable("X1",buff);
	    have_xlims = 1;
	 }
      }
      if(is_capval("y0")) {
	 if(!is_capval("y1")) {
	    msg("You must specify neither or both of y0 and y1 in a file\n");
	 } else {
	    ymin = *(float *)&arr[file_start + rec_start + get_capint("y0")];
	    ymax = *(float *)&arr[file_start + rec_start + get_capint("y1")];
	    (void)sprintf(buff,"%g",ymin); set_image_variable("Y0",buff);
	    (void)sprintf(buff,"%g",ymax); set_image_variable("Y1",buff);
	    have_ylims = 1;
	 }
      }

      if(sm_verbose > 0) {
	 msg_1s("File %s",filename);
	 if(which != 0) {
	    msg_1d(" (Data unit %d)",du);
	 }
	 msg_1d("'s dimensions are %d*",nx);
	 msg_1d("%d\n",ny);
	 if(!is_table) {
	    if(have_xlims) {
	       msg_1f("x: %g - ",xmin);
	       msg_1f("%g\n",xmax);
	    }
	    if(have_ylims) {
	       msg_1f("y: %g - ",ymin);
	       msg_1f("%g\n",ymax);
	    }
	 }
      }
/*
 * Now get the record length if we were asked for it. We couldn't get it
 * before because the first record (with header information) can have a
 * different value of rec_len
 */
      if(rec_len < 0) {            /* must get it from the O/S */
	 if((rec_len = get_recl(fd)) < 0) {
	    msg_1s("Can't get record length for %s\n",filename);
	    close(fd);
	    return(NULL);
	 }
      }
/*
 * Ready to allocate for and read the main body of the file, if we've got
 * that far in the file. Otherwise skip over the data unit. If we were
 * asked for the 0th data unit, and is was empty, skip on to the next
 */
      if(du == which) {
	 if(which == 0 && nx*ny == 0) {
	    if(sm_verbose) {
	       msg_2s("%s%s\n","You asked for TABLE 0 but it's empty, ",
		      "so I'll give you TABLE 1 instead");
	    }
	    which = 1;
	 } else {
	    break;
	 }
      }
/*
 * Not there yet; try to skip table
 */
      if(!is_table) {
	 msg("I only know how to skip certain types of tables, not images\n");
	 msg_1s("Try setting FS suitably in the filecap file, %s",
		"and read the `first' image\n");
	 close(fd);
	 return(NULL);
      } else {
	 if(fits) {
	    if(*(ptr = get_keyword("PCOUNT")) == '\0') {
	       if(du > 0) {
		  msg("Cannot find keyword PCOUNT; setting to 0\n");
	       }
	    }
	    size = atoi(ptr) + nx*ny;		/* size of main table + heap */
	    size = (size + FITS_SIZE - 1)/FITS_SIZE;	/* number of records */
	    size *= (rec_start + FITS_SIZE + rec_end); /* record size with
							  start/end junk */
	    
	    if(lseek(fd,size,1) == -1) {
	       msg_1d("Cannot seek to end of table %d\n",du);
	       close(fd);
	       return(NULL);
	    }
	 } else {
	    msg("I only know how to skip tables in FITS binary table files\n");
	    msg_1s("Try setting FS suitably in the filecap file, %s",
		   "and read the `first' table\n");
	    close(fd);
	    return(NULL);
	 }
      }
   }
/*
 * See if the file specifies a table format; this may well be overridden
 * with one in the TABLE or READ TABLE commands, but it doesn't hurt to check
 */
   if(is_table) {
      int fmt_start, fmt_len;		/* position of format in header */
      strncpy(i_table_fmt,get_capstr("FM"),2*SSIZE);
      if(i_table_fmt[0] == '\0' && header != NULL) { /* no, see if there's
							one in the file*/
	 fmt_start = get_capint("FS");
	 fmt_len = get_capint("FL");
	 if(fmt_len > 2*SSIZE) {
	    msg_1d("The maximum size for a table format in %dbytes\n",2*SSIZE);
	    fmt_len = 2*SSIZE;
	 }
	 if(fmt_start < 0 || fmt_start + fmt_len >= header_size) {
	    msg_1d("The specified table format string (%d --",fmt_start);
	    msg_1d(" %d) is outside the header\n",fmt_start + fmt_len);
	 } else {
	    strncpy(i_table_fmt,&header[fmt_start],fmt_len);
	 }
      }
   }
/*
 * If line_1 is negative, we don't really want to read any data at all,
 * and only the header was desired. We've read it, so return.
 */
   if(line_1 < 0) {
      (void)close(fd);
      return(NULL);
   }
/*
 * If we are reading a table, only some of the rows may be desired. This
 * is in general difficult, if the file has a record structure that is
 * incommensurate with the length of the rows; in this case we merely
 * stop reading when we've read enough
 */
   row0 = 0;				/* first row to read */
   if(line_1 == 0) line_1 = 1;
   if(line_2 == 0) line_2 = ny;
   
   if(is_table && (line_1 != 1 || line_2 != ny)) {
      if(line_1 <= 0 || line_2 > ny) {
	 msg_1d("Lines %d --",line_1);
	 msg_1d(" %d are out of range",line_2);
	 msg_1d(" 1 -- %d\n",ny);
	 (void)close(fd);
	 return(NULL);
      }
      
      if(rec_start + rec_len + rec_end == 0 || rec_len == dsize*nx) {
	 rec_len = dsize*nx;		/* read a line at a time */
	 if(lseek(fd,(line_1 - 1)*rec_len,1) == -1) {
	    msg_1d("Cannot seek to start of line %d\n",line_1);
	    close(fd);
	    return(NULL);
	 }
	 row0 = line_1 - 1;
	 ny = line_2 - line_1 + 1;
      } else {
	 ny = line_2;
      }
   }
/*
 * In a table we may not actually need to read all of the data to be
 * able to read the requested columns. This is important in the case of
 * very large tables, where we may not be able to fit the whole thing
 * into memory. Unfortunately, it means that we need to parse the
 * header here, in the middle of reading the file.
 */
   if(is_table) {
/*
 * If the format is "byname", try to get the names of the columns. Initially,
 * and probably as you read this, this only works for FITS binary tables.
 *
 * If it's "bycolumn" they've asked for some combination of column names and
 * numbers, so we need some combination of the format and byname lookup
 */
      if(strcmp(how,"byname") == 0 || strcmp(table_fmt,"byname") == 0) {
	 *nvec = parse_table_names("",vecs,*nvec,nextra,offset,elsize,type);
      } else if(strcmp(how,"bycolumn") == 0) {
	 *nvec =
	   parse_table_names(table_fmt,vecs,*nvec,nextra,offset,elsize,type);
      } else {
	 *nvec =
	  parse_table_format(table_fmt,vecs,*nvec,nextra,0,offset,elsize,type);
      }
/*
 * If some formats included a "*", we don't know if specified column
 * numbers were out of range. Now's the time to check.
 */
      for(i = 0;i < *nvec;i++) {
	 if(offset[i] + elsize[i] > nx) { /* Here's a bad one */
	    msg_1s("Data requested for column %s lies outside the table\n",
		   vecs[i]->name);
	    offset[i] = elsize[i] = 0;
	 }
      }
/*
 * Find the range of byte offsets that we need to read, and adjust
 * rec_start, rec_len, rec_end, and nx appropriately
 *
 * We only bother to do this if the record structure is trivial, as
 * the bookeeping would be too much of a nightmare.
 *
 * The approach used is to override the intrinsic record structure of
 * the file; this is OK on unix boxes (unless you are reading from tape),
 * but could be a problem under VMS so it can be turned OFF by setting
 * the variable table_preserve_records to be true; don't do this unless
 * you have to.
 */
      if(atoi(print_var("table_preserve_records")) == 0 &&
					      rec_start == 0 && rec_end == 0) {
	 int min = nx, max = 0;
	 for(i = 0;i < *nvec;i++) {
	    if(offset[i] < min) {
	       min = offset[i];
	    }
	    if(offset[i] + elsize[i] > max) {
	       max = offset[i] + elsize[i];
	    }
	 }

	 for(i = 0;i < *nvec;i++) {
	    offset[i] -= min;
	 }
	 
	 rec_start = min;
	 rec_end = nx - max;
	 rec_len = nx = max - min;
      }
   }
/*
 * Now we can read the real data. We must allow for the record structure
 * when reading it, which we do with seeks
 */
   size = nx*ny*dsize;
   rsize = rec_len;
   if(rsize == 0) {             /* no record structure is specified */
      rsize = (size > SIZE) ? SIZE : size;
   }
   if(size <= 0) {
      if(size == 0) {
	 msg("You have asked for 0 bytes of data\n");
      } else {
	 msg_1d("Your data has a negative size (%d)\n",size);
      }
      (void)close(fd);
      return(NULL);
   }
/*
 * Now get storage for data, and pointers to data
 */
   if((zz = malloc(nx*ny*sizeof(float))) == NULL) {
      msg("Can't allocate image in get_image\n");
      (void)close(fd);
      return(NULL);
   }
   if(dtype != FLOAT) {                 /* also need conversion buffer */
      if((cbuff = malloc(rsize)) == NULL) {
	 msg("Can't allocate cbuff in get_image\n");
	 free(zz); zz = NULL;
	 (void)close(fd);
	 return(NULL);
      }
   }
/*
 * allocate and set up the pointers to the image
 *
 * If row0 is not equal to 0, we have skipped over some lines in the
 * data file, and thus should adjust the z pointers to reflect this
 */
   if((z = (float **)malloc(ny*sizeof(float *))) == NULL) {
      msg("Can't allocate image pointers in get_image\n");
      free(zz); zz = NULL;
      if(cbuff != NULL) free(cbuff);
      (void)close(fd);
      return(NULL);
   }
   if(is_table) {
      for(i = row0;i < row0 + ny;i++) {
	 z[i] = (float *)((char *)zz + i*nx);
      }
   } else {
      for(i = row0;i < row0 + ny;i++) {
	 z[i] = (float *)zz + i*nx;
      }
   }
/*
 * We'll now skip over the first rec_start bytes (may be 0), then simply
 * read the data
 */
   if(lseek(fd,rec_start,1) == -1) {
      msg("failed to skip extra bytes at start of record\n");
   }

   for(zi = row0, i = 0;!sm_interrupt && i < size;) {
      if(rsize > size - i) {         /* may be junk at end of file */
	 rsize = size - i;
      }
      if(is_table) {
	 if((rsize = read(fd,&zz[zi],rsize)) <= 0) {
	    break;
	 }
	 i += rsize;
	 zi += rsize;
      } else {
	 if(dtype == FLOAT) {
	    if((rsize = read(fd,&zz[zi],rsize)) <= 0) {
	       break;
	    }
	    if(swap_bytes) {
	       swap_4(&zz[zi],rsize);
	    }
	 } else {
	    if((rsize = read(fd,cbuff,rsize)) <= 0) {
	       break;
	    }
	    if(swap_bytes) {
	       if(dsize == 2) {
		  swap_2(cbuff,rsize);
	       } else if(dsize == 4) {
		  swap_4(cbuff,rsize);
	       }
	    }
	    CONVERT;
	 }
	 i += rsize;
	 zi += rsize*sizeof(float)/(float)dsize;
      }
      if(rec_end + rec_start != 0) {		/* skip record structure */
	 if(lseek(fd,rec_end + rec_start,1) == -1) {
	    msg("failed to skip inter-record bytes\n");
	 }
      }
   }

   if(i < size) {                       /* we didn't find enough data */
      msg_1s("Data in file %s ",filename);
      msg_1d("contained %d ",i);
      msg_1d("not %d bytes\n",size);
   } else {                             /* check that file is all read */
      if((i = read(fd,arr,SIZE)) > rec_end) {
	 if(fits && i - rec_end < FITS_SIZE) {
	    ;				/* just the end of the record */
	 } else {
	    if(!is_table) {
	       msg_1d("At least %d bytes remain unread at end of data file\n",
							i - rec_end);
	    }
	 }
      }
   }

   (void)close(fd);
   if(dtype != FLOAT) {                 /* also free conversion buffer */
      free(cbuff);
   }
   if((image = (IMAGE *)malloc(sizeof(IMAGE))) == NULL) {
      fprintf(stderr,"Can't allocate storage for image structure\n");
      return(NULL);
   }
   strncpy(image->name,filename,sizeof(image->name) - 1);
   image->nx = nx;
   image->ny = ny;
   if(have_xlims) {                     /* from file header */
      image->xmin = xmin;
      image->xmax = xmax;
   } else {
      image->xmin = 0;
      image->xmax = nx - 1;
   }
   if(have_ylims) {
      image->ymin = ymin;
      image->ymax = ymax;
   } else {
      image->ymin = line_1 - 1;
      image->ymax = ny - 1;
   }
   image->ptr = z;
   image->space = zz;
/*
 * scale image if necessary
 */
   if(bscale != 0.0 || bbzero != 0.0) { /* it is */
      register float *zend,*zptr;

      for(zptr = z[0],zend = z[0] + nx*ny;zptr < zend;zptr++) {
	 *zptr = *zptr*bscale + bbzero;
      }
   }
   return(image);
}

char *
read_image_variable(name)
char *name;				/* name of variable to find */
{
   char buff[40];
   int offset;
   char *val;

   if(*(val = get_keyword(name)) != '\0') { /* found it */
      return(val);
   }
   if(header == NULL) {
      return("");
   }
   if(is_capval(name)) {
      if((offset = get_capint(name)) > header_size - sizeof(float)) {
	 msg_1s("Offset for variable %s is too large\n",name);
	 return("");
      }
      sprintf(buff,"%g",*(float *)(&header[offset]));
      set_image_variable(name,buff);
      return(get_keyword(name));
   }
   return("");
}

/*
 * Make an image, and define its variables
 */
int
create_image(nx,ny,xmin,xmax,ymin,ymax)
int nx,ny;			/* dimension of data array */
double xmin,xmax,ymin,ymax;	/* size of data array in user coords */
{
   char buff[30];			/* temp. buffer */
   int i;
   float *s_data;			/* storage for image */
   float **data;			/* pointers to image */
   
   if((s_data = (float *)malloc(nx*ny*sizeof(float))) == NULL) {
      fprintf(stderr,"Can't allocate storage for image\n");
      return(-1);
   }
   if((data = (float **)malloc(ny*sizeof(float *))) == NULL) {
      fprintf(stderr,"Can't allocate storage for image\n");
      free((char *)s_data);
      return(-1);
   }

   data[0] = s_data;
   for(i = 0;i < nx;i++) {
      data[0][i] = 0.0;
   }
   for(i = 1;i < ny;i++) {
      data[i] = s_data + nx*i;
      memcpy((char *)data[i],(char *)data[0],nx*sizeof(float));
   }
   
   if(sm_defimage(data,xmin,xmax,ymin,ymax,nx,ny) < 0) {
      free((char *)data);
      free((char *)s_data);
      return(-1);
   }
   set_image_name("interactive SM IMAGE");
   
   (void)sprintf(buff,"%d",nx); set_image_variable("NX",buff);
   (void)sprintf(buff,"%d",ny); set_image_variable("NY",buff);
   (void)sprintf(buff,"%g",xmin); set_image_variable("X0",buff);
   (void)sprintf(buff,"%g",xmax); set_image_variable("X1",buff);
   (void)sprintf(buff,"%g",ymin); set_image_variable("Y0",buff);
   (void)sprintf(buff,"%g",ymax); set_image_variable("Y1",buff);
   
   return(0);
}

/*****************************************************************************/

void
list_table_cols(table_fmt)
char *table_fmt;			/* format to use for table */
{
   char buff[100];			/* output buffer for more() (which is
					   assumed to be initialised) */
   char *coltype;			/* type of a column */
   int i;
   char key[80];			/* a keyword to look up */
   int nel;				/* number of elements of field */
   char *val;				/* pointer to values */

   for(i = 1;;i++) {
      sprintf(key,"TFORM%d",i);
      if(*(val = get_keyword(key)) == '\0') { /* we've seen all the columns */
	 break;
      }
      if(isdigit(*val)) {
	 nel = atoi(val);
	 while(isdigit(*val)) val++;
      } else {
	 nel = 1;
      }
      coltype = val;
      
      sprintf(key,"TTYPE%d",i);
      val = get_keyword(key);

      switch (*coltype) {
       case 'A':
	 coltype = "string";
	 break;
       case 'B':
	 coltype = "char";
	 break;
       case 'D':
	 coltype = "double";
	 break;
       case 'E':
	 coltype = "float";
	 break;
       case 'I':
	 coltype = "short";
	 break;
       case 'J':
	 coltype = "int";
	 break;
       case 'P':
	 coltype = "heap";
	 break;
       default:
	 msg_1d("Unsupported format type %c in table ",*coltype);
	 msg_1s("for column %s\n",val);
	 coltype = "(unknown)";
      }

      if(nel == 1) {
	 sprintf(buff,"%-12s %s\n",coltype,val);
      } else {
	 sprintf(buff,"%-12s %s[%d]\n",coltype,val,nel);
      }
      if(more(buff) < 0) {
	 return;
      }
   }
}

/******************************************************/
/*
 * Get the record length of the current record
 */
static int
get_recl(fd)
int fd;                                 /* current file */
{
   int recl;
#ifndef unix
   struct stat stat_buff;       /* used by fstat to interrogate system */
#endif

#if !defined(VMS)
   (void)read(fd,(char *)&recl,sizeof(int)); /* first int is record length */
   (void)lseek(fd,-sizeof(int),1);
#else
   if(fstat(fd,&stat_buff) != 0) {
      (void)close(fd);
      return(-1);
   }
   recl = stat_buff.st_fab_mrs;         /* record length for VMS */
#endif
   return(recl);
}

/*****************************************************************************/
/*
 * A routine to parse table formats such as "x10sd4". If vecs is non-NULL,
 * we use i to count through the vecs array setting the various fields. If
 * vecs is NULL, we assume that we just want the one set of values
 * corresponding to column col
 */
static int
parse_table_format(table_format,vecs,nvec,nextra,col,offset,size,type)
char *table_format;			/* format to read */
VECTOR **vecs;				/* vectors that we'll be reading */
int col;				/* target column (vecs == NULL) */
int nvec;				/* number of vectors */
int nextra;				/* number of extra vectors allocated */
int *offset;				/* offsets of elements in a row */
int *size;				/* sizes of elements in a row */
int *type;				/* type of an element */
{
   int cc;				/* counter for columns */
   char *fmt;				/* pointer to table_format */
   int i,j;
   int off;				/* offset in the line */
   char coltype;			/* type of a column */
   int nval;				/* repeat count for this element */

   off = 0;
   for(i = cc = 0, fmt = table_format;*fmt != '\0';) {
      coltype = *fmt++;
      if(isdigit(*fmt)) {
	 nval = atoi(fmt);
	 while(isdigit(*fmt)) fmt++;
      } else if(*fmt == '*') {
	 fmt++;
	 if(*fmt != '\0') {
	    msg("A * must be the last element of a table format; ");
	    msg_1s("in %s it isn't\n",table_format);
	    *fmt = '\0';
	 }
	 nval = (vecs == NULL) ? 1000000 : nvec - i;
      } else {
	 nval = 1;
      }
      
      switch (coltype) {
       case 'a':
	 if(vecs != NULL) {
	    if(i >= nvec) {
	       msg_1s("Format %s specifies too many vectors ",table_format);
	       msg_1d("(> %d)\n",nvec);
	       return(nvec);
	    }
	    
	    vecs[i]->type = VEC_STRING;
	 }
	 offset[i] = off;
	 size[i] = nval;
	 if(nval >= VEC_STR_SIZE) {
	    msg_1d("Field %c",coltype);
	    msg_1d("%d is too wide for a string vector; truncating\n",nval);
	    size[i] = VEC_STR_SIZE - 1;
	 }
	 type[i] = STRING;
	 off += nval;
	 
	 if(vecs != NULL) {
	    i++;
	 }

	 cc++;
	 if(cc == col) {		/* we're done */
	    return(1);
	 }
	 break;
       case 'c':
       case 's':
       case 'i':
       case 'f':
       case 'd':
	 for(j = 0;j < nval;j++) {
	    if(vecs != NULL) {
	       if(i >= nvec) {
		  msg_1s("Format %s specifies too many vectors ",table_format);
		  msg_1d("(> %d)\n",nvec);
		  return(nvec);
	       }
	       
	       vecs[i]->type = VEC_FLOAT;
	    }
	    offset[i] = off;
	    switch (coltype) {
	     case 'c':
	       size[i] = sizeof(char); type[i] = CHAR; break;
	     case 's':
	       size[i] = sizeof(short); type[i] = SHORT; break;
	     case 'i':
	       size[i] = sizeof(int); type[i] = INT; break;
	     case 'l':
	       size[i] = sizeof(long); type[i] = LONG; break;
	     case 'f':
	       size[i] = sizeof(float); type[i] = FLOAT; break;
	     case 'd':
	       size[i] = sizeof(double); type[i] = DOUBLE; break;
	    }
	    off += size[i];

	    if(vecs != NULL) {
	       i++;
	    }

	    cc++;
	    if(cc == col) {		/* we're done */
	       return(1);
	    }
	 }
	 break;
       case ' ':
       case 'x':
	 off += nval;
	 break;
       default:
	 msg_1s("Unknown format character in format %s:",table_format);
	 msg_1d(" %c\n",coltype);
	 break;
      }
   }
   if(i != nvec) {
      msg_1s("Format '%s' specifies",table_format);
      msg_1d(" %d",i);
      msg_1d(" not %d formats\n",nvec);
   }

   return(i);
}

/*****************************************************************************/
/*
 * A routine to take a list of vector names, and find their corresponding
 * columns. This currently works only for FITS binary tables. If there are
 * arrays in the table (e.g. TTYPE5 = id; TFORM5 = 5E), id[1] will be
 * interpreted as a request for a vector called id1 from the 2nd element.
 */
static int
parse_table_names(table_fmt,vecs,nvec,nextra,offset,size,type)
char *table_fmt;			/* format of table */
VECTOR **vecs;				/* vectors that we'll be reading */
int nvec;				/* number of vectors */
int nextra;				/* number of extra vectors allocated */
int *offset;				/* offsets of elements in a row */
int *size;				/* sizes of elements in a row */
int *type;				/* type of an element */
{
   char basename[100];			/* basename of a vector
					   (with "[...]" removed) */
   char key[80];			/* a keyword to look up */
   int i,j,k;
   int jj;				/* index for extra vecs beyond nvec */
   int ind;				/* index of an element in an array
					   of columns */
   int ind2;				/* index of end of a range of columns*/
   int *len;				/* length of names */
   int nel;				/* number of elements at this TTYPE */
   int off;				/* offset of a value in the row */
   char colsize;			/* sizeof a column (one element if
					   an array) */
   char coltype;			/* type of a column */
   int nfound;				/* number of vectors that we found */
   char *val;				/* value of a keyword */

   if((len = (int *)malloc((nvec + nextra)*sizeof(int))) == NULL) {
      msg("Can't allocate storage for len\n");
      return(0);
   }

   for(j = 0;j < nvec;j++) {
      val = strchr(vecs[j]->name,'[');
      len[j] = (val == NULL) ? strlen(vecs[j]->name) : val - vecs[j]->name;
   }
   for(j = 0;j < nvec + nextra;j++) {
      offset[j] = -1;
   }
/*
 * retrieve the column names in order, and see if we want them
 */
   jj = nvec;				/* index for "extra" vectors */
   off = 0;
   for(i = 1;;i++) {
      sprintf(key,"TFORM%d",i);
      if(*(val = get_keyword(key)) == '\0') { /* we've seen all the columns */
	 break;
      }
      if(isdigit(*val)) {
	 nel = atoi(val);
	 while(isdigit(*val)) val++;
      } else {
	 nel = 1;
      }
      coltype = *val;
      
      sprintf(key,"TTYPE%d",i);
      val = get_keyword(key);

      switch (coltype) {
       case 'A':
	 coltype = STRING;
	 colsize = sizeof(char);
	 break;
       case 'B':
	 coltype = CHAR;
	 colsize = sizeof(char);
	 break;
       case 'D':
	 coltype = DOUBLE;
	 colsize = sizeof(double);
	 break;
       case 'E':
	 coltype = FLOAT;
	 colsize = sizeof(float);
	 break;
       case 'I':
	 coltype = SHORT;
	 colsize = sizeof(short);
	 break;
       case 'J':
	 coltype = INT;
	 colsize = sizeof(int);
	 break;
       case 'P':
	 coltype = HEAP;
	 colsize = 2*sizeof(int);
	 break;
       default:
	 msg_1d("Unsupported format type %c in table ",coltype);
	 msg_1s("for column %s\n",val);

	 i = 0;
	 for(j = 0;j < nvec;j++) {
	    if(offset[j] >= 0) {
	       i++;
	    }
	 }
	 free((char *)len);
	 return(i);
      }

      for(j = 0;j < nvec;j++) {
	 if(strncmp(val,vecs[j]->name,len[j]) == 0 &&
					    val[len[j]] == '\0') { /* got it */
	    if(len[j] != strlen(vecs[j]->name)) { /* an array element */
	       k = len[j] + 1;
	       ind = ind2 = atoi(&vecs[j]->name[k]);
	       for(;isdigit(vecs[j]->name[k]);k++) continue;
	       if(vecs[j]->name[k] == '-') {
		  ind2 = atoi(&vecs[j]->name[k + 1]);
	       }

	       if(ind < 0 || ind >= nel) {
		  msg_1s("Index specified by \"%s\" is out",vecs[j]->name);
		  msg_1d(" of range 0 -- %d\n",nel - 1);
		  len[j] = strlen(vecs[j]->name); /* this will stop future
						     matches */
		  continue;
	       } else if(ind2 >= nel) {
		  msg_1d("Upper index %d",ind2);
		  msg_1s(" requested for \"%s\" is out",vecs[j]->name);
		  msg_1d(" of range 0 -- %d\n",nel - 1);
		  ind2 = nel - 1;
	       }
	       strcpy(basename,vecs[j]->name);
	       basename[len[j]] = '\0';
	       sprintf(vecs[j]->name,"%s%d",basename,ind);
	       len[j] = strlen(vecs[j]->name);

	       for(k = 0;ind + 1 + k <= ind2;k++) { /* we want more elements */
		  if(jj + k >= nvec + nextra) { /* there's no space for it */
		     msg_1s("There is no slot allocated for %s",basename);
		     msg_1d("%d",ind + k + 1);
		     ind2 = ind + k;
		  } else {
		     sprintf(vecs[jj + k]->name,"%s%d",basename,ind + k + 1);
		     len[jj + k] = strlen(vecs[jj + k]->name);
		  }
	       }
	    } else {
	       ind = ind2 = 0;
	    }

	    if(coltype == HEAP) {
	       msg_1s("Unsupported format (heap) for column %s\n",val);
	       msg("I shall return the number of bytes not the value\n");

	       vecs[j]->type = VEC_FLOAT;
	       offset[j] = off + ind*colsize;
	       type[j] = LONG;
	       size[j] = colsize/2;	/* we only want the first half */
	    } else {
	       vecs[j]->type = (coltype == STRING) ? VEC_STRING : VEC_FLOAT;
	       offset[j] = off + ind*colsize;
	       type[j] = coltype;
	       size[j] = colsize;
	    }

	    for(k = 0;ind + 1 + k <= ind2;k++) { /* we want more elements */
	       vecs[jj + k]->type = vecs[j]->type;
	       offset[jj + k] = offset[j] + (k + 1)*colsize;
	       type[jj + k] = type[j];
	       size[jj + k] = size[j];
	    }

	    jj += (ind2 - ind);
	 }
      }

      off += nel*colsize;
   }

   free((char *)len);
/*
 * Count how many vectors we found
 */
   i = 0;
   for(j = 0;j < nvec + nextra;j++) {
      if(offset[j] >= 0) {
	 i++;
      }
   }
   if(i == nvec + nextra) {			/* all present and correct */
      return(nvec + nextra);
   }
/*
 * we didn't get them all. Maybe some are column numbers?
 */
   for(j = 0;j < nvec;j++) {
      if(offset[j] >= 0) {		/* is it an integer? */
	 for(i = 0;isdigit(vecs[j]->name[i]);i++) continue;
	 if(vecs[j]->name[i] != '\0') { /* not an integer */
	    continue;
	 }

	 if(parse_table_format(table_fmt,(VECTOR **)NULL,0,0,
			   atoi(vecs[j]->name),&offset[j],&size[j],&type[j])) {
	    vecs[j]->type = (type[j] == STRING ? VEC_STRING : VEC_FLOAT);
	 } else {
	    offset[j] = -1;		/* still not got it */
	 }
      }
   }
/*
 * Are any still missing?
 */
   i = 0;
   for(j = 0;j < nvec + nextra;j++) {
      if(offset[j] >= 0) {
	 i++;
      }
   }
   if(i == nvec + nextra) {		/* all present and correct */
      return(nvec + nextra);
   }
/*
 * Some are not here. Report the problem and prepare to process the good ones
 */
   nfound = i;
   msg_1d("I can only find %d",nfound);
   msg_1d(" of %d requested columns. Failed to locate column",nvec + nextra);
   msg_1s("%s:\n",nvec - nfound == 1 ? "" : "s");
   for(j = 0;j < nvec + nextra;j++) {
      if(offset[j] < 0 && *vecs[j]->descrip != '\0') {
	 msg_1s("   %s\n",vecs[j]->descrip);
      }
   }
/*
 * Move the valid vectors down to the start of the array
 */
   j = 0;
   for(i = 0;i < nvec + nextra;i++) {
      if(offset[i] >= 0) {
	 offset[j] = offset[i];
	 vecs[j] = vecs[i];
	 size[j] = size[i];
	 type[j] = type[i];
	 j++;

	 if(i > j) {
	    offset[i] = -1;
	 }
      }
   }

   return(nfound);
}

/***************************************************/
/*
 * Now routines to administer `filecap' - actually just interfaces
 * to the general *cap routines used primarily by graphcap.
 */

static TTY *file_desc = NULL;		/* descriptor for filecap */

static int
find_entry(name,file)
char name[],
     file[];
{
   if(file_desc != NULL) free((char *)file_desc);
   if((file_desc = ttyopen(file,1,&name,(int (*)())NULL)) == NULL) {
      msg("ttyopen returns NULL in find_entry()\n");
      return(-1);
   }
   tty_index_caps(file_desc,file_desc->t_capcode,file_desc->t_capindex);
   return(0);
}

static int
get_capint(cap)
char *cap;
{
   if(file_desc == NULL) return(0);     /* file not correctly opened */
   return(ttygeti(file_desc,cap));
}

/*
 * Does capability exist?
 */
static int
is_capval(cap)
char *cap;
{
   if(file_desc == NULL) return(0);     /* file not correctly opened */
   return(ttygetb(file_desc,cap));
}


static char *
get_capstr(cap)
char *cap;
{
   static char capstr[21];

   if(file_desc == NULL) return("");  /* file not correctly opened */
   if(ttygets(file_desc,cap,capstr,20) == 0) capstr[0] = '\0';
   return(capstr);
}

/*****************************************************************************/
/*
 * Read a FITS header, either an image or a table
 */
static int
read_fits_header(is_table,du,
		 record,nx,ny,dtype,dsize,bscale,bbzero,xmin,xmax,ymin,ymax)
int is_table;
int du;					/* data unit that we are reading */
char *record;
int *nx,*ny,                            /* size of file */
    *dtype,                             /* type of data */
    *dsize;                             /* length of data */
float *bscale,                          /* data = bbzero + I*bscale */
      *bbzero,
      *xmin,*xmax,                      /* range of values on the x axis */
      *ymin,*ymax;                      /* and on the y axis */
{
   if(is_table) {
      *xmin = *xmax = *ymin = *ymax = 0.0;
      return(read_fits_table_header(du,record,nx,ny,dsize));
   } else {
      return(read_fits_image_header(du,record,nx,ny,dtype,dsize,
				    bscale,bbzero,xmin,xmax,ymin,ymax));
   }
}

static int
read_fits_image_header(du,
		 record,nx,ny,dtype,dsize,bscale,bbzero,xmin,xmax,ymin,ymax)
int du;					/* data unit that we are reading */
char *record;
int *nx,*ny,                            /* size of file */
    *dtype,                             /* type of data */
    *dsize;                             /* length of data */
float *bscale,                          /* data = bbzero + I*bscale */
      *bbzero,
      *xmin,*xmax,                      /* range of values on the x axis */
      *ymin,*ymax;                      /* and on the y axis */
{
   char buff[81],                       /* for reading value (81 plays safe) */
	*keyword,                       /* keyword from a FITS card */
	*value;                         /* value of a FITS keyword */
   int i,j,
       naxis = 0;                       /* number of axes in data */

   i = 0;
   if(*nx < 0) {                       /* first record of header */
      if(read_card(record,&keyword,&value) < 0) {
	 return(-1);
      }
      if(strcmp(keyword,"SIMPLE") != 0) {
	 msg_1s("First keyword on FITS header is %s not SIMPLE\n",keyword);
	 return(-1);
      }
      (void)str_scanf(value,"%s",buff);
      if(!strcmp(buff,"T")) {           /* OK, simple FITS. Good */
	 *dtype = INT;                  /* pending value of BITPIX */
      } else if(!strcmp(buff,"F")) {    /* Assume it is floating */
	 if(sm_verbose) {
	    msg("FITS file isn't simple, assuming `float'\n");
	 }
	 *dtype = FLOAT;
      } else {
	 msg_1s("Illegal value for keyword SIMPLE : %s\n",value);
	 return(-1);
      }
      i++; record += 80;

      if(read_card(record,&keyword,&value) < 0) {
	 return(-1);
      }
      if(strcmp(keyword,"BITPIX") != 0) {
	 msg("Second keyword on FITS header isn't BITPIX\n");
	 return(-1);
      }
      *dsize = atoi(value)/BITS_IN_CHAR;
      if(*dsize < 0) {			/* floating IEEE */
	 *dsize = -(*dsize);
	 *dtype = FLOAT;
      }
      if(*dtype == FLOAT) {
	 if(*dsize != 4) {
	    msg_1d("Illegal value of BITPIX for floating data: %d\n",
								atoi(value));
	    return(-1);
	 }
      } else if(*dsize == 1) {
	 *dtype = CHAR;
      } else if(*dsize == 2) {
	 *dtype = SHORT;
      } else if(*dsize == 4) {
	 *dtype = LONG;
      } else {
	 msg_1d("Illegal value of BITPIX: %d\n",atoi(value));
	 return(-1);
      }
      i++; record += 80;

      if(read_card(record,&keyword,&value) < 0) {
	 return(-1);
      }
      if(strcmp(keyword,"NAXIS") != 0) {
	 msg("Third keyword on FITS header isn't NAXIS\n");
	 return(-1);
      }
      naxis = atoi(value);
      i++; record += 80;

      if(read_card(record,&keyword,&value) < 0) {
	 return(-1);
      }
      if(strcmp(keyword,"NAXIS1") != 0) {
	 msg("Fourth keyword on FITS header isn't NAXIS1\n");
	 return(-1);
      }
      *nx = atoi(value);
      i++; record += 80;

      if(naxis == 1) {
	 *ny = 1;
      } else {
	 if(read_card(record,&keyword,&value) < 0) {
	    return(-1);
	 }
	 if(strcmp(keyword,"NAXIS2") != 0) {
	    msg("Fifth keyword on FITS header isn't NAXIS2\n");
	    return(-1);
	 }
	 *ny = atoi(value);
	 i++; record += 80;
      }
   }
/*
 * We've read the compulsary header, now look for END
 */
   for(;i < 36;i++) {
      if(read_card(record,&keyword,&value) < 0) {
	 continue;			/* try to recover */
      } else if(!strcmp(keyword,"END")) {
	 break;
      }
      record += 80;
   }

   if(naxis > 2) {
      for(j = 3;j < naxis;j++) {
	 sprintf(buff,"NAXIS%d",j);
	 if(*(value = get_keyword(buff)) != '\0' && atoi(value) != 1) {
	    msg_1d("NAXIS isn't 1 or 2 (it's %d), ",naxis);
	    msg_1d("NAXIS%d = ",j);
	    msg_1d("%d, so data isn't 2-dimensional.\n",atoi(value));
	    msg_2s("%s%s\n",
		"I'll ignore the extra dimensions, ",
		"and pretend that NAXIS is 2.");
	 }
      }
   }
   if(*(value = get_keyword("BSCALE")) != '\0') *bscale = atof2(value);
   if(*(value = get_keyword("BZERO")) != '\0') *bbzero = atof2(value);

   *xmin = 0;
   *xmax = *nx - 1;
   if(*get_keyword("X0") != '\0' && *get_keyword("X1") != '\0') {
      *xmin = atof2(get_keyword("X0"));
      *xmax = atof2(get_keyword("X1"));
   } else {
      float crval,crpix,cdelt;
      char *val1,*val2,*val3;
      
      val1 = get_keyword("CRPIX1");
      val2 = get_keyword("CRVAL1");
      val3 = get_keyword("CDELT1");

      if(*val1 != '\0' && *val2 != '\0' && *val3 != '\0') {
	 if(*(value = get_keyword("CROTA1")) != '\0' && atof2(value) != 0) {
	    msg_1f("CROTA1 = %f in non-zero; I'm it ignoring\n",atof2(value));
	 } else {
	    crpix = atof2(val1);
	    crval = atof2(val2);
	    cdelt = atof2(val3);
	    *xmin = crval + (1 - crpix)*cdelt;
	    *xmax = crval + (*nx - crpix)*cdelt;
	    (void)sprintf(buff,"%g",*xmin); set_image_variable("X0",buff);
	    (void)sprintf(buff,"%g",*xmax); set_image_variable("X1",buff);
	 }
      }
   }

   *ymin = 0;
   *ymax = *ny - 1;
   if(*get_keyword("Y0") != '\0' && *get_keyword("Y1") != '\0') {
      *ymin = atof2(get_keyword("Y0"));
      *ymax = atof2(get_keyword("Y1"));
   } else {
      float crval,crpix,cdelt;
      char *val1,*val2,*val3;
      
      val1 = get_keyword("CRPIX2");
      val2 = get_keyword("CRVAL2");
      val3 = get_keyword("CDELT2");
      if(*val1 != '\0' && *val2 != '\0' && *val3 != '\0') {
	 if(*(value = get_keyword("CROTA2")) != '\0' && atof2(value) != 0) {
	    msg_1f("CROTA2 = %f in non-zero; I'm it ignoring\n",atof2(value));
	 } else {
	    crpix = atof2(val1);
	    crval = atof2(val2);
	    cdelt = atof2(val3);
	    *ymin = crval + (1 - crpix)*cdelt;
	    *ymax = crval + (*ny - crpix)*cdelt;
	    (void)sprintf(buff,"%g",*ymin); set_image_variable("Y0",buff);
	    (void)sprintf(buff,"%g",*ymax); set_image_variable("Y1",buff);
	 }
      }
   }

   return(i == 36 ? 1 : 0);
}

static int
read_fits_table_header(du,record,nx,ny,dsize)
int du;					/* data unit that we are reading */
char *record;
int *nx,*ny;                            /* size of file */
int *dsize;				/* BITPIX/8 == 1 */
{
   char buff[81],                       /* for reading value (81 plays safe) */
	*keyword,                       /* keyword from a FITS card */
	*value;                         /* value of a FITS keyword */
   int i,j,
       naxis = 0;                       /* number of axes in data */

   i = 0;
   if(*nx < 0) {			/* first record of header */
      if(read_card(record,&keyword,&value) < 0) {
	 return(-1);
      }

      if(du == 0) {
	 if(strcmp(keyword,"SIMPLE") != 0) {
	    msg_1s("First keyword on FITS header is %s not SIMPLE\n",
		   keyword);
	    return(-1);
	 }
	 (void)str_scanf(value,"%s",buff);
	 if(strcmp(buff,"T") != 0) {
	    msg("Fits binary tables should have a PDU with SIMPLE=T, ");
	    msg_1s("not %s, but I'll continue\n",buff);
	 }
      } else {
	 if(strcmp(keyword,"XTENSION") != 0) {
	    msg_1s("First keyword on FITS header is %s not XTENSION\n",
		   keyword);
	    return(-1);
	 }
	 if(strcmp(value,"BINTABLE") != 0) {
	    msg_1s("Value of first keyword is %s not BINTABLE\n",value);
	    return(-1);
	 }
      }
      i++; record += 80;

      if(read_card(record,&keyword,&value) < 0) {
	 return(-1);
      }
      if(strcmp(keyword,"BITPIX") != 0) {
	 msg("Second keyword on FITS header isn't BITPIX\n");
	 return(-1);
      }
      if(atoi(value) != 8) {
	 msg_1d("Illegal value of BITPIX for a BINTABLE: %d\n",atoi(value));
	 return(-1);
      }
      *dsize = 1;
      i++; record += 80;

      if(read_card(record,&keyword,&value) < 0) {
	 return(-1);
      }
      if(strcmp(keyword,"NAXIS") != 0) {
	 msg("Third keyword on FITS header isn't NAXIS\n");
	 return(-1);
      }
      naxis = atoi(value);
      if(du == 0 && naxis != 0) {
	 msg_1d("Illegal value of NAXIS for PDU of a BINTABLE: %d",
								  atoi(value));
	 msg(" (must be 0)\n");
	 return(-1);
      }
      *nx = *ny = 0;
      i++; record += 80;

      if(du == 0) {
	 if(read_card(record,&keyword,&value) < 0) {
	    return(-1);
	 }
	 if(strcmp(keyword,"EXTEND") != 0) {
	    msg("Fourth keyword on FITS header isn't EXTEND\n");
	    return(-1);
	 }
	 if(strcmp(value,"T") != 0) {
	    msg_1s("Value of EXTEND is %s not T\n",value);
	    return(-1);
	 }
	 i++; record += 80;
      } else {
	 if(read_card(record,&keyword,&value) < 0) {
	    return(-1);
	 }
	 if(strcmp(keyword,"NAXIS1") != 0) {
	    msg("Fourth keyword on FITS header isn't NAXIS1\n");
	    return(-1);
	 }
	 *nx = atoi(value);
	 i++; record += 80;
	 
	 if(naxis == 1) {
	    *ny = 1;
	 } else {
	    if(read_card(record,&keyword,&value) < 0) {
	       return(-1);
	    }
	    if(strcmp(keyword,"NAXIS2") != 0) {
	       msg("Fifth keyword on FITS header isn't NAXIS2\n");
	       return(-1);
	    }
	    *ny = atoi(value);
	    i++; record += 80;
	 }
      }
   }
/*
 * We've read the compulsary header, now look for END
 */
   for(;i < 36;i++) {
      if(read_card(record,&keyword,&value) < 0) {
	 ;
      } else if(!strcmp(keyword,"END")) {
	 break;
      }
      record += 80;
   }

   if(naxis > 2) {
      for(j = 3;j < naxis;j++) {
	 sprintf(buff,"NAXIS%d",j);
	 if(*(value = get_keyword(buff)) != '\0' && atoi(value) != 1) {
	    msg_1d("NAXIS isn't 1 or 2 (it's %d), ",naxis);
	    msg_1d("NAXIS%d = ",j);
	    msg_1d("%d, so data isn't 2-dimensional.\n",atoi(value));
	    msg_2s("%s%s\n",
		"I'll ignore the extra dimensions, ",
		"and pretend that NAXIS is 2.");
	 }
      }
   }

   return(i == 36 ? 1 : 0);
}

static int
read_card(card,keyword,value)
char *card,
     **keyword,
     **value;
{
   int i;

   *keyword = &card[0];

   if(!(!strncmp(*keyword,"COMMENT",7) || !strncmp(*keyword,"END",3) ||
	!strncmp(*keyword,"HISTORY",7) || !strncmp(*keyword,"        ",8))) {
      if(card[8] != '=') {
	 (*keyword)[8] = '\0';
	 if(sm_verbose) {
	    msg_1s("Missing = for keyword %s\n",*keyword);
	 }
      }
   }

   (*keyword)[8] = '\0';
   for(i = 7;i >= 0;i--) {
      if((*keyword)[i] == ' ') (*keyword)[i] = '\0';
   }
   if((*keyword)[0] == '\0') {
      if(sm_verbose > 0) {
	 msg("Card in FITS header has no keyword\n");
      }
      return(-1);
   }

   for(i = 10;i < 80 && isspace(card[i]);i++) continue; /* skip leading space*/
   if(card[i] == '\'') {		/* character value */
      *value = &card[++i];
      while(i < 80 && card[i] != '\'') i++;
   } else {				/* non-character value */
      *value = &card[i];
      while(i < 80 && !isspace(card[i]) && card[i] != '/') i++;
   }
   if(i >= 80) i = 79;
   card[i] = '\0';

   set_image_variable(*keyword,*value);
   return(0);
}

/*
 * Byte swap ABABAB -> BABABAB in place
 */
static void
swap_2(arr,n)
char *arr;                              /* array to swap */
int n;                                  /* number of bytes */
{
   char *end,
	t;

   if(n%2 != 0) {
      msg_1d("Attempt to byte swap odd number of bytes: %d\n",n);
      n = 2*(int)(n/2);
   }

   for(end = arr + n;arr < end;arr += 2) {
      t = arr[0];
      arr[0] = arr[1];
      arr[1] = t;
   }
}

/*
 * Byte swap ABCDABCD -> DCBADCBA in place (e.g. sun <--> vax)
 */
static void
swap_4(arr,n)
char *arr;                              /* array to swap */
int n;                                  /* number of bytes */
{
   char *end,
	t;

   if(n%4 != 0) {
      msg_1d("Attempt to byte swap non-multiple of 4 bytes: %d\n",n);
      n = 4*(int)(n/4);
   }

   for(end = arr + n;arr < end;arr += 4) {
      t = arr[0];
      arr[0] = arr[3];
      arr[3] = t;
      t = arr[1];
      arr[1] = arr[2];
      arr[2] = t;
   }
}

/***************************************************************/
/*
 * Stuff to handle the tree for keywords
 */
#define VALSIZE 40
typedef struct {
   char value[VALSIZE + 1];
} KNODE;

static char *kvalue;
static void kkill();
static Void *kmake();
static TREE kk = { NULL, kkill, kmake};

/*
 * Add a keyword to the tree
 */
void
set_image_variable(name,value)
char *name;
char *value;
{
   char fname[101];			/* name with '-' converted to '_' */
   char *ptr;

   kvalue = value;

   if(strchr(name,'-') != NULL) {
      strncpy(fname,name,100); fname[100] = '\0';
      ptr = fname;
      while((ptr = strchr(ptr,'-')) != NULL) {
	 *ptr = '_';
      }
      name = fname;
   }

   insert_node(name,&kk);
}

/*
 * return value of keyword
 */
static char *
get_keyword(name)
char *name;
{
   KNODE *node;

   if((node = (KNODE *)get_rest(name,&kk)) == NULL) {/* unknown keyword */
      return("");
   } else {
      int len = strlen(node->value) - 1;
      while(len >= 0 && isspace(node->value[len])) {
	 len--;
      }
      node->value[len + 1] = '\0';	/* trim trailing spaces */
      return(node->value);
   }
}

/*
 * Forget all keywords
 */
void
kill_keywords()
{
   kill_tree(&kk);
}

static void
kkill(nn)
Void *nn;
{
   free((char *)nn);
}

/*
 * make new node
 */
static Void *
kmake(name,nn)
char *name;
Void *nn;			/* current value of node */
{
   KNODE *node;
   
   if(nn == NULL) {
      if((node = (KNODE *)malloc(sizeof(KNODE))) == NULL) {
	 msg_1s("malloc returns NULL in vmake() for %s\n",name);
	 return(NULL);
      }
      node->value[VALSIZE] = '\0';		/* Check value is terminated */
   } else {
      node = (KNODE *)nn;
   }
   (void)strncpy(node->value,kvalue,VALSIZE);

   return((Void *)node);
}
