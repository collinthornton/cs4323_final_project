
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "recordbook.h"


#define SHARED_KEY 0x2345

#define TEST_RECORDBOOK


/*Semaphore for the ensuring mutual exclusion*/
typedef struct SharedMutex {
    pthread_mutex_t mutex;
} SharedMutex;

//defines the name for the record book text file
#define DEFAULT_RECORD_FILE "./data/recordbook.txt"

/*Pointer to store name of the file.*/
char *RecordFileName = NULL;


//creates the shared mutex
SharedMutex *shared_mutex;


static bool TEST = false;

//This function takes the struct 'emp' and adds the name, id, and weight to the record book text file
void addToRecordBook(struct emp *empValue)
{
    FILE *fp = NULL;

    //starts in locked mode
    pthread_mutex_lock(&shared_mutex->mutex);

    if(TEST) {
        printf("%d ENTERED LOCKED AREA WAITING 2 SECONDS\r\n", getpid());
        sleep(2);
    }

    //opens the file in append mode
    fp = fopen(RecordFileName, "a");
    if (fp == NULL)
    {
        perror("Failed to open the file. \n");
    }

    char buffer[1024];

    //print all values to the record book text file in the correct format
    sprintf(buffer, "Client Name: %s\t", empValue->name);
    sprintf(buffer + strlen(buffer), "ID: %d\t", empValue->id);
    sprintf(buffer + strlen(buffer), "Total Weight: %d\r\n", empValue->weight);

    fputs(buffer, fp);

    fclose(fp);

    //unlocks the mutex lock

    if(TEST) {
        printf("%d LEFT LOCKED AREA\r\n", getpid());
    }

    pthread_mutex_unlock(&shared_mutex->mutex);
}

//function for printing out the information from the record book text file
void displayRecordBook()
{
    FILE *fp = NULL;
    struct emp empVal;

    //starts in locked mode
    pthread_mutex_lock(&shared_mutex->mutex);
    
    //opens the file in read mode
    fp = fopen(RecordFileName, "r");
    if (fp == NULL)
    {
        perror("Failed to open the file. \n");
    }

    //prints the information line by line to the console
    while (fread(&empVal, sizeof(empVal), 1, fp) ==  1)
    {
        printf("Name = %s \t Id = %d \t Weight = %d \n", empVal.name, empVal.id, empVal.weight);
        memset(&empVal, 0x00, sizeof(empVal));
    }
    fclose(fp);

    //unlocks the mutex lock
    pthread_mutex_unlock(&shared_mutex->mutex);
}

// Opens the file and truncates it, implying all the records are cleared
void clearRecordBook()
{
    FILE *fp = NULL;

    //opens in locked mode
    pthread_mutex_lock(&shared_mutex->mutex);

    fp = fopen(RecordFileName, "w");
    if (fp == NULL)
    {
        perror("Failed to open the file. \n");
    }
    fclose(fp);

    //unlocks the mutex lock
    pthread_mutex_unlock(&shared_mutex->mutex);
}




//function for initializing the record book. The function MUST be called before invoking any of the record logging operation.
void initRecordBook()
{
    //creating a shared mutex attribute
    pthread_mutexattr_t psharedm;

    //initializing the shared mutex attribute
    pthread_mutexattr_init(&psharedm);
    pthread_mutexattr_setpshared(&psharedm, PTHREAD_PROCESS_SHARED);

    //creating the shared memory space for the shared mutex
    int sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedMutex), IPC_CREAT|0644);

    //if no shared memory space then creates it, after creation and theres still non exit
    if (sharedMemoryID == -1){
        // FAILSAFE FOR MISHANDLED SHARED MEMORY DEALLOCATION
        system("ipcrm -M 9029");
        sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedMutex), IPC_CREAT|0644);

        if(sharedMemoryID == -1) {
            perror("Something went wrong allocating the shared memory space");
            exit(-1);
        }
    }


    //attaching the shared mutex to the shared memory
    shared_mutex = shmat(sharedMemoryID, NULL, 0);

     if (shared_mutex == (void *) -1){
        perror("Could not attached to the shared memory\n");
        return;
    }

    //initializing the shared mutex
    pthread_mutex_init(&shared_mutex->mutex, &psharedm);

    if (shmdt(shared_mutex) == -1 && errno != EINVAL){
        perror("Something happened trying to detach from shared memory\n");
        return;
    }

}

//function to opens the record book. needs to be opened for each trainer to add to it
void openRecordBook() {
    //First get shared object from memory
    int sharedMemoryID;
    int *sharedMemoryAddress;

    //allocates the memory space for the shared mutex
    sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedMutex), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        perror("Something went wrong allocating the shared memory space\n");
        return;
    }

    //attatching the shared mutex to the shared memory
    shared_mutex = shmat(sharedMemoryID, NULL, 0);

    if (shared_mutex == (void *) -1){
        perror("Could not attached to the shared memory\n");
        return;
    }

    RecordFileName = DEFAULT_RECORD_FILE;

    return;
}

//function to close the record book and detach the shared memory
void closeRecordBook() {
    if (shmdt(shared_mutex) == -1 && errno != EINVAL) {
        perror("Something happened trying to detach from shared memory\n");
        return;
    }
}

//function destroys the record book
void destroyRecordBook() {
    int sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedMutex), IPC_CREAT|0644);

    if (shmctl(sharedMemoryID,IPC_RMID,0) == -1){
        // It's already been closed by another process. Just ignore.
        perror("Something went wrong with the shmctl function\n");
        return;
    }
}



void test_recordbook() {

    TEST = true;

    initRecordBook();
    closeRecordBook();

    for(int i=0; i<2; ++i) {
        pid_t pid = fork();

        if(pid < 0) {
            perror("fork");
            exit(1);
        }
        else if(pid == 0) {
            // CHILD PROCESS
            openRecordBook();

            srand(getpid());
            Emp entry;
            
            sprintf(entry.name, "Process %d", getpid());
            entry.id = getpid();
            entry.weight = rand() % 500;

            addToRecordBook(&entry);

            closeRecordBook();

            exit(0);
        }
        else {
            // PARENT PROCESS -> DON'T DO MUCH
            printf("Spawned process %d\r\n", pid);
        }
    }

    for(int i=0; i<2; ++i) wait(NULL);

    TEST = false;

}
