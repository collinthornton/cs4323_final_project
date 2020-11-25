// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project deadlock detection src
//   Date    -   11-23-20
//
// ########################################## 

#include <stdbool.h>

#include "deadlock_detection.h"


pid_t checkForDeadlock() {
    WeightMatrix *available = getGymResources();
    WeightMatrix *allocation = getWeightAllocation();
    WeightMatrix *request = getWEightRequest();

    WeightMatrix work = *available;

    // unsigned int num_procs = allocation

}