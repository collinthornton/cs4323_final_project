
typedef enum {ARRIVING, WAITING, TRAINING, TRAVELING, LEAVING} ClientState;

typedef struct {
    int pID;
    ClientState state;
} Client;

typedef struct {
    Client* node;
    Client* prev;
    Client* next;
} ClientNode;

typedef struct {
    ClientNode* HEAD, TAIL;
    ClientNode* node;

    int size;
    int allocated;
} ClientList;

// Allocate client on heap. Init params as NULL if unavailable
Client* client_init(ClientState state, int pID);

ClientList* client_init_list();
int client_add_to_list(Client *client, ClientList* list);
int client_rem_from_list(Client *client, ClientList *list);

int client_del_list(ClientList *list);
int client_del_client(Client *client);