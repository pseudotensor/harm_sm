/*
 * This file is used to specify user options for SM.
 * It should be included by all files that are to be linked
 * into SM, including device drivers
 *
 * You may well be able to use the script set_opts in the directory
 * above this file to set your compilation environment automatically,
 * and never edit this file be hand.
 *
 * After editing this file, you may also have to fiddle with the
 * Makefiles (e.g., does your system understand ranlib?)
 *
 * (If you don't know C, stuff like this from slash-star to star-slash
 * is a comment. A line starting in the first column like
#define ABC xyz
 * says to define a symbol ABC to have the value xyz. Inside a comment
 * it doesn't do anything, so to activate a definition move it outside
 * the comment, i.e. swap it with the following star-slash line. Comments
 * don't nest, by the way, so that's why I'm writing star-slash)
 *
 * First which device drivers:
 */

/*
 * BGI graphics on a PC (device bgi)
 * You must have the Borland C-compiler version 3.0 or later to use this driver
#define BGI
 */

/*
 * Imagen Printer - IMPRESS language (device imagen)
#define IMAGEN
 */

/*
 * Microsoft Windows (device msw)
 * You must have the Borland C-compiler version 3.0 or later to use this driver
#define MSWINDOWS
 */

/*
 * IBM OS/2 v2.xx
 * You must have Borland's C++ compiler for OS/2 V1.5 to use this device
 #define OS2PM
 */

/*
 * Silicon Graphics GL driver (device sgi)
 * You should add $(SGILIB) to $(LDFLAGS) in src/Makefile
#define SGI
 */

/*
 * SunView driver (device sunview)
 * You should add $(SUNVLIB) to $(LDFLAGS) in src/Makefile
#define SUN_VIEW
 */

/*
 * sunwindowsdriver (device sunwindows)
 * This is obselete -- use sunview instead
 * You should add $(SUNLIB) to $(LDFLAGS) in src/Makefile
#define SUNWINDOWS
 */

/*
 * linux svgalib driver
#define SVGALIB
 */

/*
 * X-windows V10 driver (device xwindows)
 * If you are using X11 DON'T define this!
 * You should add $(XLIB) to $(LDFLAGS) in src/Makefile
#define XWINDOWS
 */

/*
 * X-windows V11 driver (device x11)
 * You should add $(XLIB11) to $(LDFLAGS) in src/Makefile
 */
#define X11

/*
 * Driver for a Unix PC (a 3b1)
#define UNIXPC
 */

/* 
 * Driver for VAX workstations using UIS calls.  If you define this device, 
 * you will need to edit sm.opt in the [.src] subdirectory to
 * include the vaxuis library, i.e. add the line
 *	sys$library:uisxshr/share
 * before the line
 *	library:decw$dwtlibshr/share
 *
#define VAXUIS 
 */

/*
 * If you want to use backing store even when SM doesn't think that
 * you need to, define this symbol. This is typically used if you
 * have an application that calls SM libraries and also does a lot
 * of calculations, and wants the graphics to remain updated.
#define FORCE_BACKING_STORE
 */

/*
 * MACROSIZE is the length of the largest macro that can be read from a file
 */
#define MACROSIZE 99999

/*
 * VEC_STR_SIZE is the length of each element of a string-valued vector
 * (they are all the same size at present).
 *
 * Making this very large will waste memory
 */
#define VEC_STR_SIZE 300

/*
 * REAL is the data type used for arithmetic vectors (but not images; they
 * are always internally represented as floats)
 */
typedef double REAL;


/*
 * You may well not have to edit this file further; what we believe to
 * be correct settings for
 *	alliant borland-C hp linux rs6000 sgi sun (including solaris)
 *      ultrix vms
 * are set automatically.
 */

/*
 * Now choose your operating system: choose no more than one of the following:
 */
/*
 * If you are running on a machine running System V Unix, define this
#define SYS_V
 */
/*
 * If you are running VMS define this (it's probably done automatically)
#define VMS
 */
#if defined(vms) && !defined(VMS)
#  define VMS
#endif
/*
 * If you're running this on an IBM PC,
 * __MSDOS__ and maybe __BORLANDC__ will be defined (and used)
 */

/*
 * Set some options to do with compiling SM:
 */
/*
 * If your compiler doesn't support alloca() define this:
 * (SYS V, VMS, and RISC/6000 compilers don't, but we deal with that for you)
#define NO_ALLOCA
 */

/*
 * If your compiler doesn't support ftruncate() define this:
 * (SYS V and VMS compiler don't, but we deal with that for you)
#define NO_FTRUNCATE
 */

/*
 * If your compiler doesn't support memcpy() define this:
#define NO_MEMCPY
 */

/*
 * If your compiler doesn't support system() define this:
 * (VMS doesn't, but we deal with that for you)
#define NO_SYSTEM
 */

/*
 * If your compiler doesn't support select() define this:
 * (VMS doesn't, but we deal with that for you)
#define NO_SELECT
 */

/*
 * If you have the readdir() system call define this; should be OK
 * for most BSD unix systems but not VMS or System V (we deal with VMS
 * and SysV for you)
 *
 * SM will compile without this, but APROPOS won't be able to search
 * help files
 */
#if !defined(VMS) && !defined(SYS_V)
#  define HAVE_READDIR
#endif

/*
 * The old option RENAME has been removed. As of version 2.3.0, all
 * user callable SM functions begin sm_ (e.g. sm_line).
 */

/*
 * Set the debug level for compiling. Most users should not define this,
 * as it can slow things down. On the other hand, if you are debugging
 * you might try setting it. For example, on a sun DEBUG==2 checks the
 * heap on each transaction
#define DEBUG 2
 */

/*
 * Did an exclamation mark (!) introduce a comment in Mongo macros?
 * (used by READ OLD)
#define EXCLAMATION_IS_COMMENT 1
 */

/*
 * This is the log_10 of the largest floating point number that you
 * can handle (or at least, it is no larger than that). It's used to
 * prevent certain overflows.
 *			 (MUST BE DEFINED)
 */
#define MAX_POW 308

/*
 * If FITS files must have their byte order swapped, define this
 * (e.g. on a vax, but we do it for you)
#define NEED_SWAP
 */

/*
 * Different compilers treat C and fortran external symbols differently.
 * For unix systems, the main variants are for fortran externs to have
 * an extra trailing _, and for the fortran and C names to be identical.
 * In the latter case we have to call our fortran-callable versions things
 * like fsm_box; this is achieved by defining these symbols. If neither
 * is defined, we'll define FORTRAN_APPEND as _.
#define FORTRAN_APPEND _
#define FORTRAN_PREPEND f
 */


/*
 * Now for Posix compliance. Posix is an ANSI standard for a unix-like
 * O/S and compilers that think that they are supporting it sometimes
 * argue with SM. If this is a problem define this symbol. We do it
 * for you for IBM RISC/6000 machines.
#define _POSIX_SOURCE 1
 */

/*
 * The following options are to do with ANSI compilance. If you have a
 * fully ANSI C compiler, you can skip the rest of the file (as we'll
 * set the correct values if __STDC__ is defined as 1). If you don't have
 * an ANSI compiler, you may need to change some of the values following
 * the "#else" line.  At the moment the only vms compiler that thinks it is
 * ANSI compliant, namely gcc, isn't, so we are punting for the nonce
 */
#if __STDC__				/* Hurrah! ANSI */
#  define ANSI_CPP			/* preprocessor is ANSI */
#  if !defined(__GNUC__)
#     define ANSI_INCLUDE		/* has the ANSI include files */
#  endif
#  define ANSI_PROTO			/* supports prototypes */
#  define STDARG			/* supports STDARG */
#  define Const const
#  define Signed signed
#  define Void void			/* understands (void *) */
#  define Volatile volatile
/*
 * Many implementations of sscanf don't work with read-only strings,
 * and ANSI compilers are free to put string constants into read-only
 * memory. For ANSI compliant compilers we have provided a function
 * str_scanf that does. If you want to use the system sscanf anyway,
 * define str_scanf to be sscanf (this is done automatically for
 * non-ANSI compilers)
#  define str_scanf sscanf
 */
#else					/* Not ANSI */
#define str_scanf sscanf
/*
 * If your compiler doesn't support memcpy() define this:
 * (The VMS compiler does)
#define NO_MEMCPY
 */

/*
 * If you have an ANSI compiler, it'll want to use ## to concatenate
 * preprocessor tokens. If it does, define this: (must do this for VMS gcc)
#define ANSI_CPP
 */

/*
 * If your compiler has ANSI-compliant system headers you can define this,
 * it'll stop SM from trying to define thigs like strlen() itself
#define ANSI_INCLUDE
 */

/*
 * If your compiler supports function prototypes you can define this.
 * It won't make any difference to running SM whatever you do,
 * but it matters to the authors.
#define ANSI_PROTO
 */

/*
 * If your linker can't find the function strchr(), define this:
 * (not needed for VMS or Unix System V). Similarily for strrchr()
#define strchr index
#define strrchr rindex
 */

/*
 * Some compilers break with functions returning (void *). We get around this
 * by defining a type Void. It's safe to define this to be char, but
 * it'd be better to use void
 *			 (MUST BE DEFINED)
 */
#define Void char

/*
 * If your compiler knows the keywords "const", "signed", or "volatile",
 * define the Capitalised form here as the all-lowercase equivalent.
 * This probably doesn't matter much -- but they must be defined.
#define Const const
#define Signed signed
#define Volatile volatile
 */
#define Const
#define Signed
#define Volatile

#endif					/* __STDC__ */

/******************************************************************/
/*
 * Now deal with those operating system decisions: Users should not
 * have to edit this section
 *
 * Some fiddling to make control.y acceptable to unix boxes and PCs
 */
#if defined(__BORLANDC__) || defined(_Windows)
#  define Static
#  define Far far
#else
#  define Static static
#  define Far
#endif

/*
 * The values to return to the operating system. For all Unix systems
 * you want 0 and 1 respectively; VMS wants them reversed
 */
#define EXIT_OK 0
#define EXIT_BAD 1

#if defined(alliant)
#  undef HAVE_READDIR
#  undef MAX_POW
#  define MAX_POW 308
#endif

#if defined(alpha)
#  define unix
#  define _OSF_SOURCE
#  define _POSIX_SOURCE
#  define HAVE_IOCTL_DEFN
#  define FD_SET_H <sys/types.h>
#  define SELECT_H <sys/select.h>
#endif

#if defined(__BORLANDC__)
#  define NO_FTRUNCATE
#endif

#if !defined(STANDALONE)		/* not something like makeyyl */
#   if defined(BGI) && !(!defined(_Windows) && defined(__BORLANDC__))
      #error You have specified the BGI driver, but are not using \
  the borland compiler, or you are using windows
#   endif
#   if defined(MSWINDOWS) && !(defined(_Windows) && defined(__BORLANDC__))
      #error You have specified the MS-Windows driver, but are not using \
  the borland compiler with windows
#   endif
#endif

#if defined(hp)
#  if !defined(unix)
#     define unix
#  endif
#  define HPUX_SOURCE
#  define _HPUX_SOURCE
#  define SYS_V
#  define _POSIX_SOURCE 1
#  define HAVE_IOCTL_DEFN
#  define FORTRAN_PREPEND f
#  define FD_SET_H <sys/types.h>
#  define SELECT_H <sys/time.h>
#endif

#if defined(linux)
#  if !defined(unix)
#     define unix
#  endif
#  define _POSIX_SOURCE 1
#  define FORTRAN_APPEND __
#  define HAVE_IOCTL_DEFN
#  define FD_SET_H <sys/time.h>
#  define LOCAL_ECVT
#endif

#if defined(FreeBSD)
#  if !defined(unix)
#     define unix
#  endif
#  define ANSI_INCLUDE
#  define POSIX_INCLUDE
#  define FORTRAN_APPEND __
#  define HAVE_IOCTL_DEFN
#  define FD_SET_H <sys/types.h>
#  define LOCAL_ECVT
#endif

#if defined(rs6000)
#  if !defined(unix)
#     define unix
#  endif
#  define NO_ALLOCA
#  define HAVE_IOCTL_DEFN
#  define _POSIX_SOURCE 1
#  define _ALL_SOURCE   1
#  define FD_SET_H <sys/select.h>
#  define FORTRAN_PREPEND f
#endif

#if defined(sgi)
#  if !defined(unix)
#     define unix
#  endif
#  define ANSI_INCLUDE
#  if defined(sgi4)
#     define FD_SET_H <sys/types.h>
#  else
#     define FD_SET_H <sys/select.h>
#     define SELECT_H <sys/select.h>
#  endif
#  define SYS_V
#  define STDARG
#  if !defined(sgi4)
#     define HAVE_IOCTL_DEFN
#  endif
#endif

#if defined(sun)
#  if !defined(unix)
#     define unix
#  endif
#  if defined(solaris)
#     define ANSI_CPP
#     define ANSI_INCLUDE
#     define POSIX_INCLUDE
#     define POSIX_TTY
#     define TIMEVAL_H <sys/time.h>
#     define FD_SET_H <sys/select.h>
#     define SYS_V
#     define HAVE_IOCTL_DEFN
#  else
#     define SUN_BROKEN_DEFNS		/* this is terrible! It's due to the
					   Sun OS 4.1.1 headers being broken;
					   they conflict with ... prototypes */
#     if (defined(__STDC__) && __STDC__ == 0) /* probably acc */
#        define ANSI_CPP
#        define ANSI_INCLUDE
#        define NO_ALLOCA
#     endif
#     define FD_SET_H <sys/time.h>
#     define TIMEVAL_H <sys/time.h>
#     if !defined(__GNUC__)
#        define HAVE_IOCTL_DEFN
#     endif
#  endif
#endif

#if defined(SYS_V)
#  if !defined(__GNUC__)
#     define NO_ALLOCA
#  endif
#  define NO_FTRUNCATE
#endif

#if defined(VMS) 
#  define ANSI_INCLUDE
#  undef EXIT_OK
#  undef EXIT_BAD
#  define EXIT_OK 1
#  define EXIT_BAD 0
#  define NO_ALLOCA
#  define NO_FTRUNCATE
#  define NO_SELECT
#  define NO_SYSTEM
#  define pow vms_pow			/* theirs is/was broken */
#endif

#if defined(ultrix) 
#  define ANSI_INCLUDE
#endif

#if defined(_POSIX_SOURCE)
#  define ANSI_INCLUDE
#  define HAVE_READDIR
#  define POSIX_TTY
#  define POSIX_INCLUDE
#endif

#if defined(alpha) || defined(vax) || defined(linux) 
#  define NEED_SWAP
#endif
/*
 * Deal with Fortran--C name differences; the default is to add an _
 */
#if !defined(FORTRAN_PREPEND) && !defined(FORTRAN_APPEND)
#  define FORTRAN_APPEND _
#endif

/*
 * Implement ANSI_CPP
 */
#if defined(ANSI_CPP)
#  define _CONCATENATE(P,F) P ## F
#  define CONCATENATE(P,F) _CONCATENATE(P,F)
#else
#  define CONCATENATE(P,F) P/**/F
#endif

/*
 * Some DOS systems require an O_BINARY flag to open() (which isn't POSIX),
 * so define it if doesn't exist
 */
#if !defined(O_BINARY)
#  define O_BINARY 0
#endif

/*
 * Include file book-keeping:
 */
#if !defined(INC_OPTIONS_H)
#  define INC_OPTIONS_H
#endif
