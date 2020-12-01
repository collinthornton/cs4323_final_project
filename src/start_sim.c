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
#include <errno.h>
#include <time.h>

#include "start_sim.h"

#define NUM_CLIENTS 2 //MAX_CLIENTS

void start_sim(const int num_trainers, const int num_couches, const bool boundary_case, const bool realistic, const bool detect_deadlock, const bool fix_deadlock, const bool trainer_log) {

    if(num_trainers < 3 || num_trainers > MAX_TRAINERS) {
        printf("Number of trainers must be between %d and %d\r\n\r\n", MIN_TRAINERS, MAX_TRAINERS);
        exit(-1);
    }

    if(num_couches < 3 || num_couches > MAX_COUCHES) {
        printf("Number of couches must be between %d and %d\r\n\r\n", MIN_COUCHES, MAX_COUCHES);
        exit(-1);
    }


    // Set the random generator seed
    srand(time(0));

    //////////////////////////
    //
    // Initalized semaphores and shared memory
    //

    if(init_shared_gym(num_couches, num_trainers, boundary_case, realistic, detect_deadlock, fix_deadlock, trainer_log) == 1) exit(1);
    init_resource_manager();
    init_trainer_sem();
    init_client_sem();
    initRecordBook();

    openRecordBook();
    clearRecordBook();

    //sem_close(shared_gym_sem);
    close_shared_gym();
    close_resource_manager();
    close_trainer_sem();
    close_client_sem();
    closeRecordBook();

    //////////////////////////
    //
    // Launch child processes
    //

    printf("parent -> pid %d\r\n", getpid());

    printf("parent -> spawning %d trainers\r\n", num_trainers);
    pid_t trainer_pids[num_trainers];
    for(int i=0; i<num_trainers; ++i) trainer_pids[i] = trainer_start();



    int num_clients = num_couches - 1;
    printf("parent -> spawning %d clients\r\n", num_clients);
    pid_t client_pids[num_clients];
    for(int i=0; i<num_clients; ++i) client_pids[i] = client_start();


    //////////////////////////
    //
    // Setup shared memory
    //

    open_shared_gym();   
    open_resource_manager();
    open_trainer_sem(); 
    open_client_sem();
    openRecordBook();
    
    Gym *gym = gym_init();
    update_gym(gym);   

    delay(2*gym->unit_time);

    close_shared_gym();
    close_resource_manager();
    close_trainer_sem();
    close_client_sem();
    closeRecordBook();
    gym_del(gym);

    client_start();
    client_start();

    open_shared_gym();
    open_resource_manager();
    open_trainer_sem();
    open_client_sem();
    openRecordBook();
    gym = gym_init();
    update_gym(gym);


    //////////////////////////
    //
    // Run parent tasks
    // - Deadlock detection, etc.
    // 

    printf("gym unit time %d\r\n", gym->unit_time);

    delay(15*gym->unit_time);
    
    update_gym(gym);

    int print_delay = 0;

    bool first_time = true;

    while(gym->trainerList->len > 0) {


        if(print_delay % 5 == 0) {
            printf("\r\nCOUCHES TAKEN: %d of %d\r\n", gym->waitingList->len, gym->maxCouches);
        }


        if((gym->detect_deadlock || gym->fix_deadlock) && print_delay % 5 == 0) {
    
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
                if(gym->fix_deadlock) {
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
            }
        }

        update_shared_gym(gym);
        delay(1*gym->unit_time);
        update_gym(gym);

        ++print_delay;
    }



    // test_workout_room();


    //////////////////////////
    //
    // Wait for child processes to exit
    //

    printf("parent -> waiting for processes to exit\r\n");
    for(int i=0; i<NUM_CLIENTS; ++i)  waitpid(client_pids[i], NULL, 0);
    for(int i=0; i<num_trainers; ++i) waitpid(trainer_pids[i], NULL, 0);



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

    close_client_sem();
    destroy_client_sem();

    closeRecordBook();
    destroyRecordBook();
}
