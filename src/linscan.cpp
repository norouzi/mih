#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "bitops.h"
#include "types.h"
#include "linscan.h"

void linscan_query(UINT32 *counter, UINT32 *res, UINT8 *codes, UINT8 *queries, int N, UINT32 NQ, int B, int R, \
		   int dim1codes, int dim1queries) {

    int B_over_8 = B / 8;
    int *cum_counter;    // cumulative counter
    UINT32 *ind;         // stores indices arranged based on thir Hamming distances

    cum_counter = new int[B+1];
    ind = new UINT32[R*(B+1)];
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
	    if (counter[h]++ < R)
		ind[R*h + counter[h] - 1] = j+1;
    	}

    	cum_counter[0] = counter[0];
	int uptoj = 0;
    	for (int j=1; j<=B; j++) {
    	    cum_counter[j] = cum_counter[j-1] + counter[j];
	    if (cum_counter[j] >= R && cum_counter[j-1] < R)
		uptoj = j;
	}

	cum_counter[uptoj] = R;	// so we stop at R

	int indres = 0;
	for (int h=0; h<=uptoj; h++) {
	    int ind0 = h == 0 ? 0 : cum_counter[h-1];
	    
	    for (int i=ind0; i<cum_counter[h]; i++)
		res[i] = ind[R*h + i - ind0];
	}
	
    	res += R;
    	counter += B+1;
    }
    
    delete [] cum_counter;
    delete [] ind;
}

// void* threadquery(void*  ptr) {
//     threadinfo*info = (threadinfo*) ptr;
//     query(info->threadid, info->counter, info->results, info->q, info->numq);
//     pthread_exit(NULL);
// }

// void batchquery(UINT32* counter, UINT32* results, UINT8* q, UINT32 numq, int _numthreads)
// {
//     int numthreads = _numthreads;

//     // numqdone = new UINT32 [numthreads];
//     // for (int i=0; i<numthreads; i++)
//     // 	numqdone[i] = 0;
	
//     if (numthreads > 1) {
// 	pthread_t* threads = new pthread_t [numthreads];
// 	threadinfo* tinfo = new threadinfo [numthreads];
	
// 	UINT64 tnumq = numq/numthreads;
// 	for (int i=0; i<numthreads; i++) {
// 	    tinfo[i].threadid = i;
// 	    tinfo[i].q = q + (UINT64)(B_over_8)*i*tnumq;
// 	    tinfo[i].numq = tnumq + ( (i==numthreads-1)? numq % numthreads : 0 );
// 	    tinfo[i].results = results + (UINT64)R * i * tnumq;
// 	    tinfo[i].counter = counter + (UINT64)(B+1) * i * tnumq;
		
// 	    pthread_create(&threads[i], NULL, threadquery, (void*) &tinfo[i]);
// 	}
	
// 	// UINT32 done = 0;
// 	// while (done != numq) {
// 	//     done = 0;
// 	//     for (int i=0; i<numthreads; i++)
// 	// 	done += numqdone[i];
// 	// }
	
// 	void* status;
// 	for (int i=0; i<numthreads; i++)
// 	    pthread_join(threads[i], &status);

// 	delete [] threads;
// 	delete [] tinfo;
//     } else {
// 	query(0, counter, results, q, numq);
//     }

//     // delete [] numqdone;
// }
