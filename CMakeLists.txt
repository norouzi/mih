cmake_minimum_required(VERSION 2.6)
project (mih)

set (CMAKE_CXX_FLAGS "-O2 -DNDEBUG")

set (MATLAB_LIB_PATH ${MATLABROOT}/bin/glnxa64)  

FIND_LIBRARY (MATLAB_MAT_LIBRARY mat ${MATLAB_LIB_PATH})  
FIND_LIBRARY (MATLAB_MX_LIBRARY mx ${MATLAB_LIB_PATH})  
FIND_LIBRARY (MATLAB_MEX_LIBRARY mex ${MATLAB_LIB_PATH})  

# FIND_LIBRARY (MATLAB_MAT_LIBRARY mat)
# FIND_LIBRARY (MATLAB_MX_LIBRARY mx)
# FIND_LIBRARY (MATLAB_MEX_LIBRARY mex)
set (MATLAB_INCLUDE_DIR ${MATLABROOT}/extern/include)
set (MATLAB_LIBRARIES ${MATLAB_MAT_LIBRARY} ${MATLAB_MX_LIBRARY} ${MATLAB_MEX_LIBRARY} )
MESSAGE (STATUS ${MATLAB_INCLUDE_DIR})
MESSAGE (STATUS ${MATLAB_LIBRARIES})

include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories (${MATLAB_INCLUDE_DIR})

add_executable (mih src/array32.cpp src/mihasher.cpp src/sparse_hashtable.cpp src/bucket_group.cpp interface/mih_interface.cpp)

target_link_libraries (mih ${MATLAB_LIBRARIES})


project (linscan)

add_executable (linscan src/linscan.cpp interface/linscan_interface.cpp)

target_link_libraries (linscan ${MATLAB_LIBRARIES})
