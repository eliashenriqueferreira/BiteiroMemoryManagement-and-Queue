#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BMM_ALLOC_KEY	's'
#define BMM_ALLOC_TAIL	't'

typedef struct st_bitMM {
	struct st_bitMM *pnext;
	struct st_bitMM *pprev;
	char status_key;
	size_t bufLen;
	char *pbuff;
} bitMM;

typedef struct st_bitMM_Statistics {
	size_t allocs;
	size_t frees;
	size_t total_mem;
} bitMMSTS;


#if defined BITEIRO_MM_C
#define EXTERN
#elif defined CPP_CODE
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

EXTERN bitMM *bitMM_ctor();
EXTERN char *bitMM_alloc(size_t len);
EXTERN int bitMM_free(char *);
EXTERN int bitMM_dest(bitMM *);
EXTERN int testBiteiroMemoryManagement(int argc, char ** argv);
