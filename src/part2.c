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
#include <time.h>

#include "part2.h"
#include "entrance.h"
#include "workout_room.h"
#include "deadlock.h"

#define NUM_CLIENTS  MAX_CLIENTS
#define NUM_TRAINERS MAX_TRAINERS

#define NUM_COUCHES 3


void start_part2(){

    // Set the random generator seed
    srand(time(0));

    //////////////////////////
    //
    // Initalized semaphores and shared memory
    //

    if(init_shared_gym(NUM_COUCHES) == 1) exit(1);
    init_resource_manager();
    init_trainer_sem();

    sem_close(shared_gym_sem);
    close_resource_manager();
    close_trainer_sem();

    //TODO Get rid of this if unneeded
    //open_gym(3,3,5,0);

    //////////////////////////
    //
    // Launch child processes
    //

    printf("parent -> pid %d\r\n", getpid());

    printf("parent -> spawning %d clients\r\n", NUM_CLIENTS);
    pid_t client_pids[NUM_CLIENTS];
    for(int i=0; i<NUM_CLIENTS; ++i) client_pids[i] = client_start();

    printf("parent -> spawning %d trainers\r\n", NUM_TRAINERS);
    pid_t trainer_pids[NUM_TRAINERS];
    
    for(int i=0; i<NUM_TRAINERS; ++i) trainer_pids[i] = trainer_start();


    //////////////////////////
    //
    // Setup shared memory
    //

    open_shared_gym();   
    open_resource_manager();
    open_trainer_sem(); 
    
    Gym *gym = gym_init();
    update_gym(gym);   


    //////////////////////////
    //
    // Run parent tasks
    // - Deadlock detection, etc.
    // 

    sleep(2*gym->unit_time);
    update_gym(gym);

    while(gym->trainerList->len > 0) {

        pid_t deadlock_array[MAX_CLIENTS];
        int num_deadlocked = checkForDeadlock(deadlock_array);

        printf("\r\nDEADLOCKED CLIENTS      ");

        for(int i=0; i<num_deadlocked; ++i) {
            printf("%d ", deadlock_array[i]);
        }

        if(num_deadlocked == 0) {
            printf("NO DEADLOCKED PROCESSES\r\n\r\n");
        }
        else {
            printf("\r\n\r\n");    

            pid_t deadlock_victim;
            int least_sets = 9999;
            for(int i=0; i<num_deadlocked; ++i) {
                Client *tmp = client_list_find_pid(deadlock_array[i], gym->workoutList);
                if(tmp != NULL && (tmp->workout.total_sets - tmp->workout.sets_left) < least_sets) {
                    printf("parent -> searching client %d with %d of %d sets completed\r\n", tmp->pid, tmp->workout.total_sets-tmp->workout.sets_left, tmp->workout.total_sets);
                    deadlock_victim = tmp->pid;
                }
            }

            gym->deadlock_victim = deadlock_victim;
            printf("parent -> chose client %d as deadlock victim\r\n\r\n", deadlock_victim);
        }

        update_shared_gym(gym);
        sleep(5*gym->unit_time);
        update_gym(gym);
    }


    // test_workout_room();


    //////////////////////////
    //
    // Wait for child processes to exit
    //

    printf("parent -> waiting for processes to exit\r\n");
    for(int i=0; i<NUM_CLIENTS; ++i)  waitpid(client_pids[i], NULL, 0);
    for(int i=0; i<NUM_TRAINERS; ++i) waitpid(trainer_pids[i], NULL, 0);



    //////////////////////////
    //
    // Cleanup the mess
    //

    printf("parent -> destorying data\r\n");

    gym_del(gym);
    close_shared_gym();
    destroy_shared_gym();

    close_resource_manager();
    destroy_resource_manager();

    close_trainer_sem();
    destroy_trainer_sem();
}



int main(int argc, char **argv) {
    //test_deadlock_detection();
    start_part2();
}