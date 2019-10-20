#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include "gtmp.h"

/*
    From the MCS Paper: A scalable, distributed tree-based barrier with only local spinning.

    type treenode = record
        parentsense : Boolean
	parentpointer : ^Boolean
	childpointers : array [0..1] of ^Boolean
	havechild : array [0..3] of Boolean
	childnotready : array [0..3] of Boolean
	dummy : Boolean //pseudo-data

    shared nodes : array [0..P-1] of treenode
        // nodes[vpid] is allocated in shared memory
        // locally accessible to processor vpid
    processor private vpid : integer // a unique virtual processor index
    processor private sense : Boolean

    // on processor i, sense is initially true
    // in nodes[i]:
    //    havechild[j] = true if 4 * i + j + 1 < P; otherwise false
    //    parentpointer = &nodes[floor((i-1)/4].childnotready[(i-1) mod 4],
    //        or dummy if i = 0
    //    childpointers[0] = &nodes[2*i+1].parentsense, or &dummy if 2*i+1 >= P
    //    childpointers[1] = &nodes[2*i+2].parentsense, or &dummy if 2*i+2 >= P
    //    initially childnotready = havechild and parentsense = false
	
    procedure tree_barrier
        with nodes[vpid] do
	    repeat until childnotready = {false, false, false, false}
	    childnotready := havechild //prepare for next barrier
	    parentpointer^ := false //let parent know I'm ready
	    // if not root, wait until my parent signals wakeup
	    if vpid != 0
	        repeat until parentsense = sense
	    // signal children in wakeup tree
	    childpointers[0]^ := sense
	    childpointers[1]^ := sense
	    sense := not sense
*/
typedef struct TreeNode {
    bool* child_pointer[2];
    bool parent_sense;
    bool* parent_pointer;
    bool have_child[4];
    bool child_not_ready[4];
    bool dummy;
} TreeNode;

TreeNode *shared_nodes;  // nodes[vpid] is allocated in shared memory locally accessible to processor vpid



void gtmp_init(int num_threads){
    shared_nodes = malloc(sizeof(TreeNode) * num_threads);
    for (int i = 0; i < num_threads; i++) {
        // we need to prevent false sharing. force spin variables on seperate lines
        posix_memalign((void**)&shared_nodes[i], LEVEL1_DCACHE_LINESIZE, LEVEL1_DCACHE_LINESIZE);
        shared_nodes[i].parent_sense = false;
        for (int j = 0; j < 4; j++) {
            shared_nodes[i].have_child[j] = ((4 * i + j + 1) < num_threads);
            shared_nodes[i].child_not_ready[j] = shared_nodes[i].have_child[j];
        }

        if (i == 0) shared_nodes[i].parent_pointer = &shared_nodes[i].dummy;
        else shared_nodes[i].parent_pointer = &shared_nodes[floor((i-1)/4)].childnotready[(i-1) % 4];

        if ((2 * i + 2) < num_threads) {
            shared_nodes[i].child_pointer[0] = &shared_nodes[2 * i + 1].parent_sense;
            shared_nodes[i].child_pointer[1] =  &shared_nodes[2 * i + 2].parent_sense;
        } else {
            shared_nodes[i].child_pointer[0] = &shared_nodes[i].dummy;
            shared_nodes[i].child_pointer[1] = &shared_nodes[i].dummy;
        }

    }
}

void _copy_array(bool* to_copy, bool* )

void gtmp_barrier(){
    int vpid = omp_get_thread_num();
    bool local_sense = true;

    // spin until the child are all ready
    while (shared_nodes[vpid].child_not_ready[0] || shared_nodes[vpid].child_not_ready[1] || shared_nodes[vpid].child_not_ready[2] || shared_nodes[vpid].child_not_ready[3]) {}

    // prepare for next barrier
    for (int i = 0; i < 4; i++) {
        shared_nodes[vpid].child_not_ready[i] = shared_nodes[vpid].have_child[i];
     }

    *shared_nodes[vpid].parent_pointer = false;  //let parent know I'm ready

    // if not root, wait until my parent signals wakeup
    if (vpid != 0) {
        while (local_sense != shared_nodes[vpid].parent_sense);
    }

    *shared_nodes[vpid].child_pointer[0] = local_sense;
    *shared_nodes[vpid].child_pointer[1] = local_sense;
}

void gtmp_finalize(){
    free(shared_nodes);
}
