#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "gtmpi.h"

/*
    From the MCS Paper: The scalable, distributed dissemination barrier with only local spinning.

    type flags = record
        myflags : array [0..1] of array [0..LogP - 1] of Boolean
	partnerflags : array [0..1] of array [0..LogP - 1] of ^Boolean
	
    processor private parity : integer := 0
    processor private sense : Boolean := true
    processor private localflags : ^flags

    shared allnodes : array [0..P-1] of flags
        //allnodes[i] is allocated in shared memory
	//locally accessible to processor i

    //on processor i, localflags points to allnodes[i]
    //initially allnodes[i].myflags[r][k] is false for all i, r, k
    //if j = (i+2^k) mod P, then for r = 0 , 1:
    //    allnodes[i].partnerflags[r][k] points to allnodes[j].myflags[r][k]

    procedure dissemination_barrier
        for instance : integer :0 to LogP-1
	    localflags^.partnerflags[parity][instance]^ := sense
	    repeat until localflags^.myflags[parity][instance] = sense
	if parity = 1
	    sense := not sense
	parity := 1 - parity
*/
int Log(int val) {
    int i = 0;
    while (val >>= 1) i++;
    return ((1 << i) == val) ? i : i + 1;
}

int NUM_PROCESS;
void gtmpi_init(int num_threads){
    printf("init: %d\n", num_threads);
    NUM_PROCESS = num_threads;
}

void gtmpi_barrier(){
    int vpid;
    static MPI_Status stats; // dummpy stats for MPI_recieve

    MPI_Comm_rank(MPI_COMM_WORLD, &vpid);
    int num_round = Log(NUM_PROCESS);
    for (int round = 0; round < num_round; round++) {
        int destination = (vpid + (1 << round)) % NUM_PROCESS;

        // notify the destination node
        MPI_Send(NULL, 0, MPI_INT, destination, 1, MPI_COMM_WORLD);

        // wait for response from source node coming to this node
        int source = (vpid + NUM_PROCESS - (1 << round)) % NUM_PROCESS;
        MPI_Recv(NULL, 0, MPI_INT, source, 1, MPI_COMM_WORLD, &stats);
    }

}

void gtmpi_finalize(){

}
