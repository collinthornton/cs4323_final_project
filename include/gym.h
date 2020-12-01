// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project Gym include
//   Date    -   11-15-20
//
// ########################################## 


#ifndef GYM_H
#define GYM_H

#include <stdbool.h>
#include <fcntl.h>

#include "trainer.h"
#include "client.h"

#define BUFFER_SIZE 1024

// #define VERBOSE


/**
 * @brief Non-pointer version of Gym struct. Used for shared memory. Represents total resources of gym
 */
typedef struct {
    // List of client on a couch (in waiting room)
    Client waitingList[MAX_CLIENTS];

    // List of clients arriving
    Client arrivingList[MAX_CLIENTS];

    // List of client training
    Client workoutList[MAX_CLIENTS];

    // List of trainers
    Trainer trainerList[MAX_TRAINERS];


    // PID of deadlock victim chosen by paren
    pid_t deadlock_victim;

    // Constants of gym
    int maxCouches;
    int num_trainers;
    int unit_time; // milliseconds


    // Execution flags
    bool boundary_case;
    bool realistic;
    bool fix_deadlock;
    bool detect_deadlock;
    bool trainer_log;

} SharedGym;


/**
 * @brief Represents total resources of gym as related to process
 */
typedef struct {
    // List of clients on a couch
    ClientList* waitingList;

    // List of arriving clients
    ClientList* arrivingList;

    // List of training clients
    ClientList* workoutList;

    // List of trainers
    TrainerList* trainerList;


    // PID of deadlock victim chosen by parent
    pid_t deadlock_victim; 

    // Constants of gym
    int maxCouches;
    int num_trainers;
    int unit_time; // milliseconds   

    // Execution flags
    bool boundary_case;
    bool realistic;
    bool fix_deadlock;
    bool detect_deadlock;
    bool trainer_log;
} Gym;



//////////////////////////////
//
// Gym funcitons
//


/**
 * @brief Initialize a gym on the heap
 * @return (Gym*) newly allocated struct
 */
Gym* gym_init();


/**
 * @brief Free a gym from the heap
 * @param gym (Gym*) gym to be freed
 */
void gym_del(Gym *gym);




//////////////////////////////
//
// Shared memory and semaphore functions
//


/**
 * @brief Initialize the shared memory space and semaphore. Should only be called by parent process
 * @param maxCouches (int) Max number of clients in waiting room
 * @param numTrainers (int) total number of trainers in simulation
 * @param boundary_case (bool) Flag to solve for Part B
 * @param realistic (bool) Flag to toggle how clients choose weights
 * @param detectDeadlock (bool) Flag to solve for Part C
 * @param fixDeadlock (bool) Flag to solve for Part D
 * @param trainerLog (bool) Flag to solve for Part E
 */
int init_shared_gym(int maxCouches, int numTrainers, bool boundary_case, bool realistic, bool detectDeadlock, bool fixDeadlock, bool trainerLog);


/**
 * @brief Open the shared memory and semaphore in the current process.
 */
void open_shared_gym();


/**
 * @brief Close the shared memory and semaphore in the current process
 */
void close_shared_gym();


/**
 * @brief Free the shared memory and semaphore. Should follow a call to init_share_gym() in the parent process.
 */
void destroy_shared_gym();



//////////////////////////////
//
// Update shared and local memory
//


/**
 * @brief Copy a locally stored, LL-based gym, to the shared memory space. Will only modify structs with the current process ID
 * @param gym (Gym*) gym to be copied
 */
void update_gym(Gym *gym);


/**
 * @brief Copy the shared space, array-based gym, to the local memory space. Will not modify structs with the current process ID
 */
void update_shared_gym(Gym* gym);



//////////////////////////////
//
// Helper functions
//


/**
 * @brief Copy a client struct from src to dest
 * @param dest (Client*) Destination of copy
 * @param src (Client*) Source of copy
 * @return (Client*) same as dest
 */
Client* copy_client(Client *dest, Client *src);


/**
 * @brief Copy a trainer from src to dest
 * @param src (Trainer*) destination of copy
 * @param dest (Trainer*) src of copy
 * @return (Trainer*) same as dest
 */
Trainer* copy_trainer(Trainer* dest, Trainer *src);


/**
 * @brief Delay in milliseconds
 * @param mS (long) delay time
 */
void delay(long mS);

#endif // GYM_H