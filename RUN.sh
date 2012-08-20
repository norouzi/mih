# compilation
mkdir -p build
cd build
cmake .. -DMATLABROOT=***your_matlab_root_dir***
make
cd ..

# creation of the binary codes
# runs matlab, and then create_lsh_codes script inside it. Then exits.
matlab -nojvm -nodisplay -nosplash -r "create_lsh_codes;quit";

# number of substrings to be used for each case and running
# multi-index hashing for 12 different subsets of the data
nch[1]=5;
nch[2]=4;
nch[3]=4;
nch[4]=3;
nch[5]=3;
nch[6]=3;
nch[7]=3;
nch[8]=2;
nch[9]=2;
nch[10]=2;
nch[11]=2;
nch[12]=2;
for ((nm=1; nm<=12; nm=nm+1))
do ./build/mih codes/lsh/lsh_64_sift_1B.mat cache/mih_64_1B.mat -Q 1000 -nMs 0.01 0.10 1 2 5 10 20 50 100 200 500 1000 -nM $nm -m ${nch[$nm]}
done

# testing
matlab -nojvm -nodisplay -nosplash -r "addpath('test');test_mih_with_linscan('/tmp/lsh/lsh_256_sift_1B.mat', 32, 'cache/compsbk2/mih_256_1B_m8.mat', 'cache/compsbk2/linscan_256_1B.mat');quit"
