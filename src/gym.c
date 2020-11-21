// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project gym resource source
//   Date    -   11-20-20
//
// ########################################## 


#include <stdlib.h>
#include <stdio.h>

#include "gym.h"


Weight* weight_init(short plate_array[8], float total_weight) {
    Weight* weight = malloc(sizeof(Weight));

    if(weight == NULL) {
        perror("gym.c weight_init malloc()");
        return NULL;
    }

    if(plate_array != NULL) {
        for(int i=0; i<8; ++i) weight->num_plates[i] = 0;
    } else {
        for(int i=0; i<8; ++i) weight->num_plates[i] = plate_array[i];
    }

    weight->total_weight = total_weight;
    return weight;
}

int weight_del(Weight *weight) {
    free(weight);
    weight = NULL;
}