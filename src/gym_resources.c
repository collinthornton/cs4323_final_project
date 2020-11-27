// ##########################################
// 
//   Author  -   Collin Thornton / Robert Cook
//   Email   -   robert.cook@okstate.edu
//   Brief   -   Final Project gym resource source
//   Date    -   11-20-20
//
// ########################################## 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gym_resources.h"



//////////////////////////////
//
// Weight functions
//


Weight* weight_init(int plate_array[8]) {
    Weight* weight = malloc(sizeof(Weight));

    if(weight == NULL) {
        perror("gym.c weight_init malloc()");
        return NULL;
    }

    weight->total_weight = 0;
    if(plate_array == NULL) {
        for(int i=0; i<8; ++i) weight->num_plates[i] = 0;
    } else {
        for(int i=0; i<8; ++i) {
            weight->num_plates[i] = plate_array[i];
        }
    }

    weight->total_weight = weight_calc_total_weight(weight);
    return weight;
}

int weight_del(Weight *weight) {
    if(weight == NULL) return 1;
    free(weight);
    weight = NULL;

    return 0;
}


float weight_calc_total_weight(Weight *weight) {
    if(weight == NULL) return 0;

    float total_weight = 0;
    
    for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) {
        switch (i) {
            case TWO_HALF:
                total_weight += weight->num_plates[i]*2.5;
                break;
            case FIVE:
                total_weight += weight->num_plates[i]*5.0;
                break;
            case TEN:
                total_weight += weight->num_plates[i]*10.0;
                break;
            case FIFTEEN:
                total_weight += weight->num_plates[i]*15.0;
                break;
            case TWENTY:
                total_weight += weight->num_plates[i]*20.0;
                break;
            case TWENTY_FIVE:
                total_weight += weight->num_plates[i]*25.0;
                break;
            case THIRTY_FIVE:
                total_weight += weight->num_plates[i]*35.0;
                break;
            case FORTY_FIVE:
                total_weight += weight->num_plates[i]*45.0;
                break;
        }
    }
    return total_weight;
}

const char* weight_to_string(Weight *weight, char buffer[]) {
    buffer[0] = '\0';

    for(int j=TWO_HALF; j<=FORTY_FIVE; ++j) {
        sprintf(buffer+strlen(buffer), "%d,", weight->num_plates[j]);
    }    
    buffer[strlen(buffer)-1] = '\0';
    return buffer;
}