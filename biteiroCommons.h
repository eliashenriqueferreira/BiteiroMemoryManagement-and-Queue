#pragma once

#define TRUE 	1
#define FALSE 	0

#define MATCH(s1, s2) (strcmp(s1, s2) == 0)
#define ABS(x)        ((x) < 0) ? (-(x)) : ((x))
#define MENOR(x,y)    ((x) < (y))?(x):(y)
#define MAIOR(x,y)    ((x) > (y))?(x):(y)
#define MIN(x,y)	MENOR(x,y)
#define MAX(x,y)    MAIOR(x,y)

inline void SWAP_LONG(long *pa, long *pb)
{
	*pa = *pa ^ *pb;
	*pb = *pa ^ *pb;
	*pa = *pa ^ *pb;
}
inline void SWAP_SIZE_T(size_t *pa, size_t *pb)
{
	*pa = *pa ^ *pb;
	*pb = *pa ^ *pb;
	*pa = *pa ^ *pb;
}


#if defined (_MSC_VER)
// code specific to Visual Studio compiler
#ifdef __cplusplus
	#pragma message("===================================== C++ compiler ===================================================")
#else
#ifdef __STDC__
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(__GNUC__) && !defined(__STRICT_ANSI__))
	#pragma message("================== ANSI X3.159-1999 / ISO 9899:1999 (C99) compiler ===================================")
#else
	#pragma message("============= ANSI X3.159-1989 (C89) / ISO/IEC 9899:1990 (C90) C compiler ============================")
	/* __STDC__ Defined as 1 only when compiled as C and if the /Za compiler option is specified. Otherwise, undefined. */
	/* Configurations Properties --> C/C++ --> Language --> Disable Language Extensions */
#endif
#else
	//#pragma message("============================= Pre-ANSI (K&R) C compiler ==============================================")
	#pragma message("==================== Enable Language Extensions for ANSI C compiler ==================================")
#endif
typedef unsigned char bool;
#endif
#endif





