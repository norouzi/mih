~~~~~~~~~~~~~ About

This is a C++/matlab implementation of the algorithm presented in the paper "Fast
Search in Hamming Space with Multi-Index Hashing, M. Norouzi, A. Punjani,
D. J. Fleet, CVPR 2012". The goal is to perform fast exact nearest neighbor search
in Hamming distance on binary codes. Using this code, one can re-run the
experiments described in our paper. This implementation improves the storage
efficiency of our previous implementation explained in the paper by utilizing
sparse hash tables. (http://www.cs.toronto.edu/~norouzi/research/mih/)


~~~~~~~~~~~~~~ Datasets

Download the INRIA bigann dataset (1 billion SIFT features) from
http://corpus-texmex.irisa.fr/ and store it under data/inria/.

You can also download the Tiny images dataset (80 million GIST
descriptors) from http://horatio.cs.nyu.edu/mit/tiny/data/index.html
and store it under data/tiny.

By running create_lsh_codes.m (a matlab snippet) you can generate binary codes
from the above datasets using random projections (LSH, "Similarity estimation
techniques from rounding algorithms, M. Charikar, STOC. 2002"). By changing the
first few lines of create_lsh_codes, you can control the parameters of the matlab
snippet. See create_lsh_codes.m for more details.


~~~~~~~~~~~~~~ Usage

~~~ RUN.sh: is a bash script that compiles the code, and provides an
example run of the program for 64-bit codes. For 128-bit and 256-bit
experiments, you could set the bash variable "nb" to 128 or 256, and
export it, and then RUN.sh will perform the experiments with "$nb"
bits.


Compilation:

You need make, cmake, and matlab headers and libraries, to build this
project. Take a look at RUN.sh and set the variable "matlab_root_dir"
to point to your matlab root dir. Generally, for compilation, you need
to go into the build folder and run:

cmake .. -DMATLABROOT=your_matlab_root_dir
make

This should generate two binary files: mih and linscan. RUN.sh should
run the above commands for you.

~~~ linscan: provides an efficient implementation of exhaustive linear scan for
kNN in Hamming distance on binary codes. (for comparison)

linscan <infile> <outfile> [options]

Options:
 -nMs <n1 n2 ..>  An array of multiples of one million items that we intend to run the experiments on them
 -nM <number>     The index from the nMs array to be used for this run (first is 1)
 -Q <number>      The number of query points to use from <infile>, default all
 -K <number>      The number of results to return per query, default 100 (k in kNN)
 -B <number>      The number of bits per code, default autodetect

Examples:
./build/linscan codes/lsh/lsh_128_sift_1B.mat cache/linscan_128_1B.mat -Q 1000 -K 100 -nMs 1 10 100 1000 -nM 1
./build/linscan codes/lsh/lsh_128_sift_1B.mat cache/linscan_128_1B.mat -Q 1000 -K 100 -nMs 1 10 100 1000 -nM 2
./build/linscan codes/lsh/lsh_128_sift_1B.mat cache/linscan_128_1B.mat -Q 1000 -K 100 -nMs 1 10 100 1000 -nM 3
./build/linscan codes/lsh/lsh_128_sift_1B.mat cache/linscan_128_1B.mat -Q 1000 -K 100 -nMs 1 10 100 1000 -nM 4

Assuming that a dataset of 128-bit binary codes is stored at
codes/lsh/lsh_128_sift_1B.mat, running the above lines will create an output file
cache/linscan_128_1B.mat, which stores the results and timings for 100-NN search
on 1 million, 10 million, 100 million, and 1 billion binary codes. If the output
file does not exist (the first time), the output file is created with the
appropriate number of elements in an struct array (based on -nMs <n1 n2 ..>). If
the output file exists (from the second time), the file is appended with the new
results.

Note that '-nMs 1 10 100 1000' lets the program know that we intend to run the
experiments on 4 subsets of the dataset (with 1M, 10M, 100M, and 1B data points)
and '-nM i' specifies the index of the subset that should be used for the current
run of the program (eg, here -nM3 corresponds to 100M).

~~~ mih: provides an implementation of multi-index hashing for fast exact kNN in
Hamming distance on binary codes.

mih <infile> <outfile> [options]

Options:
 -nMs <n1 n2 ..>  An array of multiples of one million items that we intend to run the experiments on them
 -nM <number>     The index from the nMs array to be used for this run (first is 1)
 -Q <number>      The number of query points to use from <infile>, default all
 -B <number>      The number of bits per code, default autodetect
 -m <number>      The number of substrings to use, default 1

The mih's options are very similar to linscan. It has an additional argument (-m)
to determine the number of hash tables / substrings to use. The code is set up
such that it performs kNN for all k in {1, 10, 100, 1000} so the -K argument is
not used.


~~~~~~~~~~~~~ License

Copyright (c) 2012, Mohammad Norouzi <mohammad.n@gmail.com> and Ali Punjani
<alipunjani@cs.toronto.edu>. This is a free software; for license information
please refer to license.txt file.


~~~~~~~~~~~~~~ TODO

The code reads the full matrix of binary codes B each time, regardless of N (or
nMs[nM]) which takes time and storage. It would be nice if we can only read the
first N columns of B (probably using http://www.hdfgroup.org/HDF5/). Because of
this problem, the current estimate of storage is not accurate and it includes the
full matrix B.

Improve SparseHashtable insertion speed. It is currently very slow, but can be
improved in different ways.
