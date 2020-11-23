// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project part2 source
//   Date    -   11-20-20
//
// ########################################## 


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#include "part2.h"

#include "resource_manager.h"


int main(int argc, char** argv) {
    test_resource_manager();
}


void test_resource_manager(void) {   
    Weight *database;
    if((database = getGymResources()) == NULL) {
        exit(1);
    }

    WeightMatrix *allocationMatrix;
    if((allocationMatrix = getWeightAllocation()) == NULL) {
        exit(1);
    }


    char line[1024];
    weight_to_string(database, line);

    printf("GYM RESOURCES\r\n\r\n");
    printf("%s\r\n", line);


    weight_matrix_to_string(allocationMatrix, line);

    printf("\r\n----------\r\n\r\n");
    printf("WEIGHT ALLOCATION\r\n\r\n");
    printf("%s\r\n", line);


    pid_t pid = getpid();
    unsigned short weights[8] = {2, 2, 2, 2, 2, 2, 2, 2};

    Weight *weight = weight_init(weights);

    // weight_matrix_add_req(pid, weight, allocationMatrix);
    
    writeWeightAllocation(pid+4, weight);

    weight = weight_init(weights);
    writeWeightRequest(pid, weight);

    //weight = weight_init(weights);
    //removeWeightAllocation(820, weight);


    weight_del(database);
    //weight_del(weight);
    weight_matrix_del(allocationMatrix);
}