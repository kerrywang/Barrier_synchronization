cmake_minimum_required (VERSION 2.6)
project (oms-aos-barrier)
set(CMAKE_C_COMPILER mpicc)
find_package(OpenMP)



add_executable(mpi hello_mpi.c)
add_executable(openmp hello_openmp.c)

if(OpenMP_CXX_FOUND)
    target_link_libraries(openmp PUBLIC OpenMP::OpenMP_CXX)
endif()