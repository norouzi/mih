cmake_minimum_required(VERSION 2.6)
project (mih)

# set (MATLABROOT /opt/matlab-714)
set (MATLAB_LIB_PATH ${MATLABROOT}/bin/glnxa64)
set (CMAKE_CXX_FLAGS "-O3 -DNDEBUG")

FIND_LIBRARY (MATLAB_MAT_LIBRARY mat ${MATLAB_LIB_PATH})
FIND_LIBRARY (MATLAB_MX_LIBRARY mx ${MATLAB_LIB_PATH})
FIND_LIBRARY (MATLAB_MEX_LIBRARY mex ${MATLAB_LIB_PATH})
set (MATLAB_INCLUDE_DIR ${MATLABROOT}/extern/include)
set (MATLAB_LIBRARIES ${MATLAB_MAT_LIBRARY} ${MATLAB_MX_LIBRARY} ${MATLAB_MEX_LIBRARY} )
MESSAGE (STATUS ${MATLAB_INCLUDE_DIR})
MESSAGE (STATUS ${MATLAB_LIBRARIES})

include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories (${MATLAB_INCLUDE_DIR})

add_executable (mih array32.cpp mihasher.cpp sparse_hashtable.cpp bucket_group.cpp interface/mih_interface.cpp)

target_link_libraries (mih ${MATLAB_LIBRARIES})


project (linscan)

add_executable (linscan linscan.cpp interface/linscan_interface.cpp)

target_link_libraries (linscan ${MATLAB_LIBRARIES})
