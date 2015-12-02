#define RESOURCE_CONTAINER              1
#define RESOURCE_TASK                   2
#define RESOURCE_VFSBACKEND             3
#define RESOURCE_VFSPROTOCOL            4
#define RESOURCE_VFSCHANNEL             5
#define RESOURCE_VFSMOUNT               6
#define RESOURCE_TASKDESCRIPTOR         7
#define RESOURCE_BINARYFORMAT           8
#define RESOURCE_BUS                    9
#define RESOURCE_DRIVER                 10
#define RESOURCE_ETHERNETINTERFACE      11
#define RESOURCE_ETHERNETPROTOCOL       12
#define RESOURCE_IPV4PROTOCOL           13
#define RESOURCE_AUDIOINTERFACE         14
#define RESOURCE_VIDEOINTERFACE         15
#define RESOURCE_TIMERINTERFACE         16
#define RESOURCE_CLOCKINTERFACE         17
#define RESOURCE_KEYBOARDINTERFACE      18
#define RESOURCE_MOUSEINTERFACE         19
#define RESOURCE_BLOCKINTERFACE         20
#define RESOURCE_CONSOLEINTERFACE       21

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
