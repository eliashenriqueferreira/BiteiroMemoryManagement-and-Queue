#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define SANNITY_CHECK_VALUE	0X5A6596A5
#define MAX_BUFFER_QUEUE_LENGHT		1000
#define MAX_BUFFER_QUEUE_IDENTITY	512
#define INTERNAL_MAX_KEY_LEN		12	


typedef struct st_bitQueue_Statistics {
	size_t keyLen;
	size_t maxKeyLen;
	long pkey[INTERNAL_MAX_KEY_LEN];	// Key used to search patterns
	size_t bufferLen;
	size_t queueLen;
	size_t qtItens;			// Quantity of itens inserted in pbufferList
	size_t qtOrder;			// Quantity of ordered list in psortedList. Must be the same value of qtItens
	size_t indxNextHead;
	size_t indxNextTail;
	size_t indxGet;
	size_t indxPut;
	double hurstValue;		// This is the hurst number. (Remember 0 ==> is Alternance; 1 ==> is Assertivity; 0.5 ==> is Random )
	double _R_S;			// This is the confiability factor of hurst
	long _upperLimitValue;
	long _lowerLimitValue;
	long sum;
	long maxValue;
	long minValue;
	long maxValueBackup;	// To speed up return of max values
	long minValueBackup;	// To speed up return of min values
	long lastValue;			// This is the last value inserted.
	long medianValue;		// This is the tendency value or the midle value.
	double meanHurst;		// This is the average used by hurst calculus.
	long modeValue;			// equals to puniqueList[0] 
	size_t *prankList;		// rankList and uniqueList will work together 
	long *puniqueList;		// This list will order by rankList
	long *psortedList;		// Used to calculate median value.
	long *pbufferList;		// The real data is here.
} bitQUEUE;


#if defined BITEIRO_QUEUE_C
#define EXTERN
#elif defined CPP_CODE
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

EXTERN bitQUEUE *bitQ_alloc(size_t buf_len, size_t queue_len, size_t p_keyLen, long p_min, long p_max);
EXTERN int bitQ_free(bitQUEUE *);
EXTERN long bitQ_remove(bitQUEUE *pbq, bool tail);
EXTERN long bitQ_insert(bitQUEUE *pbq, long value, bool tail);
EXTERN void bitQ_printAllItens(bitQUEUE *pq);
EXTERN void bitQ_printUniqueItens(bitQUEUE *pq);
EXTERN void bitQ_printRankValues(bitQUEUE *pq);
EXTERN void bitQ_showStatistics(bitQUEUE *pq, bool onlyLast);
EXTERN int testBiteiroQueue(int argc, char ** argv);

#undef EXTERN