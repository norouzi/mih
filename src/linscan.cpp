#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "bitops.h"
#include "types.h"
#include "linscan.h"


/**
 * Performs kNN using linear scan in Hamming distance between codes and queries
 * Inputs:
 *   N: number of codes in the db
 *   NQ: number of queries to be answered
 *   B: number of bits in the db/query codes that should be taken into account in Hamming distance
 *   K: number of results to be returned for each query ie, k in kNN
 *   codes: an array of UINT8 storing the db codes
 *   queries: an array of UINT8 storing the query codes
 *   dim1codes: number of words in the database codes -- very likely dim1codes = B/8
 *   dim2codes: number of words in the query codes -- very likely dim2codes = B/8
 * Outputs:
 *   counter: int[(B+1)*N], stores the number of db items with different Hamming 
 *     distances (ranging from 0 to B) from each query.
 *   res: int[K*N], stores the ids of K nearest neighbors for each query
 */
void linscan_query(UINT32 *counter, UINT32 *res, UINT8 *codes, UINT8 *queries, int N, UINT32 NQ, int B, int K,
		   int dim1codes, int dim1queries) {

    int B_over_8 = B / 8;
    int *cum_counter;    // cumulative counter
    UINT32 *ind;         // stores indices arranged based on thir Hamming distances

    cum_counter = new int[B+1];
    ind = new UINT32[K*(B+1)];
    memset(counter, 0, (B+1)*NQ*sizeof(*counter));

    UINT8 *pqueries = queries;

    for (int i=0; i<NQ; i++, pqueries += dim1queries) {
    	UINT8 *pcodes = codes;
    	for (int j=0; j<N; j++, pcodes += dim1codes) {
    	    int h = match(pcodes, pqueries, B_over_8);
	    if (h > B || h < 0) {
		printf("Wrong Hamm distance\n");
		exit(1);
	    }
	    if (counter[h]++ < K)
		ind[K*h + counter[h] - 1] = j+1;
    	}

    	cum_counter[0] = counter[0];
	int uptoj = 0;
    	for (int j=1; j<=B; j++) {
    	    cum_counter[j] = cum_counter[j-1] + counter[j];
	    if (cum_counter[j] >= K && cum_counter[j-1] < K)
		uptoj = j;
	}

	cum_counter[uptoj] = K;	// so we stop at K

	int indres = 0;
	for (int h=0; h<=uptoj; h++) {
	    int ind0 = h == 0 ? 0 : cum_counter[h-1];
	    
	    for (int i=ind0; i<cum_counter[h]; i++)
		res[i] = ind[K*h + i - ind0];
	}
	
    	res += K;
    	counter += B+1;
    }
    
    delete [] cum_counter;
    delete [] ind;
}
