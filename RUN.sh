# change your_matlab_root_dir to your matlab root dir
your_matlab_root_dir=

# add matlab to the PATH if it is not already there
# download the data, and create data/inria/ folder that includes ANN_SIFT1B folder and files


# some sanity checks
if [ "$your_matlab_root_dir" = "" ]
then
    echo "set your_matlab_root_dir to your matlab root dir."
    exit
fi
if [ ! -d data/inria/ANN_SIFT1B ]
then
    echo "data/inria/ANN_SIFT1B does not exist."
    exit
fi
if [ ! -d data/inria/matlab ]
then
    echo "data/inria/matlab does not exist."
    exit
fi


# compilation
mkdir -p build
cd build
cmake .. -DMATLABROOT=$your_matlab_root_dir
make
cd ..

# creation of the binary codes
# runs matlab, and then create_lsh_codes script inside it.
matlab -nojvm -nodisplay -nosplash -r "nb=64;create_lsh_codes;quit"
if [ $? != 0 ]; then
    echo "Could not run matlab to create lsh codes... Aborting"
    exit
fi

# number of substrings to be used for each case is stored in a bash
# array nch. Multi-index hashing is run for 12 different subsets of
# the binary codes (10K 100K 1M 2M 5M 10M 20M 100M 200M 500M 1B)
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
do 
    ./build/mih codes/lsh/lsh_64_sift_1B.mat cache/mih_64_1B.mat -Q 1000 -nMs 0.01 0.10 1 2 5 10 20 50 100 200 500 1000 -nM $nm -m ${nch[$nm]}
    if [ $? != 0 ]; then
	echo "Could not run mih for some reason... Aborting"
	exit
    fi
done

# testing
matlab -nojvm -nodisplay -nosplash -r "addpath('test');test_mih_with_linscan('/tmp/lsh/lsh_64_sift_1B.mat',8,'cache/mih_64_1B.mat','cache/linscan_64_1B.mat');quit"
