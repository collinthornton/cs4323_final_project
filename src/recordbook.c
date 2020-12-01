
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

/*Semaphore for the ensuring mutual exclusion*/
pthread_mutex_t lock;

/* Default file name */
#define DEFAULT_RECORD_FILE "./data/recordbook.txt"

/*Pointer to store name of the file.*/
char *RecordFileName = NULL;



SharedMutex *shared_mutex;


/*Function to add record to the file.
 * File is opened in the append mode, so, that records are added to the end of the file */
void addToRecordBook(struct emp *empValue)
{
    FILE *fp = NULL;

    pthread_mutex_lock(&lock);
    printf("%d ENTERED LOCKED AREA\r\n", getpid());

    /*
     * Open the file in the append mode.
     * */
    fp = fopen(RecordFileName, "a");
    if (fp == NULL)
    {
        printf("Failed to open the file. \n");
    }

    char buffer[1024];

    sprintf(buffer, "Client Name: %s\t", empValue->name);
    sprintf(buffer + strlen(buffer), "ID: %d\t", empValue->id);
    sprintf(buffer + strlen(buffer), "Total Weight: %d\r\n", empValue->weight);

    fputs(buffer, fp);

    //fwrite( empValue, sizeof( *empValue ), 1, fp );
    fclose(fp);

    printf("%d LEFT LOCKED AREA\r\n", getpid());
    pthread_mutex_unlock(&lock);
}

/* Display the records stored in the file.
 * */
void displayRecordBook()
{
    FILE *fp = NULL;
    struct emp empVal;
    pthread_mutex_lock(&lock);
    /*
     * Open the file in the append mode.
     * */
    fp = fopen(RecordFileName, "r");
    if (fp == NULL)
    {
        printf("Failed to open the file. \n");
    }

    while (fread(&empVal, sizeof(empVal), 1, fp) ==  1)
    {
        printf("Name = %s \t Id = %d \t Weight = %d \n", empVal.name, empVal.id, empVal.weight);
        memset(&empVal, 0x00, sizeof(empVal));
    }
    fclose(fp);
    pthread_mutex_unlock(&lock);
}

/* Opens the file and truncates it, implying all the records are cleared.
 * */
void clearRecordBook()
{
    FILE *fp = NULL;
    pthread_mutex_lock(&lock);
    fp = fopen(RecordFileName, "w");
    if (fp == NULL)
    {
        printf("Failed to open the file. \n");
    }
    fclose(fp);
    pthread_mutex_unlock(&lock);
}




/* The function MUST be called before invoking any of the record logging operation.
 * */
void initRecordBook()
{
    pthread_mutexattr_t psharedm;

    pthread_mutexattr_init(&psharedm);
    pthread_mutexattr_setpshared(&psharedm, PTHREAD_PROCESS_SHARED);
    

    int sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedMutex), IPC_CREAT|0644);


    if (sharedMemoryID == -1){
        // FAILSAFE FOR MISHANDLED SHARED MEMORY DEALLOCATION

        system("ipcrm -M 9029");
        sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedMutex), IPC_CREAT|0644);

        if(sharedMemoryID == -1) {
            perror("Something went wrong allocating the shared memory space");
            exit(-1);
        }
    }



    shared_mutex = shmat(sharedMemoryID, NULL, 0);

     if (shared_mutex == (void *) -1){
        printf("Could not attached to the shared memory\n");
        return;
    }   

    pthread_mutex_init(&shared_mutex->mutex, &psharedm);

    if (shmdt(shared_mutex) == -1){
        printf("Something happened trying to detach from shared memory\n");
        return;
    }

}


void openRecordBook() {
    //First get shared object from memory
    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedMutex), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("Something went wrong allocating the shared memory space\n");
        return;
    }

    shared_mutex = shmat(sharedMemoryID, NULL, 0);

    if (shared_mutex == (void *) -1){
        printf("Could not attached to the shared memory\n");
        return;
    }    

    RecordFileName = DEFAULT_RECORD_FILE;

    return;    
}


void closeRecordBook() {
    if (shmdt(shared_mutex) == -1){
        printf("Something happened trying to detach from shared memory\n");
        return;
    }        
}

void destroyRecordBook() {
    int sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedMutex), IPC_CREAT|0644);
    
    if (shmctl(sharedMemoryID,IPC_RMID,0) == -1){
        // It's already been closed by another process. Just ignore.
        printf("Something went wrong with the shmctl function\n");
        return;
    }        
}