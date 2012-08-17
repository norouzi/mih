#ifndef BUCKET_GROUP_H__
#define BUCKET_GROUP_H__

#include <stdio.h>
#include <math.h>
#include "types.h"
#include "array32.h"
#include "bitarray.h"

class BucketGroup {

 public:
    
    UINT32 empty;

    Array32 *group;

    BucketGroup();

    ~BucketGroup();

    void insert(int subindex, UINT32 data);

    UINT32* query(int subindex, int *size);

};

#endif
