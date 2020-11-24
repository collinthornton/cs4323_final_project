#include "client.h"
#include "trainer.h"

#define SHARED_KEY 0x1234
#define BUFFER_SIZE 1024

void open_gym(int numberTrainers, int numberCouches, int numberClients, int useSemaphors);
int init_shared_gym(int maxCouches);
Gym* get_shared_gym();
void clean_shared_gym(Gym* sharedGym);

struct Gym {
    ClientList* waitingList;
    ClientList* arrivingList;
    TrainerList* trainerList;
    int maxCouches;
} typedef Gym;

