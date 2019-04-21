#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SANNITY_CHECK_VALUE	0X5A6596A5
#define MAX_BUFFER_QUEUE_LENGHT		1000
#define MAX_BUFFER_QUEUE_IDENTITY	100

typedef struct st_bitQueue_Statistics {
	size_t len;
	size_t qtItens;
	size_t indxNextHead;
	size_t indxNextTail;
	size_t indxGet;
	size_t indxPut;
	long _upperLimitValue;
	long _lowerLimitValue;
	long sum;
	long maxValue;
	long minValue;
	long mediumValue;
	long modaValue;			// equals to puniqueList[0] 
	size_t *prankList;		// rankList and uniqueList will work together 
	long *puniqueList;		// This list will order by rankList
	long *pbufferList;		// The real data is here.
} bitQUEUE;


#if defined BITEIRO_QUEUE_C
#define EXTERN
#elif defined CPP_CODE
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

EXTERN bitQUEUE *bitQ_alloc(size_t len, long p_min, long p_max);
EXTERN int bitQ_free(bitQUEUE *);
EXTERN long bitQ_remove(bitQUEUE *pbq, bool tail);
EXTERN long bitQ_insert(bitQUEUE *pbq, long value, bool tail);
EXTERN int testBiteiroQueue(int argc, char ** argv);

#undef EXTERN