// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project deadlock detection src
//   Date    -   11-23-20
//
// ########################################## 

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "deadlock.h"
#include "vector.h"

#define TEST_DEADLOCK_DETECTION // UNCOMMENT TO TEST DEADLOCK DETECTION WITH main()


int checkForDeadlock(pid_t deadlock_array[]) {
    // BASED ON ALGORITHM DESCRIBED ON PG 339 OF TEXTBOOK


    // Step 1

    Weight *available = getAvailableWeights();
    WeightMatrix *allocation = getWeightAllocation();
    WeightMatrix *request = getWeightRequest();

    Weight work = *available;

    unsigned int num_procs = allocation->num_rows;
    bool finished[num_procs];

    for(int i=0; i<num_procs; ++i) {
        if(vector_zero(allocation->rows[i].weight->num_plates, NUMBER_WEIGHTS)) {
            finished[i] = true;
        }
        else {
            finished[i] = false;
        }
    }


    bool exec_step_three = false;
    do {
        // Step 2

        int row;
        for(int i=0; i<num_procs; ++i) {
            if(!finished[i] && vector_less_than_equal(request->rows[i].weight->num_plates, work.num_plates, NUMBER_WEIGHTS)) {
                exec_step_three = true;
                row = i;
                break;
            } else  {
                exec_step_three = false;
            }
        }

        if(exec_step_three) {
            // Step 3

            vector_add(work.num_plates, allocation->rows[row].weight->num_plates, NUMBER_WEIGHTS);
            finished[row] = true;
        }
    } while(exec_step_three == true);
        

    // Step 4

    int num_deadlocked=0;
    for(int i=0; i<num_procs; ++i) {
        if(!finished[i]) {
            if(deadlock_array != NULL) deadlock_array[num_deadlocked] = allocation->rows[i].pid;
            ++num_deadlocked;
        }
    }

    if(deadlock_array != NULL) {
        for(int i=num_deadlocked; i<num_procs; ++i) {
            deadlock_array[i] = -1;
        }
    }

    weight_del(available);
    weight_matrix_del(allocation);
    weight_matrix_del(request);

    return num_deadlocked;
}




#ifdef TEST_DEADLOCK_DETECTION

int main(int argc, char** argv) {
    // WE WILL ASSUME THE WEIGHT FILE IS PRELOADED AND AS SUCH NOT CLEAR IT

    WeightMatrix *allocation = getWeightAllocation();
    int num_procs = allocation->num_rows;

    weight_matrix_del(allocation);

    pid_t deadlock_array[num_procs];
    int num_deadlocked = checkForDeadlock(deadlock_array);

    printf("\r\n");
    
    for(int i=0; i<num_deadlocked; ++i) {
        printf("%d\r\n", deadlock_array[i]);
    }
    printf("\r\n");
}

#endif // TEST_DEADLOCK_DETECTION