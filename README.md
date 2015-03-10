Multi Index Hashing (MIH)
=======

An implementation of *"Fast Exact Search in Hamming Space with
Multi-Index Hashing, M. Norouzi, A. Punjani, D. J. Fleet, IEEE TPAMI
2014"*. See http://www.cs.toronto.edu/~norouzi/research/mih/.

This algorithm performs fast exact nearest neighbor search in Hamming
distance on binary codes. Using this code, one can re-run the
experiments described in the paper. For best results, consider using
*libhugetlbfs* with multi-index hashing.

### Compilation

You need make, cmake, hdf5 library, hdf5-dev package to build this
project. To compile, create a folder called `build`, and run:

```
cd build
rm * -rf
cmake ..
make
```
This should generate two binary files: `mih` and `linscan`

### Datasets

An example binary code dataset with 1 million 64-bit codes from SIFT
is stored in the data folder. To generate larger binary code datasets,
one should download raw data which can be converted to binary codes
using hashing techniques (e.g., LSH or MLH).  For example, download
the INRIA bigann dataset (1 billion SIFT features) from
http://corpus-texmex.irisa.fr/ and store it under data/inria/.  You
can also download the Tiny images dataset (80 million GIST
descriptors) from http://horatio.cs.nyu.edu/mit/tiny/data/index.html
and store it under data/tiny.

By running create_lsh_codes.m (a matlab snippet) one can generate
binary codes from the above datasets using random projections (LSH,
"Similarity estimation techniques from rounding algorithms,
M. Charikar, STOC. 2002"). By changing the first few lines of
create_lsh_codes, you can control the parameters of the matlab
snippet. The output is in matlab (version 7.3) format, which is
essentially hdf5 format. Hence, we use hdf5 library to read the binary
code datasets.

### Usage

`RUN.sh` is a bash script showing an example run of the program 64-bit
codes. Set the parameters `nb`, `HUGE`, `hashfunc`, etc. to change the
setting. `RUN.sh` includes suggested values for `m`: number of hash
tables.

##### Linear scan
`linscan` provides an efficient implementation of exhaustive linear scan for
kNN in Hamming distance on binary codes. This serves as a good baseline.

```
linscan <infile> <outfile> [options]
Options:
  -N <number>       Set the number of binary codes from the beginning of the dataset file to be used
  -B <number>       Set the number of bits per code, default autodetect
  -Q <number>       Set the number of query points to use from <infile>, default all
  -K <number>       Set number of nearest neighbors to be retrieved
```

Examples:
```
./build/linscan data/lsh_64_sift_1M.mat linscan_64_1M.h5 -N 100000  -B 64 -Q 1000 -K 100
./build/linscan data/lsh_64_sift_1M.mat linscan_64_1M.h5 -N 1000000 -B 64 -Q 1000 -K 100
```

Assuming that a dataset of 128-bit binary codes is stored at
`codes/lsh_64_sift_1M.mat`, running the above lines will create an
output file `linscan_64_1M.h5`, which stores the results and timings
for 100-NN search on 100K and 1M binary codes. If the output file does
not exist (the first time), the output file is created. If the output
file exists (since the second time), the file is appended with the new
results.

##### Multi Index Hashing
`mih` provides an implementation of multi-index hashing for fast exact kNN in
Hamming distance on binary codes.

```
mih <infile> <outfile> [options]
Options:
  -N <number>          Set the number of binary codes from the beginning of the dataset file to be used
  -B <number>          Set the number of bits per code, default autodetect
  -Q <number>          Set the number of query points to use from <infile>, default all
  -m <number>          Set the number of chunks to use, default 1
  -K <number>          Set number of nearest neighbors to be retrieved
  -R <number>          Set the number of codes (in Millions) to use in computing the optimal bit reordering, default OFF (0)
```

Examples:
```
./build/mih data/lsh_64_sift_1M.mat mih_64_1M.h5 -N 100000 -B 64 -m 5 -Q 10000 -K 100
./build/mih data/lsh_64_sift_1M.mat mih_64_1M.h5 -N 1000000 -B 64 -m 4 -Q 10000 -K 100
```

The mih's options are very similar to linscan. It has an additional
argument (-m) to determine the number of hash tables. It also has a
flag (-R) to determine whether the assignment of bits to the
substrings should be optimized.

### FAQs

Q: I have tried your code with some of my datasets. It works well when
I used small datasets, but it does not perform well with large
datasets.

A: Did you try decreasing the number of hash tables (by the -m switch) as
you increased the size of the database? My experience is that with the
correct choice of m, the speedup on larger datasets should be much
better. In the RUN.sh file, I have a set of suggestions for the values
of m for different number of codes in the datasets.

### License

Copyright (c) 2012, Mohammad Norouzi [<mohammad.n@gmail.com>] and Ali Punjani
[<alipunjani@cs.toronto.edu>]. This is a free software; for license information
please refer to license.txt file.

### TODO

- Automatic suggestion for the m parameter.
- Multi-core functionality.
- Improve SparseHashtable insertion speed. It is currently very slow,
but can be improved in different ways.
