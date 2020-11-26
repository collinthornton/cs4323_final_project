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
    if(init_shared_gym(num_couches) == 1) exit(1);

    Gym *gym = gym_init();
    SharedGym *sharedGymObject = get_shared_gym();

    printf("%d\r\n", sharedGymObject->unit_time);

    pid_t pids[NUM_CLIENTS];

    for(int i=0; i<NUM_CLIENTS; ++i) pids[i] = client_start();

    sleep(1);

    char buffer[BUFFER_SIZE*NUM_CLIENTS] = "\0";

    update_gym(gym, sharedGymObject);
    client_list_to_string(gym->arrivingList, buffer);
    //client_to_string(&sharedGymObject->arrivingList[0], buffer);
    printf("\r\n\r\nARRIVING LIST\r\n");
    printf("%s\r\n", buffer);


    update_gym(gym, sharedGymObject);
    client_list_to_string(gym->waitingList, buffer);
    //client_list_to_string(sharedGymObject->waitingList, buffer);
    printf("\r\n\r\nWAITING LIST\r\n");
    printf("%s\r\n", buffer);
    gym_del(gym);
    clean_shared_gym(sharedGymObject);
}


int main(int argc, char **argv) {
    start_part2();
}