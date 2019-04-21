#define BITEIRO_MM_C
#include <biteiroCommons.h>
#include <biteiroMM.h>

bitMM *pmm_last = NULL;
bitMM *pmm_first = NULL;

bitMM *bitMM_ctor()
{
	size_t bit_len = sizeof(struct st_bitMM);

	size_t sts_len = sizeof(struct st_bitMM_Statistics);

	pmm_first = (bitMM *)malloc(bit_len + sts_len + 1);

	assert(pmm_first);

	pmm_first->bufLen = sts_len;
	pmm_first->pnext = NULL;
	pmm_first->pprev = NULL;
	pmm_first->statusKey = BMM_ALLOC_KEY;

	bitMMSTS *psts = (bitMMSTS *)((size_t)pmm_first + bit_len);

	psts->allocs = 1;
	psts->frees = 0;
	psts->total_mem = bit_len + sts_len + 1;

	pmm_first->pbuff = (char *)psts;
	pmm_first->pbuff[pmm_first->bufLen] = BMM_ALLOC_TAIL;

	pmm_last = pmm_first;

	return pmm_first;
}

char *bitMM_alloc(size_t len)
{
	size_t bit_len = sizeof(struct st_bitMM);

	bitMM *pp = (bitMM *)malloc(bit_len + len + 1);  // Plus Tail flag

	assert(pp != NULL);

	assert(pmm_last->statusKey == BMM_ALLOC_KEY);

	assert(pmm_last->pbuff[pmm_last->bufLen] == BMM_ALLOC_TAIL);


	pp->pbuff = (char *)((size_t)pp + bit_len);
	pp->pnext = NULL;
	pp->pprev = pmm_last;
	pp->statusKey = BMM_ALLOC_KEY;
	pp->bufLen = len;
	pp->pbuff[pp->bufLen] = BMM_ALLOC_TAIL;

	pmm_last->pnext = pp;
	pmm_last = pp;


	((bitMMSTS *)(pmm_first->pbuff))->total_mem += (bit_len + len + 1);
	((bitMMSTS *)(pmm_first->pbuff))->allocs++;

	return pp->pbuff;
}

int bitMM_free(void *pbuf)
{
	size_t bit_len = sizeof(struct st_bitMM);

	bitMM *pp;	// Previous
	bitMM *pm;	// Middle
	bitMM *pn;	// Next


	pm = (bitMM *)((size_t)pbuf - bit_len);

	assert(pm->statusKey == BMM_ALLOC_KEY);

	assert(pm->pbuff[pm->bufLen] == BMM_ALLOC_TAIL);

	pp = pm->pprev;
	pn = pm->pnext;

	assert(pp != NULL);

	if (pn != NULL)
	{
		pp->pnext = pn;
		pn->pprev = pp;
	}
	else
	{
		pmm_last = pp;
		pp->pnext = NULL;
	}


	((bitMMSTS *)(pmm_first->pbuff))->total_mem -= (bit_len + pm->bufLen);
	((bitMMSTS *)(pmm_first->pbuff))->frees++;

	free(pm);

	return 0;
}

int bitMM_dest(bitMM *p_pmm)
{
	int deallocs = 0;

	assert(pmm_first);
	assert(p_pmm == pmm_first);

	bitMM *paux = pmm_first->pnext;

	while (paux != NULL)
	{
		bitMM *pp = paux;
		paux = pp->pnext;

		assert(pp->statusKey == BMM_ALLOC_KEY);

		assert(pp->pbuff[pp->bufLen] == BMM_ALLOC_TAIL);

		deallocs++;

		free(pp);

	} 

	assert(pmm_first->statusKey == BMM_ALLOC_KEY);

	deallocs++;

	free(pmm_first);

	return deallocs;
}

void bitMM_printStatus(bitMM *pm)
{
//	printf(" O Tamanho do bool em c eh %zd\n", sizeof(bool));

	bitMMSTS *ps = (bitMMSTS *)pm->pbuff;

	printf("\n Allocs: %zd - Frees: %zd - Total Memory : %zd", ps->allocs, ps->frees, ps->total_mem);

}

int testBiteiroMemoryManagement(int argc, char ** argv)
{
	printf("\n =========================================== Biteiro Management Memory Tests ========================================================== \n");
	char *paux, *paux2 = NULL;
	int dealocs;

	bitMM *pm0 = bitMM_ctor();

	for (int i = 0; i < 10; i++)
	{

		size_t mem_alloc = i;

		paux = bitMM_alloc(mem_alloc);

		if (mem_alloc)
		{
			memset(paux, 'z', mem_alloc);

			paux[mem_alloc - 1] = 0;

			printf("\n\n%s", paux);
		}
		else
		{
			paux2 = bitMM_alloc(1000);
		}

		bitMM_printStatus(pm0);
		bitMM_free(paux);
		bitMM_printStatus(pm0);

	}
	printf("\n Out of LOOP ");
	bitMM_free(paux2);
	bitMM_printStatus(pm0);

	dealocs = bitMM_dest(pm0);
	printf("\n %d Deallocations for pm0", dealocs);



	bitMM *pm1 = bitMM_ctor();
	for (int i = 2; i < 10; i++)
	{

		size_t mem_alloc = i * i * i * i - i * i * i;
		
		char *paux = bitMM_alloc(mem_alloc);

		memset(paux, 'z', mem_alloc);

		//paux[mem_alloc - 1] = 0;

		//printf("\n\n%s", paux);

		bitMM_printStatus(pm1);

		bitMM_free(paux);
	}

	bitMM_printStatus(pm1);
	dealocs = bitMM_dest(pm1);
	printf("\n %d Deallocations for pm1", dealocs);


	bitMM *pm2 = bitMM_ctor();
	for (int i = 2; i < 10; i++)
	{

		size_t mem_alloc = i * i * i * i - i * i * i;

		paux = bitMM_alloc(mem_alloc);

		memset(paux, 'z', mem_alloc);

		//paux[mem_alloc - 1] = 0;

		//printf("\n\n%s", paux);

		bitMM_printStatus(pm2);

	}
	printf("\n Out of LOOP ");
	bitMM_free(paux);

	bitMM_printStatus(pm2);
	dealocs = bitMM_dest(pm2);
	printf("\n %d Deallocations for pm2", dealocs);

	return 0;
}
