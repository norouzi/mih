# change matlab_root_dir to your matlab root dir
matlab_root_dir=

# add matlab to the PATH if it is not already there
# download the data, and create data/inria/ folder that includes ANN_SIFT1B folder and files

# if number of bits (nb) is not provided from outside, set it to 64
if [ -z "$nb" ]
then
    nb=64;
fi


# some sanity checks
if [ "$matlab_root_dir" = "" ]
then
    echo "set matlab_root_dir to your matlab root dir."
    exit 1
fi
if [ ! -d data/inria/ANN_SIFT1B ]
then
    echo "data/inria/ANN_SIFT1B does not exist."
    exit 1
fi
if [ ! -d data/inria/matlab ]
then
    echo "data/inria/matlab does not exist."
    exit 1
fi


# compilation
mkdir -p build
cd build
cmake ..
make
cd ..

# creation of the binary codes
# runs matlab, and then create_lsh_codes script inside it.
matlab -nojvm -nodisplay -nosplash -r "nb=$nb;create_lsh_codes;quit"
if [ $? != 0 ]; then
    echo "Could not run matlab to create lsh codes... Aborting"
    exit 1
fi

# Multi-index hashing is being run for 12 different subsets of the
# binary codes (10K 100K 1M 2M 5M 10M 20M 100M 200M 500M 1B
# items). Number of substrings to be used for each case is stored in a
# bash array nch.
if [ "$nb" = "64" ]
then
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
else
    if [ "$nb" = "128" ]
	nch[1]=10;
	nch[2]=8;
	nch[3]=8;
	nch[4]=6;
	nch[5]=6;
	nch[6]=5;
	nch[7]=5;
	nch[8]=5;
	nch[9]=5;
	nch[10]=4;
	nch[11]=4;
	nch[12]=4;
    else
	if [ "$nb" = "256" ]
	    nch[1]=19;
	    nch[2]=15;
	    nch[3]=13;
	    nch[4]=12;
	    nch[5]=11;
	    nch[6]=11;
	    nch[7]=10;
	    nch[8]=10;
	    nch[9]=10;
	    nch[10]=9;
	    nch[11]=9;
	    nch[12]=8;
	else
	    echo "nb = $nb is not supported";
	fi
    fi
fi

db_file="codes/lsh/lsh_"$nb"_sift_1B.mat";
mih_results="cache/mih_"$nb"_1B.mat";
linscan_results="cache/linscan_"$nb"_1B.mat";

for ((nm=1; nm<=12; nm=nm+1))
do 
    build/mih $db_file $mih_results  -Q 1000 -nMs 0.01 0.10 1 2 5 10 20 50 100 200 500 1000 -nM $nm -m ${nch[$nm]}
    if [ $? != 0 ]; then
	echo "Could not run mih for some reason... Aborting"
	exit 1
    fi
done

# Linear scan is being run for 12 different subsets of the binary
# codes. The number of queries is set to 100 (-Q 100) to increase the
# speed. The K in K-NN is set to 1000 (-K 1000)
for ((nm=1; nm<=12; nm=nm+1))
do
    build/linscan $db_file $linscan_results -Q 100 -nMs 0.01 0.10 1 2 5 10 20 50 100 200 500 1000 -nM $nm -K 1000
    if [ $? != 0 ]; then
	echo "Could not run linscan for some reason... Aborting"
	exit 1
    fi
done

# Testing the results of mih with the results of linscan
matlab -nojvm -nodisplay -nosplash -r "addpath('test');test_mih_with_linscan($db_file,8,$mih_results,$linscan_results');quit"
if [ $? != 0 ]; then
    echo "Could not run test_mih_with_linscan for some reason... Aborting"
    exit 1
fi
exit 0;

# Plots: If you run the following commands inside matlab, it will
# generate plots similar to the ones in the paper. (Note: this is not
# a bash script)
addpath matlab;
plot_time('cache/mih_64_1B.mat', 'cache/linscan_64_1B.mat');
plot_time('cache/mih_64_1B.mat', 'cache/linscan_64_1B.mat', [], 0, [.01 1000], [0 .2]);
plot_time('cache/mih_64_1B.mat', 'cache/linscan_64_1B.mat', [], .0002, [.01 1000], [0 20]);


addpath matlab;
plot_time('cache/mih_mlh_128_1B.mat', 'cache/linscan_mlh_128_1B.mat');
plot_time('cache/mih_mlh_128_1B.mat', 'cache/linscan_mlh_128_1B.mat', [], 0, [.01 1000], [0 1.1]);
plot_time('cache/mih_mlh_128_1B.mat', 'cache/linscan_mlh_128_1B.mat', [], .0002, [.01 1000], [0 30]);


addpath matlab;
plot_time('cache/mih_256_1B.mat', 'cache/linscan_256_1B.mat');
plot_time('cache/mih_256_1B.mat', 'cache/linscan_256_1B.mat', [], 0, [.01 1000], [0 1.1]);
plot_time('cache/mih_256_1B.mat', 'cache/linscan_256_1B.mat', [], .0002, [.01 1000], [0 30]);
