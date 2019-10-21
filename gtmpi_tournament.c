#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "gtmpi.h"

/*
    From the MCS Paper: A scalable, distributed tournament barrier with only local spinning

    type round_t = record
        role : (winner, loser, bye, champion, dropout)
	opponent : ^Boolean
	flag : Boolean
    shared rounds : array [0..P-1][0..LogP] of round_t
        // row vpid of rounds is allocated in shared memory
	// locally accessible to processor vpid

    processor private sense : Boolean := true
    processor private vpid : integer // a unique virtual processor index

    //initially
    //    rounds[i][k].flag = false for all i,k
    //rounds[i][k].role =
    //    winner if k > 0, i mod 2^k = 0, i + 2^(k-1) < P , and 2^k < P
    //    bye if k > 0, i mode 2^k = 0, and i + 2^(k-1) >= P
    //    loser if k > 0 and i mode 2^k = 2^(k-1)
    //    champion if k > 0, i = 0, and 2^k >= P
    //    dropout if k = 0
    //    unused otherwise; value immaterial
    //rounds[i][k].opponent points to
    //    round[i-2^(k-1)][k].flag if rounds[i][k].role = loser
    //    round[i+2^(k-1)][k].flag if rounds[i][k].role = winner or champion
    //    unused otherwise; value immaterial
    procedure tournament_barrier
        round : integer := 1
	loop   //arrival
	    case rounds[vpid][round].role of
	        loser:
	            rounds[vpid][round].opponent^ :=  sense
		    repeat until rounds[vpid][round].flag = sense
		    exit loop
   	        winner:
	            repeat until rounds[vpid][round].flag = sense
		bye:  //do nothing
		champion:
	            repeat until rounds[vpid][round].flag = sense
		    rounds[vpid][round].opponent^ := sense
		    exit loop
		dropout: // impossible
	    round := round + 1
	loop  // wakeup
	    round := round - 1
	    case rounds[vpid][round].role of
	        loser: // impossible
		winner:
		    rounds[vpid[round].opponent^ := sense
		bye: // do nothing
		champion: // impossible
		dropout:
		    exit loop
	sense := not sense
*/

enum ROLE {winner, loser, bye, champion, dropout, imaterial};

typedef struct round {
    enum ROLE role;
    uint opponent;
    uint flag;
} round_t;


int Log(int val) {
    int i = 0;
    while (val >>= 1) i++;
    return ((1 << i) == val) ? i : i + 1;
}

int NUM_PROCESS;
int NUM_ROUND;
round_t **rounds;

void gtmpi_init(int num_threads){
    NUM_PROCESS = num_threads;
    NUM_ROUND = Log(num_threads) + 1;
    rounds = malloc(NUM_PROCESS * sizeof(round_t*));

    int i = 0, k = 0;
    for (i = 0; i< NUM_PROCESS; i++) {
        rounds[i] = malloc(NUM_ROUND * sizeof(round_t));
        for (k = 0; k < NUM_ROUND; k++) {
            rounds[i][k].flag = 0;
            if (k == 0) rounds[i][k].role =  dropout;
            else if (k > 0 && i % (1 << k) == 0 && (i + (1 << (k - 1))) < NUM_PROCESS && (1 << k) < NUM_PROCESS) {
                rounds[i][k].role =  winner;
            } else if (k > 0 && i % (1 << k) == 0 && ((i + 1) << (k - 1)) >= NUM_PROCESS) {
                rounds[i][k].role =  bye;
            } else if (k > 0 && i % (1 << k) == 1 << (k - 1)) {
                rounds[i][k].role =  loser;
            } else if (k > 0 && i == 0 && (1 << k) >= NUM_PROCESS) {
                rounds[i][k].role =  champion;
            } else {
                rounds[i][k].role =  imaterial;
            }

            //rounds[i][k].opponent points to
            //    round[i-2^(k-1)][k].flag if rounds[i][k].role = loser
            //    round[i+2^(k-1)][k].flag if rounds[i][k].role = winner or champion
            //    unused otherwise; value immaterial
            if (rounds[i][k].role ==  loser) {
                rounds[i][k].opponent = i - (1 << (k - 1));
            }

            if (rounds[i][k].role ==  winner || rounds[i][k].role ==  champion) {
                rounds[i][k].opponent = i + (1 << (k-1));
            }

        }
    }

}

void gtmpi_barrier(){
    int round = 1, vpid = 0;
    MPI_Status stat;

    MPI_Comm_rank(MPI_COMM_WORLD, &vpid);
    printf("entering barrier at vid: %d\n", vpid);
    while (1) {
        if (rounds[vpid][round].role == loser) {
            MPI_Send(NULL, 0, MPI_INT, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD);
            MPI_Recv(NULL, 0, MPI_INT, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD, &stat);
            break;
        } else if (rounds[vpid][round].role ==  winner) {
            MPI_Recv(NULL, 0, MPI_INT, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD, &stat);
        } else if (rounds[vpid][round].role ==  champion) {
            MPI_Recv(NULL, 0, MPI_INT, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD, &stat);
            MPI_Send(NULL, 0, MPI_INT, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD);
            break;
        }


        round ++;
    }

    while (1) {
        round --;
        if (rounds[vpid][round].role ==  winner) {
            MPI_Send(NULL, 0, MPI_INT, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD);
        } else if (rounds[vpid][round].role ==  dropout) {
            break;
        }
    }

}

void gtmpi_finalize(){
    int i;
    for (i = 0; i < NUM_PROCESS; i++) {
        free(rounds[i]);
    }
    free(rounds);
}
