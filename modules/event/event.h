#define EVENT_TYPE_KEYBOARD             1
#define EVENT_TYPE_MOUSE                2
#define EVENT_TYPE_CONSOLE              3
#define EVENT_TYPE_TIMER                4
#define EVENT_TYPE_NETWORK              5

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

};

void event_notify(unsigned int type, unsigned int count, void *buffer);
