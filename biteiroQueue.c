#define BITEIRO_QUEUE_C
#include <biteiroCommons.h>
#include <biteiroMM.h>
#include <biteiroQueue.h>



bitQUEUE *bitQ_alloc(size_t len, long p_min, long p_max)
{
	bitQUEUE *pq;
	
	assert(p_max > p_min);
	assert(len < MAX_BUFFER_QUEUE_LENGHT);
	
	pq = (bitQUEUE *)bitMM_alloc(sizeof(struct st_bitQueue_Statistics));

	pq->indxGet = 0;
	pq->indxPut = 0;
	pq->indxNextHead = 0;
	pq->indxNextTail = -1;
	pq->maxValue = LONG_MIN;
	pq->minValue = LONG_MAX;
	pq->mediumValue = (p_max + p_min)/2;
	pq->modaValue = 0;
	pq->qtItens = 0;
	pq->sum = 0;
	pq->len = len;
	pq->_lowerLimitValue = p_min;
	pq->_upperLimitValue = p_max;

	pq->pbufferList = (long *)bitMM_alloc(sizeof(long)*len);
	memset(pq->pbufferList,0,(sizeof(long)*len));

	pq->puniqueList = (long *)bitMM_alloc(sizeof(long)*MAX_BUFFER_QUEUE_IDENTITY);
	memset(pq->puniqueList, 0, (sizeof(long)*MAX_BUFFER_QUEUE_IDENTITY));

	pq->prankList = (size_t *)bitMM_alloc(sizeof(size_t)*MAX_BUFFER_QUEUE_IDENTITY);
	memset(pq->prankList, 0, (sizeof(size_t)*MAX_BUFFER_QUEUE_IDENTITY));

	return pq;
}

long _bitQ_dismakeModa(bitQUEUE *pq, long value)
{
	size_t end = pq->qtItens;
	size_t ini = 0;
	long vaux = value;
	for (size_t i = ini; i < end; i++)
	{
		if (pq->puniqueList[i] == vaux)
		{
			pq->prankList[i]--;
			vaux = LONG_MAX;
			i = ini;
			continue;
		}
		else if (pq->prankList[i] > pq->prankList[i - 1])
		{
			SWAP_LONG(&pq->puniqueList[i - 1], &pq->puniqueList[i]);
			SWAP_SIZE_T(&pq->prankList[i - 1], &pq->prankList[i]);
			i = ini;
			continue;
		}
	}
	if (vaux == LONG_MAX && pq->prankList[end - 1] == 0)
	{
		// Exclude the last value
		pq->puniqueList[end-1] = 0;
		
	}
	pq->modaValue = pq->puniqueList[0];
	return pq->modaValue;
}


long _bitQ_makeModa(bitQUEUE *pq, long value)
{
	if (pq->qtItens == 1 || pq->puniqueList[0] == value)
	{
		pq->puniqueList[0] = value;
		pq->prankList[0]++;
		pq->modaValue = pq->puniqueList[0];
		return pq->modaValue;
	}

//	if (pq->qtItens > 1)

	size_t end = pq->qtItens - 1;
	size_t ini = 1;
	long vaux = value;
	for (size_t i = ini; i < end; i++)
	{
		if (pq->puniqueList[i] == vaux)
		{
			pq->prankList[i]++;
			vaux = LONG_MAX;
			i = ini;
			continue;
		} 
		else if (pq->prankList[i] > pq->prankList[i - 1])
		{
			SWAP_LONG(&pq->puniqueList[i - 1], &pq->puniqueList[i]);
			SWAP_SIZE_T(&pq->prankList[i - 1], &pq->prankList[i]);
			i = ini;
			continue;
		}
	}
	if (vaux == value)
	{
		pq->puniqueList[end] = value;
		pq->prankList[end] = 1;
	}
	pq->modaValue = pq->puniqueList[0];
	return pq->modaValue;
}

void _bitQ_shift_right(bitQUEUE *pq)
{
	int *ptmp = malloc((pq->qtItens) * sizeof(long));

	memcpy(ptmp, pq->pbufferList, (pq->qtItens) * sizeof(long));

	memcpy(&pq->pbufferList[1], ptmp, (pq->qtItens) * sizeof(long));

	pq->indxNextHead++;
	pq->indxNextTail = -1;
}

long bitQ_insert(bitQUEUE *pq, long value, bool tail)
{

	assert(pq->indxNextHead <= (pq->len-1));

	// Recalculations
	pq->qtItens++;

	// Max
	if (value > pq->maxValue)
	{
		pq->maxValue = value;
	}

	// Min
	if (value < pq->minValue)
	{
		pq->minValue = value;
	}

	// Sum
	pq->sum += value;

	// Media
	pq->mediumValue = (pq->sum / (long)pq->qtItens);

	// Moda
	_bitQ_makeModa(pq, value);


	if (tail)
	{
		if (pq->indxNextTail == -1)
		{
			_bitQ_shift_right(pq);
			pq->pbufferList[0] = value;
		}
		else
		{
			pq->pbufferList[pq->indxNextTail--] = value;
		}
	}
	else
	{
		pq->pbufferList[pq->indxNextHead++] = value;
	}

	return (long)pq->qtItens;
}

void _bitQ_makeMaxMin(bitQUEUE *pq)
{
	size_t ini = pq->indxNextTail + 1;
	size_t end = pq->indxNextHead;

	pq->maxValue = LONG_MIN;
	pq->minValue = LONG_MAX;

	for (size_t i = ini; i < end; i++)
	{
		long value = pq->pbufferList[i];

		// Max
		if (value > pq->maxValue)
		{
			pq->maxValue = value;
		}

		// Min
		if (value < pq->minValue)
		{
			pq->minValue = value;
		}

	}

}

long bitQ_remove(bitQUEUE *pq, bool tail)
{
	long value;
	assert(pq->qtItens > 0);

	pq->qtItens--;

	if (tail)
	{
		//  First we increment because next index will be indxNextTail incremented
		pq->indxNextTail++;
		value = pq->pbufferList[pq->indxNextTail];	// Saves the value to be returned to the caller
		pq->pbufferList[pq->indxNextTail] = 0;		// Mark this point free to the next entry
	}
	else
	{
		pq->indxNextHead--;
		value = pq->pbufferList[pq->indxNextHead];
		pq->pbufferList[pq->indxNextHead] = 0;
	}

	// Sum
	pq->sum -= value;

	// Media
	if (pq->qtItens > 0)
	{
		pq->mediumValue = (pq->sum / (long)pq->qtItens);
	}
	else
	{
		// By coherence, but these values must be zero.
		pq->mediumValue = pq->sum;
	}

	// Moda
	_bitQ_dismakeModa(pq, value);


	_bitQ_makeMaxMin(pq);

	return value;
}


int bitQ_free(bitQUEUE *pq)
{
	bitMM_free(pq->prankList);
	bitMM_free(pq->puniqueList);
	bitMM_free(pq->pbufferList);

	return bitMM_free(pq);
}

void bitQ_printAllItens(bitQUEUE *pq)
{
	printf(" - BuffList:");
	for (size_t i = 0; i < pq->len; i++)
	{
		if (pq->pbufferList[i] != 0)
		{
			printf("[%.2ld]", pq->pbufferList[i]);
		}
		else
		{
			printf("[--]");
		}

	}
}

int testBiteiroQueue(int argc, char ** argv)
{
	printf("\n =========================================== Biteiro Queue Tests ========================================================== \n");

	int values[] = {1,2,3,4,5,6,7,8,2,3,4,1,5,3,2,1,4,2,6,7,9};

	size_t len = sizeof(values) / sizeof(int);

	bitQUEUE *pq = bitQ_alloc(len, 1, 9);

	printf("\n ---------------------------- Inserting --------------------------");
	printf("\n Itens=%zd, Max=%ld, Min=%ld, Med=%ld, Mod=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->mediumValue, pq->modaValue);

	for (int i = 0; i < len; i++)
	{
		bitQ_insert(pq, values[i], i % 2);
		printf("\n Itens=%zd, Max=%ld, Min=%ld, Med=%ld, Mod=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->mediumValue, pq->modaValue);
		bitQ_printAllItens(pq);
	}

	printf("\n ---------------------------- After Insert --------------------------");
	printf("\n Itens=%zd, Max=%ld, Min=%ld, Med=%ld, Mod=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->mediumValue, pq->modaValue);
	bitQ_printAllItens(pq);

	printf("\n ---------------------------- Removing --------------------------");

	for (int i = 0; i < len; i++)
	{
		long val = bitQ_remove(pq, i % 2);
		printf("\n Itens=%zd, Max=%ld, Min=%ld, Med=%ld, Mod=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->mediumValue, pq->modaValue);
		bitQ_printAllItens(pq);
		printf(" - Value %ld removed.", val);
	}
	printf("\n Itens=%zd, Max=%ld, Min=%ld, Med=%ld, Mod=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->mediumValue, pq->modaValue);



	return 0;
}

/*
 =========================================== Biteiro Queue Tests ==========================================================

 ---------------------------- Inserting --------------------------
 Itens=0, Max=-2147483648, Min=2147483647, Med=5, Mod=0
 Itens=1, Max=1, Min=1, Med=1, Mod=1 - BuffList:[01][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=2, Max=2, Min=1, Med=1, Mod=1 - BuffList:[02][01][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=3, Max=3, Min=1, Med=2, Mod=1 - BuffList:[02][01][03][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=4, Max=4, Min=1, Med=2, Mod=1 - BuffList:[04][02][01][03][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=5, Max=5, Min=1, Med=3, Mod=1 - BuffList:[04][02][01][03][05][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=6, Max=6, Min=1, Med=3, Mod=1 - BuffList:[06][04][02][01][03][05][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=7, Max=7, Min=1, Med=4, Mod=1 - BuffList:[06][04][02][01][03][05][07][--][--][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=8, Max=8, Min=1, Med=4, Mod=1 - BuffList:[08][06][04][02][01][03][05][07][--][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=9, Max=8, Min=1, Med=4, Mod=1 - BuffList:[08][06][04][02][01][03][05][07][02][--][--][--][--][--][--][--][--][--][--][--][--]
 Itens=10, Max=8, Min=1, Med=4, Mod=2 - BuffList:[03][08][06][04][02][01][03][05][07][02][--][--][--][--][--][--][--][--][--][--][--]
 Itens=11, Max=8, Min=1, Med=4, Mod=2 - BuffList:[03][08][06][04][02][01][03][05][07][02][04][--][--][--][--][--][--][--][--][--][--]
 Itens=12, Max=8, Min=1, Med=3, Mod=2 - BuffList:[01][03][08][06][04][02][01][03][05][07][02][04][--][--][--][--][--][--][--][--][--]
 Itens=13, Max=8, Min=1, Med=3, Mod=2 - BuffList:[01][03][08][06][04][02][01][03][05][07][02][04][05][--][--][--][--][--][--][--][--]
 Itens=14, Max=8, Min=1, Med=3, Mod=2 - BuffList:[03][01][03][08][06][04][02][01][03][05][07][02][04][05][--][--][--][--][--][--][--]
 Itens=15, Max=8, Min=1, Med=3, Mod=2 - BuffList:[03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][--][--][--][--][--][--]
 Itens=16, Max=8, Min=1, Med=3, Mod=2 - BuffList:[01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][--][--][--][--][--]
 Itens=17, Max=8, Min=1, Med=3, Mod=2 - BuffList:[01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][--][--][--][--]
 Itens=18, Max=8, Min=1, Med=3, Mod=2 - BuffList:[02][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][--][--][--]
 Itens=19, Max=8, Min=1, Med=3, Mod=2 - BuffList:[02][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][06][--][--]
 Itens=20, Max=8, Min=1, Med=3, Mod=2 - BuffList:[07][02][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][06][--]
 Itens=21, Max=9, Min=1, Med=4, Mod=2 - BuffList:[07][02][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][06][09]
 ---------------------------- After Insert --------------------------
 Itens=21, Max=9, Min=1, Med=4, Mod=2 - BuffList:[07][02][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][06][09]
 ---------------------------- Removing --------------------------
 Itens=20, Max=8, Min=1, Med=3, Mod=2 - BuffList:[07][02][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][06][--] - Value 9 removed.
 Itens=19, Max=8, Min=1, Med=3, Mod=2 - BuffList:[--][02][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][06][--] - Value 7 removed.
 Itens=18, Max=8, Min=1, Med=3, Mod=2 - BuffList:[--][02][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][--][--] - Value 6 removed.
 Itens=17, Max=8, Min=1, Med=3, Mod=2 - BuffList:[--][--][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][04][--][--] - Value 2 removed.
 Itens=16, Max=8, Min=1, Med=3, Mod=2 - BuffList:[--][--][01][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][--][--][--] - Value 4 removed.
 Itens=15, Max=8, Min=1, Med=3, Mod=2 - BuffList:[--][--][--][03][01][03][08][06][04][02][01][03][05][07][02][04][05][02][--][--][--] - Value 1 removed.
 Itens=14, Max=8, Min=1, Med=3, Mod=3 - BuffList:[--][--][--][03][01][03][08][06][04][02][01][03][05][07][02][04][05][--][--][--][--] - Value 2 removed.
 Itens=13, Max=8, Min=1, Med=3, Mod=3 - BuffList:[--][--][--][--][01][03][08][06][04][02][01][03][05][07][02][04][05][--][--][--][--] - Value 3 removed.
 Itens=12, Max=8, Min=1, Med=3, Mod=3 - BuffList:[--][--][--][--][01][03][08][06][04][02][01][03][05][07][02][04][--][--][--][--][--] - Value 5 removed.
 Itens=11, Max=8, Min=1, Med=4, Mod=3 - BuffList:[--][--][--][--][--][03][08][06][04][02][01][03][05][07][02][04][--][--][--][--][--] - Value 1 removed.
 Itens=10, Max=8, Min=1, Med=4, Mod=3 - BuffList:[--][--][--][--][--][03][08][06][04][02][01][03][05][07][02][--][--][--][--][--][--] - Value 4 removed.
 Itens=9, Max=8, Min=1, Med=4, Mod=2 - BuffList:[--][--][--][--][--][--][08][06][04][02][01][03][05][07][02][--][--][--][--][--][--] - Value 3 removed.
 Itens=8, Max=8, Min=1, Med=4, Mod=2 - BuffList:[--][--][--][--][--][--][08][06][04][02][01][03][05][07][--][--][--][--][--][--][--] - Value 2 removed.
 Itens=7, Max=7, Min=1, Med=4, Mod=2 - BuffList:[--][--][--][--][--][--][--][06][04][02][01][03][05][07][--][--][--][--][--][--][--] - Value 8 removed.
 Itens=6, Max=6, Min=1, Med=3, Mod=2 - BuffList:[--][--][--][--][--][--][--][06][04][02][01][03][05][--][--][--][--][--][--][--][--] - Value 7 removed.
 Itens=5, Max=5, Min=1, Med=3, Mod=2 - BuffList:[--][--][--][--][--][--][--][--][04][02][01][03][05][--][--][--][--][--][--][--][--] - Value 6 removed.
 Itens=4, Max=4, Min=1, Med=2, Mod=2 - BuffList:[--][--][--][--][--][--][--][--][04][02][01][03][--][--][--][--][--][--][--][--][--] - Value 5 removed.
 Itens=3, Max=3, Min=1, Med=2, Mod=2 - BuffList:[--][--][--][--][--][--][--][--][--][02][01][03][--][--][--][--][--][--][--][--][--] - Value 4 removed.
 Itens=2, Max=2, Min=1, Med=1, Mod=2 - BuffList:[--][--][--][--][--][--][--][--][--][02][01][--][--][--][--][--][--][--][--][--][--] - Value 3 removed.
 Itens=1, Max=1, Min=1, Med=1, Mod=0 - BuffList:[--][--][--][--][--][--][--][--][--][--][01][--][--][--][--][--][--][--][--][--][--] - Value 2 removed.
 Itens=0, Max=-2147483648, Min=2147483647, Med=0, Mod=0 - BuffList:[--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--][--] - Value 1 removed.
 Itens=0, Max=-2147483648, Min=2147483647, Med=0, Mod=0
*/
