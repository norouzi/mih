#include "sparse_hashtable.h"

const int SparseHashtable::MAX_B = 37;

SparseHashtable::SparseHashtable() {
    table = NULL;
    size = 0;
    b = 0;
}

int SparseHashtable::init(int _b) {
    b = _b;
    if (b < 5 || b > MAX_B || b > sizeof(UINT64)*8)
	return 1;
    
    size = UINT64_1 << (b-5);	// size = 2 ^ b
    table = (BucketGroup*) calloc(size, sizeof(BucketGroup));

    return 0;
}

SparseHashtable::~SparseHashtable () {
    free(table);
}

void SparseHashtable::insert(UINT64 index, UINT32 data) {
    table[index >> 5].insert((int)(index % 32), data);
}

UINT32* SparseHashtable::query(UINT64 index, int *size) {
    return table[index >> 5].query((int)(index % 32), size);
}
