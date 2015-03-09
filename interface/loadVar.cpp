#include "myhdf5.h"
#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "io.h"

void load_bin_codes(const char *filename, const char *varStr, UINT8 *codes, UINT32 *N, int *B, int start0/* default = 0*/) {
    hid_t       file, space, dset;          /* Handles */
    herr_t      status;
    hsize_t     dims[10];

    /* Open file and dataset using the default properties */
    file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    dset = H5Dopen(file, varStr, H5P_DEFAULT);
    space = H5Dget_space(dset);

    /* Get info of the size of the array */
    int ndims = H5Sget_simple_extent_dims(space, dims, NULL);
    assert(ndims == 2);
    
    hsize_t start[2] = {start0, 0};
    hsize_t count[2];
    if (*N == 0)
	*N = dims[0];
    else
	assert(*N <= dims[0]);
    count[0] = *N;
    if (*B == 0)
	*B = dims[1];
    else
	assert(*B == dims[1]);
    count[1] = *B;
    status = H5Sselect_hyperslab(space, H5S_SELECT_SET, start, NULL, count, NULL);
    printf("status %d\n", status);

    /*
     * Read the data using the default properties.
     */
    codes = codes - start0 * (*B); // change the pointer so that it points to the beginning of the selected set.
    printf("start0: %d, B: %d, N: %d\n", start0, *B, *N);
    // codes = codes-sizeof(*codes)*start0*(*B); // change the pointer so that it points to the beginning of the selected set.
    status = H5Dread(dset, H5T_NATIVE_UCHAR, H5S_ALL, space, H5P_DEFAULT, codes);
    printf("status %d\n", status);

    /*
     * Close and release resources.
     */
    status = H5Sclose(space);
    status = H5Dclose(dset);
    status = H5Fclose(file);
}

void load_double_matrix(const char *filename, const char *varStr, UINT8 *matrix, int *nrow, int *ncol) {
    hid_t       file, space, dset;          /* Handles */
    herr_t      status;
    hsize_t     dims[10];

    /* Open file and dataset using the default properties */
    file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    dset = H5Dopen(file, varStr, H5P_DEFAULT);
    space = H5Dget_space(dset);

    /* Get info of the size of the array */
    int ndims = H5Sget_simple_extent_dims(space, dims, NULL);
    assert(ndims == 2);
    
    hsize_t count[2];
    if (*nrow == 0)
	*nrow = dims[0];
    else
	assert(*nrow <= dims[0]);
    if (*ncol == 0)
	*ncol = dims[1];
    else
	assert(*ncol == dims[1]);

    /*
     * Read the data using the default properties.
     */
    // matrix = matrix-sizeof(*matrix)*start0*B;
    status = H5Dread(dset, H5T_NATIVE_UCHAR, H5S_ALL, space, H5P_DEFAULT, matrix);

    /*
     * Close and release resources.
     */
    status = H5Sclose(space);
    status = H5Dclose(dset);
    status = H5Fclose(file);
}
