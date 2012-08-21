#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "mat.h"

#include "types.h"
#include "linscan.h"

int main (int argc, char**argv) {
    if (argc < 3) {
	printf ("Usage:\n\nlinscan <infile> <outfile> [options]\n\n");
	printf ("Options:\n");
	printf (" -nMs <n1 n2 n3 ...>  Set an array of multiples of a million of items to experiment with\n");
	printf (" -nM <number>         Set the index from the nMs array to be used for this run (1 is the first)\n");
	printf (" -Q <number>          Set number of query points to use from <infile>, default all\n");
	printf (" -B <number>          Set number of bits per code, default autodetect\n");
	printf ("\n");
	return 0;
    } else {
	char *infile = argv[1];
	char *outfile = argv[2];
		
	UINT32 N = 0;
	int B = 0;
	UINT32 K = 100;
	int nM = 0;
	int NQ = 0;
	double *nMs = NULL;
	int nnMs = 0;

	for (int argnum = 3; argnum < argc; argnum++) {
	    // printf("%d '%s'\n", argnum, argv[argnum]);
	    if (argv[argnum][0] == '-') {
		switch (argv[argnum][1]  *(argc > argnum + 1)) {
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
		    } else if (!strcmp(argv[argnum], "-nM"))
			nM = atoi(argv[++argnum]);
		    break;
		case 'Q':
		    NQ = atoi(argv[++argnum]);
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
	mxArray *mxnMs = NULL, *mxlinscan = NULL;
	/* Opening output file to read "linscan" and "nMs" if available */
	ifp = matOpen(outfile, "r");
	if (ifp) {
	    mxnMs = matGetVariable(ifp, "nMs");
	    mxlinscan = matGetVariable(ifp, "linscan");

	    double *nMs2 = (double*)mxGetPr(mxnMs);
	    int nnMs2 = mxGetN(mxnMs);

	    if (nMs != NULL) {
		if (nnMs != nnMs2) {
		    printf("#nMs is different from the #nMs read from the output file.\n");
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

	if (mxlinscan == NULL) {
	    const char *ab[] = {"res", "nres", "wt", "cput", "nq"};
	    mxlinscan = mxCreateStructMatrix(1, nnMs, 5, ab);
	}
	/* Done with initializing mxnMs and mxlinscan and sanity checks */

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
	printf(" K = %d", K);
	printf("\n");
		
	/* Run linear scan and store the required stats */
	mxArray *mxcounter = mxCreateNumericMatrix (B+1, NQ, mxUINT32_CLASS, mxREAL);
	mxArray *mxres = mxCreateNumericMatrix (K, NQ, mxUINT32_CLASS, mxREAL);
	mxArray *mxctime = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL);
	mxArray *mxwtime = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL);

	UINT32 *counter = (UINT32*) mxGetPr(mxcounter);
	UINT32 *res = (UINT32*) mxGetPr(mxres);
	double *ctime = (double*) mxGetPr(mxctime);
	double *wtime = (double*) mxGetPr(mxwtime);
	
	printf("query... ");
	fflush (stdout);
	clock_t start0, end0;
	time_t start1, end1;
	
	start1 = time(NULL);
	start0 = clock();
	
	linscan_query(counter, res, (UINT8*)mxGetPr(mxcodes), (UINT8*)mxGetPr(mxqueries), N, NQ, B, K, \
		      dim1codes, dim1queries);
	
	end0 = clock();
	end1 = time(NULL);
	
	*ctime = (double)(end0-start0) / (CLOCKS_PER_SEC) / NQ;
	*wtime = (double)(end1-start1) / NQ;
	printf ("done | cpu %.3fs | wall %.3fs.      \n", *ctime, *wtime);
	
	int ind = (nM-1);
	const char* ab[] = {"res", "nres", "wt", "cput", "nq"};
	mxSetFieldByNumber(mxlinscan, ind, 0, mxres);
	mxSetFieldByNumber(mxlinscan, ind, 1, mxcounter);
	mxSetFieldByNumber(mxlinscan, ind, 2, mxwtime);
	mxSetFieldByNumber(mxlinscan, ind, 3, mxctime);
	mxSetFieldByNumber(mxlinscan, ind, 4, mxCreateDoubleScalar(NQ));
	/* Done with linear scan and storing the stats */
	
	/* Opening the output file for writing the results */
	MATFile *ofp = matOpen (outfile, "w");
	if (!ofp) {
	    printf ("Failed to create/open output file. Aborting.\n");
	    return EXIT_FAILURE;
	}
			
	printf("Writing results to file %s... ", outfile);
	fflush(stdout);
	matPutVariable(ofp, "nMs", mxnMs);
	matPutVariable(ofp, "linscan", mxlinscan);
	printf("done.\n");
	matClose(ofp);
	/* Done with the output file */

	mxDestroyArray (mxnMs);
	mxDestroyArray (mxlinscan);
	mxDestroyArray (mxcodes);
	mxDestroyArray (mxqueries);
	/* skip deleting nMs if it is initialized by new double[] */
    }

    return 0;
}
