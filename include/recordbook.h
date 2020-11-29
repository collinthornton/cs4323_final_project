
#ifndef RECORDKEEPING_RECORDBOOK_H
#define RECORDKEEPING_RECORDBOOK_H


struct emp
{
    char name[100];
    int id;
    int weight;
};

void addToRecordBook(struct emp *empValue);
void displayRecordBook();
void clearRecordBook();
void initRecordBook(char *filename);

#endif //RECORDKEEPING_RECORDBOOK_H
