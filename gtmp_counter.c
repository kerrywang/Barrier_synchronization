#include <omp.h>
#include "gtmp.h"
#include <time.h>
#include <stdio.h>

/*
    From the MCS Paper: A sense-reversing centralized barrier

    shared count : integer := P
    shared sense : Boolean := true
    processor private local_sense : Boolean := true

    procedure central_barrier
        local_sense := not local_sense // each processor toggles its own sense
	if fetch_and_decrement (&count) = 1
	    count := P
	    sense := local_sense // last processor toggles global sense
        else
           repeat until sense = local_sense
*/
int count, sense;

void gtmp_init(int num_threads){
    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);
    count = num_threads;
    sense = 1;
}

void gtmp_barrier(){
    int local_sense = 0;
//    fprintf(stdout, "i am thread %d, reporting for finishing\n", omp_get_thread_num());
    if (__sync_fetch_and_sub(&count, 1) == 1) {
        count = omp_get_num_threads();
        sense = local_sense;
    } else {
        while (local_sense != sense) {}
    }

//    printf("thread id: %d, exiting barrier\n", omp_get_thread_num());
}

void gtmp_finalize(){

}
