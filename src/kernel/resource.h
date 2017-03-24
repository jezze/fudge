#define RESOURCE_CONTAINER              1
#define RESOURCE_CONTAINERMOUNT         2
#define RESOURCE_TASK                   3
#define RESOURCE_TASKDESCRIPTOR         4
#define RESOURCE_SERVICEBACKEND         5
#define RESOURCE_SERVICEPROTOCOL        6
#define RESOURCE_SERVICECHANNEL         7
#define RESOURCE_BINARYFORMAT           8
#define RESOURCE_BUS                    9
#define RESOURCE_DRIVER                 10
#define RESOURCE_ETHERNETINTERFACE      11
#define RESOURCE_ETHERNETPROTOCOL       12
#define RESOURCE_IPV4PROTOCOL           13
#define RESOURCE_IPV6PROTOCOL           14
#define RESOURCE_AUDIOINTERFACE         15
#define RESOURCE_VIDEOINTERFACE         16
#define RESOURCE_TIMERINTERFACE         17
#define RESOURCE_CLOCKINTERFACE         18
#define RESOURCE_KEYBOARDINTERFACE      19
#define RESOURCE_MOUSEINTERFACE         20
#define RESOURCE_BLOCKINTERFACE         21
#define RESOURCE_CONSOLEINTERFACE       22
#define RESOURCE_DEBUGLOG               23
#define RESOURCE_CON                    24
#define RESOURCE_PART                   25

struct resource
{

    struct list_item item;
    unsigned int type;
    void *data;

};

struct resource *resource_find(struct resource *resource);
struct resource *resource_findtype(struct resource *resource, unsigned int type);
void resource_register(struct resource *resource);
void resource_unregister(struct resource *resource);
void resource_init(struct resource *resource, unsigned int type, void *data);
