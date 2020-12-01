// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project part3 driver
//   Date    -   11-30-20
//
// ########################################## 


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "start_sim.h"



int main(int argc, char **argv) {
    printf("\r\nCS4323 FINAL PROJECT GROUP D\r\n");
    printf("GYM SIMULATOR\r\n\r\n");
    printf("3rd Driver File -> Parts b, d, & e\r\n\r\n");
    printf("Collin Thornton\r\nRobert Cook\r\nTyler Krebs\r\n\r\n");
    printf("Usage: ./part3 <NUM_TRAINERS> <NUM_COUCHES>\n\n");

    if(argc != 3) {
        exit(-1);
    }


    int num_trainers = 0;
    int num_couches = 0;

    if((num_trainers = strtol(argv[1], NULL, 10)) == 0 && errno != 0) {
        exit(-1);
    }

    if((num_couches = strtol(argv[2], NULL, 10)) == 0 && errno != 0) {
        exit(-1);
    }


    start_sim(num_trainers, num_couches, true, true, true, true, true);
}