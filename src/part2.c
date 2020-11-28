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

#define NUM_CLIENTS 1 //MAX_CLIENTS
#define NUM_TRAINERS 1 //MAX_TRAINERS

#define NUM_COUCHES 3


void start_part2(){
    if(init_shared_gym(NUM_COUCHES) == 1) exit(1);
    clearWeightFile();


    printf("parent -> pid %d\r\n", getpid());

    //open_gym(3,3,5,0);
    //test_workout_room();

    //test_deadlock_detection();

    printf("parent -> spawning %d clients\r\n", NUM_CLIENTS);
    pid_t client_pids[NUM_CLIENTS];
    for(int i=0; i<NUM_CLIENTS; ++i) client_pids[i] = client_start();

    printf("parent -> spawning %d trainers\r\n", NUM_TRAINERS);
    pid_t trainer_pids[NUM_TRAINERS];
    
    for(int i=0; i<NUM_TRAINERS; ++i) trainer_pids[i] = trainer_start();

    open_shared_gym();    
    
    Gym *gym = gym_init();
    update_gym(gym);    

    // DO PARENT STUFF
    sleep(5);

    char buffer[BUFFER_SIZE*(NUM_CLIENTS+1)] = "\0";

    update_gym(gym);
    trainer_list_to_string(gym->trainerList, buffer);

    printf("\r\n\r\nTRAINER LIST\r\n");
    printf("%s\r\n", buffer);
    //printf("parent trainer list -> %d\r\n", gym->arrivingList->HEAD->node->current_trainer.pid);




    // WAIT FOR PROCESS TO EXIT
    printf("parent -> waiting for processes to exit\r\n");
    for(int i=0; i<NUM_CLIENTS; ++i)  waitpid(client_pids[i], NULL, 0);
    for(int i=0; i<NUM_TRAINERS; ++i) waitpid(trainer_pids[i], NULL, 0);

    //test_part2();

    gym_del(gym);
    close_shared_gym();
    destroy_shared_gym();
}


void test_part2() {

    pid_t pids[NUM_CLIENTS];

    printf("parent pid: %d\r\n", getpid());
    for(int i=0; i<NUM_CLIENTS; ++i) pids[i] = client_start();

    Trainer *trainer = trainer_init(getpid(), pids[0], FREE);
    
    Gym *gym = gym_init();
    trainer_list_add_trainer(trainer, gym->trainerList);

    open_shared_gym();

    update_shared_gym(gym);
    update_gym(gym);
    printf("parent %d -> unit time %d\r\n", getpid(), gym->unit_time);


    sleep(2);

    char buffer[BUFFER_SIZE*(NUM_CLIENTS+1)] = "\0";

    update_gym(gym);
    client_list_to_string(gym->arrivingList, buffer);

    printf("parent %d -> ARRIVING LIST\r\n", getpid());
    printf("parent %d -> %s\r\n", getpid(), buffer);
    //printf("parent trainer id -> %d\r\n", gym->arrivingList->HEAD->node->current_trainer.pid);


    //update_gym(gym);
    client_list_to_string(gym->waitingList, buffer);

    printf("parent %d -> WAITING LIST\r\n", getpid());
    printf("parent %d -> %s\r\n", getpid(), buffer);


    gym_del(gym);
    close_shared_gym();
}

int main(int argc, char **argv) {
    start_part2();
}