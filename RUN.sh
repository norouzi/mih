# add matlab to the PATH if it is not already there
# download the data, and create data/inria/ folder that includes ANN_SIFT1B folder and files

# if number of bits (nb) is not provided from outside, set it to 64
if [ -z "$nb" ]
then
    nb=64
fi

if [ -z "$hashfunc" ]
then
    hashfunc="lsh"
fi

if [ -z "$Q0" ]
then
    Q0=0
    Q1=10000
fi

if [ -z "$R" ]
then
    R=0
fi

if [ -z "$HUGE" ]  # this is page size for libhugetlbfs
then
    HUGE=0
fi

echo $nb

# some sanity checks
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

build_dir="build_`hostname`"
cache_dir="cache_`hostname`"

# compilation
mkdir -p $build_dir
mkdir -p $cache_dir
mkdir -p $cache_dir/$nb

cd $build_dir
cmake ..
make
cd ..

# Multi-index hashing is being run for 12 different subsets of the
# binary codes (10K 100K 1M 2M 5M 10M 20M 100M 200M 500M 1B
# items).
n[1]=10000
n[2]=100000
n[3]=1000000
n[4]=2000000
n[5]=5000000
n[6]=10000000
n[7]=20000000
n[8]=50000000
n[9]=100000000
n[10]=200000000
n[11]=500000000
n[12]=1000000000

# Number of substrings to be used for each case is stored in a
# bash array m.
if [ "$nb" == "64" ]
then
    m[1]=5
    m[2]=4
    m[3]=4
    m[4]=3
    m[5]=3
    m[6]=3
    m[7]=3
    m[8]=2
    m[9]=2
    m[10]=2
    m[11]=2
    m[12]=2
else
    if [ "$nb" == "128" ]
    then
	m[1]=10
	m[2]=8
	m[3]=8
	m[4]=6
	m[5]=6
	m[6]=5
	m[7]=5
	m[8]=5
	m[9]=5
	m[10]=4
	m[11]=4
	m[12]=4
    else
	if [ "$nb" == "256" ]
	then
	    m[1]=19
	    m[2]=15
	    m[3]=13
	    m[4]=12
	    m[5]=11
	    m[6]=11
	    m[7]=10
	    m[8]=10
	    m[9]=10
	    m[10]=9
	    m[11]=9
	    m[12]=8
	else
	    echo "nb == $nb is not supported"
	fi
    fi
fi

db_file="codes/"$hashfunc"/"$hashfunc"_"$nb"_sift_1B.mat"
mih_results="$cache_dir/$nb/mih_"$hashfunc"_"$Q0"_"$Q1"_1B_R"$R".h5"
linscan_results="$cache_dir/$nb/linscan_"$Q0"_"$Q1"_1B.h5"

if [ "$HUGE" != "0" ]
    echo "*** HUGE $HUGE ***"
fi

for ((nm=1; nm<=12; nm=nm+1))
do
    for ((K=1; K<=1000; K=K*10))
    do
	echo $build_dir/mih $db_file $mih_results -B $nb -Q $Q0 $Q1 -K $K \
            -N ${n[$nm]} -m ${m[$nm]} -R $R
        if [ "$HUGE" = "0" ]
        then
            $build_dir/mih $db_file $mih_results -B $nb -Q $Q0 $Q1 -K $K \
                -N ${n[$nm]} -m ${m[$nm]} -R $R
        else
            LD_PRELOAD=libhugetlbfs.so HUGETLB_MORECORE=$HUGE $build_dir/mih \
                $db_file $mih_results -B $nb -Q $Q0 $Q1 -K $K -N ${n[$nm]} \
                -m ${m[$nm]} -R $R
        fi
    done
    if [ $? != 0 ]; then
	echo "Could not run mih for some reason... Aborting"
	exit 1
    fi
done

exit 0
