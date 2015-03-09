#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "types.h"
#include "linscan.h"
#include "memusage.h"
#include "io.h"

int main (int argc, char**argv) {
    if (argc < 3) {
	printf("Usage:\n\nmih <infile> <outfile> [options]\n\n");
	printf("Options:\n");
	printf(" -N <number>          Set the number of binary codes from the beginning of the dataset file to be used\n");
	printf(" -Q <number>          Set the number of query points to use from <infile>, default all\n");
	printf(" -B <number>          Set the number of bits per code, default autodetect\n");
	printf(" -K <number>          Set number of nearest neighbors to be retrieved\n");
	printf("\n");
	return 0;
    }

    char *infile = argv[1];
    char *outfile = argv[2];
		
    UINT32 N = 0;
    UINT32 NQ = 0, Q0 = 0, Q1 = 0;
    int B = 0;
    UINT32 K = -1;

    for (int argnum = 3; argnum < argc; argnum++) {
	if (argv[argnum][0] == '-') {
	    switch (argv[argnum][1]) {
	    case 'B':
		B = atoi(argv[++argnum]);
		break;
	    case 'K':
		K = atoi(argv[++argnum]);
		break;
	    case 'N':
		N = atoi(argv[++argnum]);
		break;
	    case 'Q':
		Q0 = atoi(argv[++argnum]);
		if (++argnum < argc) {
		    if (argv[argnum][0] != '-') {
			Q1 = atof(argv[argnum]);
		    } else {
			argnum--;
			Q1 = Q0;
			Q0 = 0;
		    }
		}
		NQ = Q1-Q0;
		break;
	    default: 
		printf("Unrecognized Option or Missing Parameter when parsing: %s\n", argv[argnum]);
		return EXIT_FAILURE;
	    }
	} else {
	    printf("Invalid Argument: %s\n", argv[argnum]);
	    return EXIT_FAILURE;
	}
    }

    if (!NQ) {
	printf("-Q is required.\n");
	return EXIT_FAILURE;
    }

    if (B % 8 != 0) {		// in case of B == 0 this should be fine
	printf("Non-multiple of 8 code lengths are not currently supported.\n");
	return EXIT_FAILURE;
    }

    if (K < 1 || K > N) {
	printf("A valid K is not provided.\n");
	return EXIT_FAILURE;
    }

    int B_over_8 = B/8;
    /* Done with initialization and sanity checks */

    /* Loading the codes and queries from the input file */
    UINT8 *codes_db;
    int dim1codes;
    UINT8 *codes_query;
    int dim1queries;

    printf("Loading codes... ");
    fflush (stdout);

    codes_db = (UINT8*)malloc((size_t)N * (B / 8) * sizeof(UINT8));
    load_bin_codes(infile, "B", codes_db, &N, &B_over_8);
    if (B == 0)
	B = B_over_8 * 8; /* in this case B_over_8 is set within load_bin_codes */
    dim1codes = B / 8;

    printf("done.\n");
    printf("Loading queries... ");
    fflush (stdout);

    codes_query = (UINT8*)malloc((size_t)NQ * (B/8) * sizeof(UINT8));
    load_bin_codes(infile, "Q", codes_query, &NQ, &B_over_8, Q0);
    dim1queries = B/8;

    printf("done.\n");
    /* Done with the inputs */

    printf("N = %.0e |", (double)N);
    printf(" NQ = %d, range [%d %d) |", NQ, Q0, Q1);
    printf(" B = %d |", B);
    printf(" K = %4d |", K);
    printf("\n");

    /* Run linear scan and store the required stats */
    clock_t start0, end0;
    time_t start1, end1;
	
    printf("query... ");
    fflush (stdout);

    result_t result;
    result.n = N;
    result.nq = NQ;
    result.k = K;
    result.b = B;
    result.m = -1;		// m is irrelevant to linear scan
    result.q0 = Q0;
    result.q1 = Q1;
    result.wt = -1;
    result.cput = -1;
    result.vm = -1;
    result.rss = -1;
    result.res = NULL;
    result.nres = NULL;

    result.res = (UINT32 **) malloc(sizeof(UINT32*)*NQ);
    result.res[0] = (UINT32 *) malloc(sizeof(UINT32)*K*NQ);
    for (size_t i=1; i<NQ; i++)
	result.res[i] = result.res[i-1] + K;

    result.nres = (UINT32 **) malloc(sizeof(UINT32*)*NQ);
    result.nres[0] = (UINT32 *) malloc(sizeof(UINT32)*(B+1)*NQ);
    for (size_t i=1; i<NQ; i++)
	result.nres[i] = result.nres[i-1] + (B+1);

    result.stats = NULL;

    start1 = time(NULL);
    start0 = clock();

    linscan_query(result.nres[0], result.res[0], codes_db, codes_query, N, NQ, B, K,
		  dim1codes, dim1queries);

    end0 = clock();
    end1 = time(NULL);

    result.cput = (double)(end0-start0) / (CLOCKS_PER_SEC) / NQ;
    result.wt = (double)(end1-start1) / NQ;
    process_mem_usage(&result.vm, &result.rss);
    result.vm  /= double(1024*1024);
    result.rss /= double(1024*1024);
    printf("done | cpu %.3fs | wall %.3fs | VM %.1fgb | RSS %.1fgb     \n", result.cput, result.wt, result.vm, result.rss);
    /* Done with linear scan and storing the results */

    /* Opening the output file for writing the results */
    printf("Writing results to file %s... ", outfile);
    fflush(stdout);

    saveRes(outfile, "linscan", &result, 1, 2);

    printf("done.\n");
    /* Done with the output file */

    free(codes_query);
    free(codes_db);
    free(result.res[0]);
    free(result.res);
    free(result.nres[0]);
    free(result.nres);

    return 0;
}
