/**********************************************************************//**
***************************************************************************
*** @file   vctypes.h
***
*** @brief  Main VC Types Header File
***
*** @author  Copyright (c) 2004-2014 Vision Components
*** @author  All rights reserved.
*** @author  This software embodies materials and concepts which are
***          confidential to Vision Components.
***
***
*** @revisionHistory
***
***    Date          Version    Author  Changes
***    Apr 11, 2014  3.13       MBE     Initial Version (from VCRT)      
***
***
*** @endRevisionHistory
***************************************************************************
***************************************************************************/
#ifndef _VCTYPES_h
#define _VCTYPES_h


  typedef unsigned char  uint_08;
  typedef unsigned short uint_16;
  typedef unsigned int   uint_32;
  typedef int            int_32;
  typedef short          int_16;
  typedef void *         pointer;

  typedef unsigned char   U8;
  typedef unsigned short U16;
  typedef unsigned int   U32;
  typedef signed   char   I8;
  typedef short          I16;
  typedef int            I32;
  typedef float          F32;
  typedef double         F64;
  
  #if defined __GNUC__
    #define VC_BASICTYPES
    typedef signed   long long I40;
    typedef unsigned long long U40;
    typedef unsigned long long U64;
    typedef signed   long long I64;
    #define   VCRT_NO_COMPAT
    #define  _nassert
    #define  _abs            abs
  #endif
  
  #if defined __TI_COMPILER_VERSION__
    #define VC_BASICTYPES
    typedef long           I40;
    typedef unsigned long  U40;
    #if __TI_COMPILER_VERSION__ >= 5000000
      typedef unsigned long long U64;
      typedef   signed long long I64;
    #endif
  #endif
  
  #ifndef VC_BASICTYPES
    // Microsoft Visual C++ 6.0 definitions
    #define VC_BASICTYPES
    typedef unsigned __int64 U40;
    typedef          __int64 I40;
    typedef unsigned __int64 U64;
    typedef          __int64 I64;
    #define   VCRT_NO_COMPAT
    #define  _nassert
    #define  _abs            abs
  #endif
    

  #ifdef  NULL
     #undef  NULL
  #endif
  #ifdef __cplusplus
     #define NULL (0)
  #else
     #define NULL ((pointer)0)
  #endif



#ifdef __cplusplus
   extern "C" {
#endif
    
  void *sysmalloc(unsigned nwords, int type);
  int   sysfree(void *ap);
  //#define vcsetup()
  #ifndef MDATA
    #define     MDATA       2
  #endif
  #define vcmalloc(x)     sysmalloc((x),MDATA)
  #define vcfree(x)       sysfree((x))
  //#define prtfree()       sysprtfree(MDATA)
  #define byte_malloc(x)  sysmalloc(((x)+3)/4,MDATA)
  #define byte_free(x)    sysfree((x))
  
  #if defined __TI_COMPILER_VERSION__
    #define  getchar    rs232rcv
    #define  putchar    rs232snd
  
    #define  printf     print
  
    void  pstr(char *);
    void _pstr(char *);
    void  print(char *, ...);
    void _print(char *, ...);
    
    #define  sprintf    io_sprintf
    #define  sprint     io_sprintf
    
    void  sprint(char *s, char *pfmt,...);
    void  io_sprintf(char *s, char *pfmt,...);
    #define  fprintf    io_fprintf
    #define  fscanf     io_fscanf
  #else
    #define  print      printf
    #define  sprint     sprintf
    #define  pstr       printf
    #define  rs232rcv   getchar
    #define  rs232snd   putchar
  #endif
    
#ifdef __cplusplus
}
#endif
    




#endif

/* EOF */
