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


/**
 * @brief Allocate Weight struct on heap
 * @param plate_array (int[NUMBER_WEIGHTS]) Array with each indice a number of grip plates of corresponding PlateIndice. Set to NULL if not yet known
 * @return (Weight*) struct allocated on heap
 */
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


/**
 * @brief free a Weight struct from the heap
 * @param weight (Weight*) struct to be freed
 * @return (int) return code. negative on failure
 */
int weight_del(Weight *weight) {
    if(weight == NULL) return 1;
    free(weight);
    weight = NULL;

    return 0;
}


/**
 * @brief Calculate the total weight represented by the num_plates array
 * @param weight (Weight*) pointer to a Weight struct
 * @return (float) total weight represented by that struct
 */
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


/**
 * @brief stringify a weight struct
 * @param weight (Weight*) struct to be stringified
 * @param buffer (char[]) buffer to store string
 * @return (const char*) same as buffer
 */
const char* weight_to_string(Weight *weight, char buffer[]) {
    buffer[0] = '\0';

    for(int j=TWO_HALF; j<=FORTY_FIVE; ++j) {
        sprintf(buffer+strlen(buffer), "%d,", weight->num_plates[j]);
    }    
    buffer[strlen(buffer)-1] = '\0';
    return buffer;
}