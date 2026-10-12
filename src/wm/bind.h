#define BIND_ACTION_CLOSE               1
#define BIND_ACTION_NEXT                2
#define BIND_ACTION_PREVIOUS            3
#define BIND_ACTION_RUN                 4
#define BIND_ACTION_SCROLLDOWN          5
#define BIND_ACTION_SCROLLUP            6
#define BIND_ACTION_SWAP                7
#define BIND_COMMANDSIZE                128

struct bind_binding
{

    unsigned int key;
    unsigned int mod;
    unsigned int action;
    char command[BIND_COMMANDSIZE];

};

void bind_add(char *line, unsigned int length);
struct bind_binding *bind_find(unsigned int key, unsigned int mod);
