/*-------------------------------------------------------------

    Author  -   Robert Cook
    Email   -   robert.cook@okstate.edu
    Date    -   11-24-2020
    Description
        This file represents the "entrance" of the gym
        This will be what handles all of the client arrivals
        and can be thought of as the front doors of the gym

-------------------------------------------------------------*/


#ifndef ENTRANCE_H
#define ENTRANCE_H


#include "client.h"
#include "trainer.h"
#include "gym.h"


void open_gym(int numberTrainers, int numberCouches, int numberClients, int useSemaphors);
void client_arriving_event(Gym* sharedGym, Client* newClient);


#endif // ENTRANCE_H
