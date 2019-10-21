#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <sys/utsname.h>
#include <time.h>
#include "gtmp.h"


int main(int argc, char **argv)
{
  int num_threads;
  // Serial code
  printf("This is the serial section\n");
  if (argc < 2){
    fprintf(stderr, "Usage: ./hello_world [NUM_THREADS]\n");
    exit(1);
  }


  num_threads = strtol(argv[1], NULL, 10);
  omp_set_dynamic(0);
  if (omp_get_dynamic()) {printf("Warning: dynamic adjustment of threads has been set\n");}
  omp_set_num_threads(num_threads);
  gtmp_init(num_threads);

    struct timespec tms;
    clock_gettime(CLOCK_REALTIME, &tms);

    double t0 = (double) tms.tv_nsec / 1e9;

#pragma omp parallel
  {
    int thread_num = omp_get_thread_num();
    struct utsname ugnm;
    
    num_threads = omp_get_num_threads();
    uname(&ugnm);

//    printf("Hello World from thread %d of %d, running on %s.\n", thread_num, num_threads, ugnm.nodename);
    gtmp_barrier();


//    printf("Hello World from thread %d of %d, running on %s.\n", thread_num, num_threads, ugnm.nodename);

  } // implied barrier
    clock_gettime(CLOCK_REALTIME, &tms);
    double t1 = tms.tv_nsec / 1e9;
    printf("used: %lf sec\n", t1 - t0);

    gtmp_finalize();
  // Resume serial code
  printf("Back in the serial section again\n");
  return 0;
}

