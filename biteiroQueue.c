#define BITEIRO_QUEUE_C
#include <biteiroCommons.h>
#include <biteiroMM.h>
#include <biteiroQueue.h>



bitQUEUE *bitQ_alloc(size_t buf_len, size_t queue_len, size_t p_keyLen, long p_min, long p_max)
{
	bitQUEUE *pq;
	
	assert(p_max > p_min);
	assert(buf_len < MAX_BUFFER_QUEUE_LENGHT);
	assert(buf_len > queue_len);
	assert(p_keyLen <= INTERNAL_MAX_KEY_LEN);
	
	size_t range = p_max - p_min + 1;

	assert(MAX_BUFFER_QUEUE_IDENTITY > range);
	
	pq = (bitQUEUE *)bitMM_alloc(sizeof(struct st_bitQueue_Statistics));

	pq->indxGet = 0;
	pq->indxPut = 0;
	pq->indxNextHead = 0;
	pq->indxNextTail = -1;
	pq->maxValue = LONG_MIN;
	pq->minValue = LONG_MAX;
	pq->meanHurst = (double)(p_max + p_min)/2;		// Initial value for mean
	pq->modeValue = 0;
	pq->medianValue = (long)(p_max + p_min) / 2;	// Aproximate value for the middle
	pq->qtItens = 0;
	pq->sum = 0;
	pq->hurstValue = 0;
	pq->_R_S= 0;
	pq->bufferLen = buf_len;
	pq->queueLen = queue_len;
	pq->_lowerLimitValue = p_min;
	pq->_upperLimitValue = p_max;

	pq->keyLen = 0;
	pq->maxKeyLen = p_keyLen;


	pq->pbufferList = (long *)bitMM_alloc(sizeof(long)*buf_len);
	memset(pq->pbufferList, 0, (sizeof(long)*buf_len));

	pq->psortedList = (long *)bitMM_alloc(sizeof(long)*buf_len);
	memset(pq->psortedList, 0, (sizeof(long)*buf_len));

	pq->puniqueList = (long *)bitMM_alloc(sizeof(long)*MAX_BUFFER_QUEUE_IDENTITY);
	memset(pq->puniqueList, 0, (sizeof(long)*MAX_BUFFER_QUEUE_IDENTITY));

	pq->prankList = (size_t *)bitMM_alloc(sizeof(size_t)*MAX_BUFFER_QUEUE_IDENTITY);
	memset(pq->prankList, 0, (sizeof(size_t)*MAX_BUFFER_QUEUE_IDENTITY));

	memset(pq->pkey, 0, INTERNAL_MAX_KEY_LEN * sizeof(long));

	return pq;
}

void _bitQ_makePatternKey(bitQUEUE *pq, long value, bool bRemIns /* 0-Remove  1-Insert*/)
{

	if (bRemIns)
	{
		if (pq->keyLen < pq->maxKeyLen)
		{
			pq->keyLen++;
		}
	}
	else
	{
		pq->keyLen--;
	}

	size_t j = pq->indxNextHead - pq->keyLen;

	for (int i = 0; i < pq->keyLen; i++, j++)
	{
		pq->pkey[i] = pq->pbufferList[j];
	}
}

void _bitQ_makeMedianValue(bitQUEUE *pq, long value, bool bRemIns /* 0-Remove  1-Insert*/)
{
	if (bRemIns)
	{
		if (pq->qtItens == 1)
		{
			pq->psortedList[0] = value;	// Ok! Already inserted the first item
			pq->qtOrder = pq->qtItens;
			pq->medianValue = value;
			return;
		}
		if (pq->qtItens == 2)
		{
			if (value < pq->psortedList[0])
			{
				SWAP_LONG(&value, &pq->psortedList[0]);
			}
			pq->psortedList[1] = value;
			pq->qtOrder = pq->qtItens;
			pq->medianValue = (pq->psortedList[0] + pq->psortedList[1])/2;
			return;
		}
	}
	else
	{
		// Removing the last item
		if (pq->qtItens == 0)
		{
			assert(pq->psortedList[0] == value);
			pq->psortedList[0] = 0;
			pq->qtOrder = 0;
			return;
		}
	}


	// Finding the entry point
	size_t entry = 0x2ADA2ADA;
	for (size_t i = 0; i < pq->qtOrder; i++)
	{
		if (bRemIns)
		{
			if (value <= pq->psortedList[i])
			{
				entry = i;
				break;
			}
		}
		else
		{
			if (value == pq->psortedList[i])
			{
				entry = i;
				break;
			}
		}

	}

	if (entry == 0x2ADA2ADA)
	{
		assert(bRemIns == TRUE);

		pq->psortedList[pq->qtOrder++] = value;

		assert(pq->qtItens == pq->qtOrder);

		size_t m = pq->qtOrder / 2;

		if (pq->qtOrder % 2 == 0)
		{
			pq->medianValue = (pq->psortedList[m] + pq->psortedList[m + 1])/2;
		}
		else
		{
			pq->medianValue = pq->psortedList[m + 1];
		}

		return;
	}

	long *ptmp = malloc((pq->qtOrder) * sizeof(long));

	memcpy(ptmp, &pq->psortedList[0], (pq->qtOrder) * sizeof(long));

	size_t is = 0;
	size_t it = 0;

	for (size_t i = 0; i < pq->qtOrder; i++)
	{
		if (i == entry)
		{
			if (bRemIns)
			{
				pq->psortedList[is++] = value;
			}
			else
			{
				it++;	// Jumps the removed item
			}
		}
		pq->psortedList[is++] = ptmp[it++];
	}

	if (bRemIns)
	{
		pq->qtOrder++;
	}
	else
	{
		pq->qtOrder--;
	}

	assert(pq->qtItens == pq->qtOrder);

	size_t m = pq->qtOrder / 2;

	if (pq->qtOrder % 2 == 0)
	{
		pq->medianValue = (pq->psortedList[m] + pq->psortedList[m - 1]) / 2;
	}
	else
	{
		pq->medianValue = pq->psortedList[m];
	}

	free(ptmp);

}

long _bitQ_dismakeModa(bitQUEUE *pq, long value)
{
	size_t end = pq->_upperLimitValue - pq->_lowerLimitValue;
	size_t ini = 0;
	long vaux = value;
	size_t i = ini;

	while (i < end)
	{
		if (pq->puniqueList[i] == vaux)
		{
			pq->prankList[i]--;
			i = ini;
			vaux = LONG_MAX;
			continue;
		}
		else if (pq->prankList[i] < pq->prankList[i + 1])
		{
			SWAP_LONG(&pq->puniqueList[i + 1], &pq->puniqueList[i]);
			SWAP_SIZE_T(&pq->prankList[i + 1], &pq->prankList[i]);
			i = ini;
			continue;
		} 
		else if (pq->prankList[i] == 0)
		{
			// This is the excluded item
			pq->puniqueList[i] = 0;
			break;
		}
		i++;
	}
	pq->modeValue = pq->puniqueList[0];
	return pq->modeValue;
}


long _bitQ_makeMode(bitQUEUE *pq, long value)
{
	assert(value >= pq->_lowerLimitValue);
	assert(value <= pq->_upperLimitValue);

	if (pq->qtItens == 1 || pq->puniqueList[0] == value)
	{
		pq->puniqueList[0] = value;
		pq->prankList[0]++;
		pq->modeValue = pq->puniqueList[0];
		return pq->modeValue;
	}


	size_t end = pq->_upperLimitValue - pq->_lowerLimitValue;
	size_t ini = 1;
	long vaux = value;
	size_t i = ini;
	while ( i < end)
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
		if (pq->puniqueList[i] == 0 && pq->prankList[i] == 0)		// This mark the end of this list
		{
			end = i;
			break;
		}
		++i;
	}
	if (vaux == value)
	{
		pq->puniqueList[end] = value;
		pq->prankList[end] = 1;
	}
	pq->modeValue = pq->puniqueList[0];
	return pq->modeValue;
}

void _bitQ_shift_right(bitQUEUE *pq)
{
	long *ptmp = malloc((pq->qtItens) * sizeof(long));

	memcpy(ptmp, pq->pbufferList, (pq->qtItens) * sizeof(long));

	memcpy(&pq->pbufferList[1], ptmp, (pq->qtItens) * sizeof(long));

	pq->indxNextHead++;
	pq->indxNextTail = -1;

	free(ptmp);
}

void _bitQ_shift_allLeft(bitQUEUE *pq)
{
	long *ptmp = malloc((pq->qtItens) * sizeof(long));

	memcpy(ptmp, &pq->pbufferList[pq->indxNextTail+1], (pq->qtItens) * sizeof(long));

	memset(pq->pbufferList, 0, pq->bufferLen * sizeof(long));

	memcpy(pq->pbufferList, ptmp, (pq->qtItens) * sizeof(long));

	pq->indxNextTail = -1;
	pq->indxNextHead = pq->qtItens;

	free(ptmp);
}

void _bitQ_makeHurstValue(bitQUEUE *pq)
{
	double media = pq->meanHurst;
	double soma = pq->sum;
	double di = 0, yi = 0;
	double maior = FLT_MIN;
	double menor = FLT_MAX;
	double pdi = 0;

	if (pq->qtItens < 4) {
		return;
	}

	size_t ini = pq->indxNextTail + 1;
	size_t end = pq->indxNextHead;

	for (size_t i = ini; i < end; i++)
	{
		// Calculo das diferencas
		di = (pq->pbufferList[i] - media);

		// Diferenca acumulada
		yi += di;

		// Maior
		if (yi > maior) {
			maior = yi;
		}

		// Menor
		if (yi < menor) {
			menor = yi;
		}

		// Potencia acumulada
		pdi += pow(di, 2.0);
	}

	double R = maior - menor;

	double S = sqrt(pdi / pq->qtItens);

	pq->_R_S = R / S;

	double logRS = log10(pq->_R_S);

	double logBase = log10((double)pq->qtItens);

	pq->hurstValue = logRS / logBase;
}



long bitQ_insert(bitQUEUE *pq, long value, bool tail)
{
	assert(value >= pq->_lowerLimitValue);
	assert(value <= pq->_upperLimitValue);

	if (pq->qtItens == pq->queueLen)
	{
		// Remove from the other side
		bitQ_remove(pq, !tail);
	}

	// Recalculations
	pq->qtItens++;

	// Max
	if (value > pq->maxValue)
	{
		pq->maxValueBackup = pq->maxValue;
		pq->maxValue = value;
	}

	// Min
	if (value < pq->minValue)
	{
		pq->minValueBackup = pq->minValue;
		pq->minValue = value;
	}

	// Sum
	pq->sum += value;

	// Media
	pq->meanHurst = ((double)pq->sum / (double)pq->qtItens);

	// Only for insersion and used to capture imediate insert and return values.
	pq->lastValue = value;

	// Moda
	_bitQ_makeMode(pq, value);


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

	if (pq->maxKeyLen > 0)
	{
		_bitQ_makePatternKey(pq, value, TRUE);
	}

	_bitQ_makeHurstValue(pq);

	_bitQ_makeMedianValue(pq, value, 1);

	return (long)pq->qtItens;
}

void _bitQ_makeMaxMin(bitQUEUE *pq)
{
	size_t ini = pq->indxNextTail + 1;
	size_t end = pq->indxNextHead;

	pq->maxValue = LONG_MIN;
	pq->minValue = LONG_MAX;
	pq->maxValueBackup = LONG_MIN;
	pq->minValueBackup = LONG_MAX;


	for (size_t i = ini; i < end; i++)
	{
		long value = pq->pbufferList[i];

		// Max
		if (value > pq->maxValue)
		{
			// Shifting values to trying backups
			pq->maxValueBackup = pq->maxValue;
			pq->maxValue = value;
		}

		// Min
		if (value < pq->minValue)
		{
			// Shifting values to trying backups
			pq->minValueBackup = pq->minValue;
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
		if ((pq->indxNextTail + pq->queueLen + 1) >= pq->bufferLen)
		{
			_bitQ_shift_allLeft(pq);
		}
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
		pq->meanHurst = ((double)pq->sum / (double)pq->qtItens);
	}
	else
	{
		// By coherence, but these values must be zero.
		pq->meanHurst = (double)pq->sum;
	}

	// Pattern Key
	if (pq->maxKeyLen > 0)
	{
		_bitQ_makePatternKey(pq, value, FALSE);
	}
	
	// Mode
	_bitQ_dismakeModa(pq, value);

	if (pq->lastValue == value)
	{
		if (pq->maxValue == value)
		{
			pq->maxValue = pq->maxValueBackup;
		}
		else if (pq->minValue == value)
		{
			pq->minValue = pq->minValueBackup;
		}
	}
	else
	{
		// Recalculates max and min
		_bitQ_makeMaxMin(pq);
	}


	_bitQ_makeHurstValue(pq);

	_bitQ_makeMedianValue(pq, value, 0);

	return value;
}


int bitQ_free(bitQUEUE *pq)
{
	bitMM_free(pq->prankList);
	bitMM_free(pq->puniqueList);
	bitMM_free(pq->pbufferList);
	bitMM_free(pq->psortedList);

	return bitMM_free(pq);
}

void bitQ_printAllItens(bitQUEUE *pq)
{
	printf(" - BuffList:");
	for (size_t i = 0; i < pq->bufferLen; i++)
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


void bitQ_showStatistics(bitQUEUE *pq, bool onlyLast)
{
	printf(" - Statistics:");
	for (size_t i = 0; pq->prankList[i] > 0; i++)
	{
		if (pq->puniqueList[i] == pq->lastValue)
		{
			printf(">%.3ld(%.2zd)", pq->puniqueList[i], pq->prankList[i]);
		}
		else
		{
			if (!onlyLast) 
			{
				printf(" %.3ld(%.2zd)", pq->puniqueList[i], pq->prankList[i]);
			}
		}
	}
}

void bitQ_printUniqueItens(bitQUEUE *pq)
{
	printf(" - UniqueList:");
	for (size_t i = 0; pq->prankList[i] > 0; i++)
	{
		printf(" %.2ld", pq->puniqueList[i]);
	}
}

void bitQ_printRankValues(bitQUEUE *pq)
{
	printf(" - RankList:");
	for (size_t i = 0; pq->prankList[i] > 0; i++)
	{
		printf(" %.2zd", pq->prankList[i]);
	}
}

int testBiteiroQueue(int argc, char ** argv)
{
	printf("\n =========================================== Biteiro Queue Tests ========================================================== \n");

	int values[] = {1,2,3,4,5,6,7,8,2,3,4,1,5,3,2,1,4,2,6,7,9};

	size_t len = sizeof(values) / sizeof(int);

	bitQUEUE *pq = bitQ_alloc(len, len, 0, 1, 9);

	printf("\n ---------------------------- Inserting --------------------------");
	printf("\n Itens=%zd, Max=%ld, Min=%ld, Mean=%f, Mode=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->meanHurst, pq->modeValue);

	for (int i = 0; i < len; i++)
	{
		bitQ_insert(pq, values[i], i % 2);
		printf("\n Itens=%zd, Max=%ld, Min=%ld, Mean=%f, Mode=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->meanHurst, pq->modeValue);
		bitQ_printAllItens(pq);
	}

	printf("\n ---------------------------- After Insert --------------------------");
	printf("\n Itens=%zd, Max=%ld, Min=%ld, Mean=%f, Mode=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->meanHurst, pq->modeValue);
	bitQ_printAllItens(pq);

	printf("\n ---------------------------- Removing --------------------------");

	for (int i = 0; i < len; i++)
	{
		long val = bitQ_remove(pq, i % 2);
		printf("\n Itens=%zd, Max=%ld, Min=%ld, Mean=%f, Mode=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->meanHurst, pq->modeValue);
		bitQ_printAllItens(pq);
		printf(" - Value %ld removed.", val);
	}
	printf("\n Itens=%zd, Max=%ld, Min=%ld, Mean=%f, Mode=%ld", pq->qtItens, pq->maxValue, pq->minValue, pq->meanHurst, pq->modeValue);



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
