// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   robert.cook@okstate.edu
//   Brief   -   Final Project part2 source
//   Date    -   11-20-20
//
// ########################################## 


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "part2.h"
#include "entrance.h"
#include "workout_room.h"
#include "deadlock.h"

#define NUM_CLIENTS MAX_CLIENTS




void start_part2(){
    sem_init(&shared_gym_sem, 1, 1);

    //First open the gym, but do not use semaphores
    //open_gym(3,3,5,0);
    //test_workout_room();

    //test_deadlock_detection();
    test_part2();
}


void test_part2() {
    int num_couches = 3;
    if(init_shared_gym(num_couches) == 1) exit(1);

    pid_t pids[NUM_CLIENTS];

    printf("parent pid: %d\r\n", getpid());
    for(int i=0; i<NUM_CLIENTS; ++i) pids[i] = client_start();

    Gym *gym = gym_init();
    open_shared_gym();
    update_gym(gym);
    printf("%d\r\n", gym->unit_time);


    sleep(2);

    char buffer[BUFFER_SIZE*(NUM_CLIENTS+1)] = "\0";

    update_gym(gym);
    client_list_to_string(gym->arrivingList, buffer);

    printf("\r\n\r\nARRIVING LIST\r\n");
    printf("%s\r\n", buffer);
    printf("parent trainer id -> %d\r\n", gym->arrivingList->HEAD->node->current_trainer.pid);


    update_gym(gym);
    client_list_to_string(gym->waitingList, buffer);

    printf("\r\n\r\nWAITING LIST\r\n");
    printf("%s\r\n", buffer);


    printf("CHANGING CLIENT STATE\r\n");
    update_gym(gym);
    Client *client = client_list_find_pid(pids[0], gym->arrivingList);

    client->state = WAITING;
    update_shared_gym(gym);

    gym_del(gym);
    close_shared_gym();
}

int main(int argc, char **argv) {
    start_part2();
}