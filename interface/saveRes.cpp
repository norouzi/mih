#include "myhdf5.h"
#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <unistd.h>

#include "mihasher.h"
#include "result.h"
#include "resulth5.h"
#include "types.h"
#include "io.h"

void saveVarRef(hobj_ref_t *ref, hid_t file, int i, int dim1, int dim2, const char *varStrMain, const char *varStr, void *var, const char *type) {
    hsize_t dims[2] = {dim1, dim2};
    hid_t  space = H5Screate_simple(2, dims, NULL);
    herr_t status;
    char str[80];

    sprintf(str, "/refs/%s%d.%s", varStrMain, i, varStr);

    printf("Writing %s with size [%d, %d]\n", str, dim1, dim2);

    hid_t dset = 0;
    if (!strcmp(type, "uint32")) {
	dset = H5Dcreate(file, str, H5T_NATIVE_UINT32, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Dwrite(dset, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, (UINT32*)var);
    } else if (!strcmp(type, "double")) {
	dset = H5Dcreate(file, str, H5T_NATIVE_DOUBLE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Dwrite(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, (double*)var);
    }
    status = H5Sclose(space);
    status = H5Dclose(dset);
    
    status = H5Rcreate(ref, file, str, H5R_OBJECT, -1);
}

void saveRes(const char *filename, const char *varStr, const result_t *result, int n, int overwrite /* = 1 */) {  
    hid_t       file, space, dset, group;          /* Handles */
    herr_t      status;
    hsize_t     dims1[1], maxdims1[1], chunkdims1[1];
    char str[80];

    if (access(filename, F_OK) == -1) {
	overwrite = 1;
    }

    /* Open file and dataset using the default properties */
    if (overwrite == 1) {
	printf("Creating the file %s\n", filename);
    	file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	/* Creating a group for references */
	group = H5Gcreate(file, "/refs", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Gclose(group);
    } else if (overwrite == 2) {
    	file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
    }

    /* Creating the result hdf5 compound type */
    hid_t restype = H5Tcreate(H5T_COMPOUND, sizeof(resulth5_t));
    status = H5Tinsert (restype, "wt",    HOFFSET(resulth5_t, wt),      H5T_NATIVE_DOUBLE);
    status = H5Tinsert (restype, "cput",  HOFFSET(resulth5_t, cput),    H5T_NATIVE_DOUBLE);
    status = H5Tinsert (restype, "vm",    HOFFSET(resulth5_t, vm),      H5T_NATIVE_DOUBLE);
    status = H5Tinsert (restype, "rss",   HOFFSET(resulth5_t, rss),     H5T_NATIVE_DOUBLE);
    status = H5Tinsert (restype, "n",     HOFFSET(resulth5_t, n),       H5T_NATIVE_INT);
    status = H5Tinsert (restype, "m",     HOFFSET(resulth5_t, m),       H5T_NATIVE_INT);
    status = H5Tinsert (restype, "b",     HOFFSET(resulth5_t, b),       H5T_NATIVE_INT);
    status = H5Tinsert (restype, "nq",    HOFFSET(resulth5_t, nq),      H5T_NATIVE_INT);
    status = H5Tinsert (restype, "q0",    HOFFSET(resulth5_t, q0),      H5T_NATIVE_INT);
    status = H5Tinsert (restype, "q1",    HOFFSET(resulth5_t, q1),      H5T_NATIVE_INT);
    status = H5Tinsert (restype, "k",     HOFFSET(resulth5_t, k),       H5T_NATIVE_INT);
    status = H5Tinsert (restype, "res",   HOFFSET(resulth5_t, res),     H5T_STD_REF_OBJ);
    status = H5Tinsert (restype, "nres",  HOFFSET(resulth5_t, nres),    H5T_STD_REF_OBJ);
    status = H5Tinsert (restype, "stats", HOFFSET(resulth5_t, stats),   H5T_STD_REF_OBJ);

    if (overwrite == 1) {
	/* Copying results into hdf5 result type, and saving the references */
	resulth5_t *result5t = new resulth5_t[n];
	for (int i=0; i<n; i++) {
	    memcpy(&result5t[i], &result[i], HOFFSET(resulth5_t, res));
	    saveVarRef(&result5t[i].res,  file, i, result[i].nq, result[i].k, varStr, "res", result[i].res[0], "uint32");
	    saveVarRef(&result5t[i].nres, file, i, result[i].nq, result[i].b+1, varStr, "nres", result[i].nres[0], "uint32");
	    if (result[i].stats != NULL)
		saveVarRef(&result5t[i].stats, file, i, result[i].nq, STAT_DIM, varStr, "stats", result[i].stats[0], "double");
	    else
		result5t[i].stats = 0;
	}
	dims1[0] = n;
	maxdims1[0] = H5S_UNLIMITED;
	chunkdims1[0] = 1;

	/* Modify dataset creation properties, i.e. enable chunking  */
	hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
	status = H5Pset_chunk(prop, 1, chunkdims1);
	
	space = H5Screate_simple (1, dims1, maxdims1);
	dset = H5Dcreate(file, varStr, restype, space, H5P_DEFAULT, prop, H5P_DEFAULT);
	status = H5Dwrite(dset, restype, H5S_ALL, H5S_ALL, H5P_DEFAULT, result5t);

	free(result5t);

	status = H5Pclose(prop);
	// status = H5Sclose(space);
	status = H5Dclose(dset);
    } else if (overwrite == 2) {
	dset = H5Dopen(file, varStr, H5P_DEFAULT);

	space = H5Dget_space(dset);
	int ndims = H5Sget_simple_extent_dims(space, dims1, NULL);
	assert(ndims == 1);
	int nold = (int)dims1[0];

	hsize_t dims1extend[1] = {n};
	hsize_t dims1new[1] = {(dims1[0]+dims1extend[0])};

	status = H5Dextend (dset, dims1new);

	/* Select a hyperslab in extened portion of dataset  */
	space = H5Dget_space (dset);
	status = H5Sselect_hyperslab(space, H5S_SELECT_SET, dims1, NULL, dims1extend, NULL);

	/* Copying results into hdf5 result type, and saving the references */
	resulth5_t *result5t = new resulth5_t[n];
	result5t = result5t - nold;
	for (int i=0; i<n; i++) {
	    memcpy(&result5t[nold+i], &result[i], HOFFSET(resulth5_t, res));
	    saveVarRef(&result5t[nold+i].res,  file, i+dims1[0], result[i].nq, result[i].k, varStr, "res",  result[i].res[0], "uint32");
	    saveVarRef(&result5t[nold+i].nres, file, i+dims1[0], result[i].nq, result[i].b+1, varStr, "nres", result[i].nres[0], "uint32");
	    if (result[i].stats != NULL)
		saveVarRef(&result5t[nold+i].stats, file, i+dims1[0], result[i].nq, STAT_DIM, varStr, "stats", result[i].stats[0], "double");
	    else
		result5t[nold+i].stats = 0;
	}
	status = H5Dwrite(dset, restype, space, space, H5P_DEFAULT, result5t);

	free(result5t+nold);

	// status = H5Sclose(space);
	status = H5Dclose(dset);
    }

    status = H5Sclose(space);
    status = H5Tclose(restype);
    status = H5Fclose(file);
}
