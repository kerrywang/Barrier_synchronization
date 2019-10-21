#include <stdio.h>
#include <sys/utsname.h>
#include "mpi.h"
#include "gtmpi.h"

int main(int argc, char **argv)
{
  int my_id, num_processes;
  struct utsname ugnm;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

  gtmpi_init(num_processes);

  uname(&ugnm);


    gtmpi_barrier();
    printf("exiting the barrier %d \n", my_id);


//  printf("Hello World from thread %d of %d, running on %s.\n", my_id, num_processes, ugnm.nodename);
  MPI_Finalize();
  gtmpi_finalize();
  return 0;
}

