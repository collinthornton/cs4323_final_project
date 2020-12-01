
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "recordbook.h"

/*Semaphore for the ensuring mutual exclusion*/
pthread_mutex_t lock;

/* Default file name */
#define DEFAULT_RECORD_FILE "recordbook.txt"

/*Pointer to store name of the file.*/
char *RecordFileName = NULL;


/*Function to add record to the file.
 * File is opened in the append mode, so, that records are added to the end of the file */
void addToRecordBook(struct emp *empValue)
{
    FILE *fp = NULL;

    pthread_mutex_lock(&lock);
    /*
     * Open the file in the append mode.
     * */
    fp = fopen(RecordFileName, "a");
    if (fp == NULL)
    {
        printf("Failed to open the file. \n");
    }
    fwrite( empValue, sizeof( *empValue ), 1, fp );
    fclose(fp);
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
void initRecordBook(char *filename)
{
    pthread_mutex_init(&lock,NULL);
    if (filename)
    {
        RecordFileName = filename;
    }
    else
    {
        RecordFileName = DEFAULT_RECORD_FILE;
    }
}
