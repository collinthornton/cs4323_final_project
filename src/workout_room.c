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

#include "workout_room.h"
#include "resource_manager.h"
#include "client.h"

//#define TEST_WORKOUT_ROOM   // UNCOMMENT TO TEST WITH main()



void test_resource_manager(void) {   
    char line[1024];

    //////////////////////////
    //
    // TEST CLEAR FUNCTION
    //

    clearWeightFile();


    //////////////////////////
    //
    // TEST WRITING FUNCITONS
    //

    pid_t pid = 2;
    int weights[8] = {2, 2, 2, 2, 2, 2, 2, 2};

    printf("Process ID: %d\r\n\r\n", pid);
    

    char in[5];

    // TEST writeWeigthAllocation (section 2 of input file). Deletes weight
    //writeWeightAllocation(pid+4, weight);



    printf("Press enter to write a weight request\r\n");
    fflush(stdin);
    fgets(in, 2, stdin);

    // TEST writeWeightRequest (section 3 of input file). Deletes weight
    Weight *weight = weight_init(weights);
    writeWeightRequest(pid, weight);

    weight = getAvailableWeights();
    weight_to_string(weight, line);
    weight_del(weight);
    printf("currently availble: %s\r\n\r\n\r\n", line);



    printf("Press enter to grant the weight request\r\n");
    fflush(stdin);
    fgets(in, 2, stdin);

    // TEST removeWeightAllocation (section 2 of input file). Deletes weight
    //weight = weight_init(weights);
    grantWeightRequest(pid);

    weight = getAvailableWeights();
    weight_to_string(weight, line);
    weight_del(weight);
    printf("currently availble: %s\r\n\r\n\r\n", line);


    printf("Press enter to remove the weight allocation\r\n");
    fflush(stdin);
    fgets(in, 2, stdin);
    
    
    // TEST removeWeightRequest (section 3 of input file). Deletes weight
    pid_t request_pid = pid;
    weight = weight_init(weights);
    removeWeightAllocation(request_pid, weight);
    weight_del(weight);

    weight = getAvailableWeights();
    weight_to_string(weight, line);
    weight_del(weight);
    printf("currently availble: %s\r\n\r\n\r\n", line);


    //////////////////////////
    //
    // TEST READING FUNCTIONS
    //

    // TEST getGymResources (section 1 of input file)
    Weight *database = getGymResources();
    if(database == NULL) exit(1);
    weight_to_string(database, line);
    weight_del(database);

    printf("\r\nGYM RESOURCES\r\n\r\n");
    printf("%s\r\n", line);    


    // TEST getWeightAllocation (section 2 of input file)
    WeightMatrix *allocationMatrix = getWeightAllocation();
    if(allocationMatrix == NULL) exit(1);
    weight_matrix_to_string(allocationMatrix, line);
    weight_matrix_del(allocationMatrix);

    printf("\r\n----------\r\n\r\n");
    printf("WEIGHT ALLOCATION\r\n\r\n");
    printf("%s\r\n", line);

    
    // TEST getWeightRequest (section 3 of input file)
    WeightMatrix *requestMatrix = getWeightRequest();
    if(requestMatrix == NULL) exit(1);
    weight_matrix_to_string(requestMatrix, line);
    weight_matrix_del(requestMatrix);

    printf("\r\n----------\r\n\r\n");
    printf("WEIGHT REQUEST\r\n\r\n");
    printf("%s\r\n", line);

    printf("\r\n----------\r\n\r\n");    
}




#ifdef TEST_WORKOUT_ROOM  
int main(int argc, char** argv) {
    test_resource_manager();
}
#endif // TEST_WORKOUT_ROOM