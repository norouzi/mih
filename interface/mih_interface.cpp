#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "mat.h"

#include "types.h"
#include "mihasher.h"
#include "memusage.h"

int main (int argc, char**argv) {
    if (argc < 3) {
	printf ("Usage:\n\nmih <infile> <outfile> [options]\n\n");
	printf ("Options:\n");
	printf (" -nMs <n1 n2 n3 ...>  Set an array of multiples of 'one million items' to experiment with\n");
	printf (" -nM <number>         Set the index from the nMs array to be used for this run (1 is the first)\n");
	printf (" -Q <number>          Set the number of query points to use from <infile>, default all\n");
	printf (" -B <number>          Set the number of bits per code, default autodetect\n");
	printf (" -m <number>          Set the number of chunks to use, default 1\n");
	printf ("\n");
	return 0;
    }

    char *infile = argv[1];
    char *outfile = argv[2];
		
    UINT32 N = 0;
    int B = 0;
    int m = 1;
    UINT32 K = 100;
    int nM = 0;
    int NQ = 0;
    double *nMs = NULL;
    int nnMs = 0;
	
    for (int argnum = 3; argnum < argc; argnum++) {
	if (argv[argnum][0] == '-') {
	    switch (argv[argnum][1]) {
	    case 'B':
		B = atoi(argv[++argnum]);
		break;
	    case 'K':
		K = atoi(argv[++argnum]);
		break;
	    case 'n':
		if (!strcmp(argv[argnum], "-nMs")) {
		    nMs = new double[100];
		    while (++argnum < argc)
			if (argv[argnum][0] != '-') {
			    nMs[nnMs++] = atof(argv[argnum]);
			} else {
			    argnum--;
			    break;
			}
		} else if (!strcmp(argv[argnum], "-nM")) {
		    nM = atoi(argv[++argnum]);
		}
		break;
	    case 'Q':
		NQ = atoi(argv[++argnum]);
		break;
	    case 'm':
		m = atoi(argv[++argnum]);
		break;
	    default: 
		printf ("Unrecognized Option or Missing Parameter when parsing: %s\n", argv[argnum]);
		return EXIT_FAILURE;
	    }
	} else {
	    printf ("Invalid Argument: %s\n", argv[argnum]);
	    return EXIT_FAILURE;
	}
    }

    MATFile *ifp = NULL;
    mxArray *mxnMs = NULL;
    mxArray *mxret = NULL;
    /* Opening output file to read "ret" and "nMs" if available */
    ifp = matOpen(outfile, "r");
    if (ifp) {
	mxnMs = matGetVariable(ifp, "nMs");
	mxret = matGetVariable(ifp, "ret");

	double *nMs2 = (double*)mxGetPr(mxnMs);
	int nnMs2 = mxGetN(mxnMs);

	if (nMs != NULL) {
	    if (nnMs != nnMs2) {
		printf("#nMs is different from the #nMs read from the output file %d vs. %d.\n", nnMs, nnMs2);
		return EXIT_FAILURE;
	    }
	    for (int i=0; i<nnMs; i++)
		if (int(nMs[i]*1.0e6) !=  int(nMs2[i] * 1.0e6)) {
		    printf("nMs are different from the nMs read from the output file.\n");
		    return EXIT_FAILURE;
		}
	    delete[] nMs;
	}

	nnMs = nnMs2;
	nMs = nMs2;
	matClose (ifp);
    } else {
	mxnMs = mxCreateNumericMatrix(1, nnMs, mxDOUBLE_CLASS, mxREAL);
	double *nMs2 = (double*)mxGetPr(mxnMs);
	for (int i=0; i<nnMs; i++)
	    nMs2[i] = nMs[i];
	delete[] nMs;
	nMs = nMs2;
    }

    if (mxret == NULL) {
	const char* ab[] = {"res", "nres", "stat", "wt", "cput", "vm", "rss", "m"};
	mxret = mxCreateStructMatrix(1000, nnMs, 8, ab);
    }
    /* Done with initializing mxnMs and mxret and sanity checks */

    /* Loading the codes and queries from the input file */	
    ifp = matOpen (infile, "r");
    if (!ifp) {
	printf ("Failed to open input file. Aborting.\n");
	return EXIT_FAILURE;
    }

    printf ("Loading codes... ");
    fflush (stdout);
    mxArray *mxcodes = matGetVariable (ifp, "B");
    printf ("done.\n");
		
    printf ("Loading queries... ");
    fflush (stdout);
    mxArray *mxqueries = matGetVariable (ifp, "Q");
    printf ("done.\n");
    matClose (ifp);
    /* Done with the inputs */
	
    int dim1codes = mxGetM(mxcodes);
    int dim1queries = mxGetM(mxqueries);
    if (!B)
	B = mxGetM(mxcodes)*8;
    if (B % 8 != 0) {
	printf ("Non-multiple of 8 code lengths are not currently supported.\n");
	return EXIT_FAILURE;
    }
    N = 1.0e6 * nMs[nM-1];
    if (N)
	N = std::min ( (UINT32)N, (UINT32)mxGetN(mxcodes) );
    if (NQ > 0)
	NQ = std::min( NQ, (int)mxGetN(mxqueries) );
    else
	NQ = mxGetN (mxqueries);
	
    printf("nM = %d |", nM);
    printf(" N = %.0e |", (double)N);
    printf(" NQ = %d |", NQ);
    printf(" B = %d |", B);
    printf(" m = %d", m);
    printf("\n");
		
    /* Run multi-index hashing for 1,10,100,1000 NN and store the required stats */
    int mold = -1;
    mihasher* MIH = NULL;
    clock_t start0, end0;
    time_t start1, end1;
    qstat *stats = (qstat*) new qstat[NQ];

    for (K = 1; K<=1000; K *= 10) {
	mxArray *mxresults = mxCreateNumericMatrix (K, NQ, mxUINT32_CLASS, mxREAL);
	mxArray *mxnumres = mxCreateNumericMatrix (B+1, NQ, mxUINT32_CLASS, mxREAL);
	mxArray *mxctime = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL);
	mxArray *mxwtime = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL);
	mxArray *mxvm  = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL);
	mxArray *mxrss = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL);
	mxArray *mxstats = mxCreateNumericMatrix (6, NQ, mxDOUBLE_CLASS, mxREAL);

	UINT32 *numres = (UINT32*) mxGetPr (mxnumres);
	UINT32 *results = (UINT32*) mxGetPr (mxresults);
	double *ctime = (double*) mxGetPr (mxctime);
	double *wtime = (double*) mxGetPr (mxwtime);	    	    
	double *vm  = (double*) mxGetPr (mxvm);
	double *rss = (double*) mxGetPr (mxrss);	    	    
	double *stats_d = (double*) mxGetPr (mxstats);
	    
	/* if m changes from K to K */
	if (mold != m) {
	    if (MIH != NULL) {
		printf ("Clearing up... ");
		fflush (stdout);
		delete MIH;
		printf ("done.          \r");
		fflush (stdout);
	    }

	    MIH = new mihasher(B, m);
		
	    printf ("Populating %d hashtables with %.0e entries...\n", m, (double)N);
	    fflush (stdout);
	    start1 = time(NULL);
	    start0 = clock();
	    
	    MIH->populate ( (UINT8*) mxGetPr(mxcodes), N, dim1codes);
	    
	    end0 = clock();
	    end1 = time(NULL);

	    double ct = (double)(end0-start0) / (CLOCKS_PER_SEC);
	    double wt = (double)(end1-start1);

	    printf ("done. | cpu %.0fm%.0fs | wall %.0fm%.0fs\n", ct/60, ct-60*int(ct/60), wt/60, wt-60*int(wt/60));
	    // printf ("done.                                            \r");
	    // fflush (stdout);
	}

	printf(" m = %2d |", m);
	printf(" K = %4d |", K);
	printf(" ");
	MIH->setK(K);

	printf("query... ");
	fflush (stdout);
	    
	start1 = time(NULL);
	start0 = clock();
	    
	MIH->batchquery (results, numres, stats, (UINT8*) mxGetPr(mxqueries), NQ, dim1queries);
	    
	end0 = clock();
	end1 = time(NULL);

	*ctime = (double)(end0-start0) / (CLOCKS_PER_SEC) / NQ;
	*wtime = (double)(end1-start1) / NQ;
	process_mem_usage(vm, rss);
	*vm  /= double(1024*1024);
	*rss /= double(1024*1024);
	printf ("done | cpu %.3fs | wall %.3fs | VM %.1fgb | RSS %.1fgb     \n", *ctime, *wtime, *vm, *rss);

	int ind = 1000*(nM-1) + K-1;

	double *pstats_d = stats_d;
	for (int i=0; i<NQ; i++) {
	    pstats_d[0] = stats[i].numres;
	    pstats_d[1] = stats[i].numcand;
	    pstats_d[2] = stats[i].numdups;
	    pstats_d[3] = stats[i].numlookups;
	    pstats_d[4] = stats[i].maxrho;
	    pstats_d[5] = (double) stats[i].ticks / CLOCKS_PER_SEC;

	    pstats_d += 6;
	}

	mxSetFieldByNumber(mxret, ind, 0, mxresults);
	mxSetFieldByNumber(mxret, ind, 1, mxnumres);
	mxSetFieldByNumber(mxret, ind, 2, mxstats);
	mxSetFieldByNumber(mxret, ind, 3, mxwtime);
	mxSetFieldByNumber(mxret, ind, 4, mxctime);
	mxSetFieldByNumber(mxret, ind, 5, mxvm);
	mxSetFieldByNumber(mxret, ind, 6, mxrss);
	mxSetFieldByNumber(mxret, ind, 7, mxCreateDoubleScalar(m));

	mold = m;
    }
    printf ("Clearing up... ");
    fflush (stdout);
    delete MIH;
    printf ("done.          \r");
    fflush (stdout);
    /* Done with mulit-index hashing and storing the stats */
	
    /* Opening the output file for writing the results */
    MATFile *ofp = matOpen (outfile, "w");
    if (!ofp) {
	printf ("Failed to create/open output file. Aborting.\n");
	return EXIT_FAILURE;
    }
			
    printf("Writing results to file %s... ", outfile);
    fflush(stdout);
    matPutVariable(ofp, "nMs", mxnMs);
    matPutVariable(ofp, "ret", mxret);
    printf("done.\n");
    matClose(ofp);
    /* Done with the output file */

    delete[] stats;
    mxDestroyArray (mxnMs);
    mxDestroyArray (mxret);
    mxDestroyArray (mxcodes);
    mxDestroyArray (mxqueries);
    /* skip deleting nMs if it is initialized by new double[] */

    return 0;
}
