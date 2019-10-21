#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"

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


static MPI_Status* status_array;
static int P;

void gtmpi_init(int num_threads){
  P = num_threads;
  status_array = (MPI_Status*) malloc((P - 1) * sizeof(MPI_Status));
}

void gtmpi_barrier(){
  int vpid, i;
  static MPI_Status dummy_stats;
  MPI_Comm_rank(MPI_COMM_WORLD, &vpid);
  
  if (vpid == 0) {
      for (i = 1; i < P; i++)
          MPI_Recv(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD, &dummy_stats);
      for (i = 1; i < P; i++)
          MPI_Send(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD);
  } else {
      MPI_Send(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD);
      MPI_Recv(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, &dummy_stats);
  }
}

void gtmpi_finalize(){
  if(status_array != NULL){
    free(status_array);
  }
}

