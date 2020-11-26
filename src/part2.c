#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "entrance.h"
#include "workout_room.h"
#include "deadlock.h"

#define NUM_CLIENTS 1

void start_part2(){
    //First open the gym, but do not use semaphores
    //open_gym(3,3,5,0);
    //test_workout_room();

    //test_deadlock_detection();

    int num_couches = 3;
    init_shared_gym(num_couches);


    Gym *sharedGymObject = get_shared_gym();
    printf("ARRIVING CLIENT LIST SIZE: %d\r\n", sharedGymObject->arrivingList->len);


    pid_t pids[NUM_CLIENTS];

    for(int i=0; i<NUM_CLIENTS; ++i) pids[i] = client_start();

    sleep(1);

    char buffer[BUFFER_SIZE*NUM_CLIENTS] = "\0";
    client_list_to_string(sharedGymObject->arrivingList, buffer);

    printf("%s\r\n", buffer);
    clean_shared_gym(sharedGymObject);
}


int main(int argc, char **argv) {
    start_part2();
}