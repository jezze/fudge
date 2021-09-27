#define RESOURCE_TASK                   1
#define RESOURCE_CORE                   2
#define RESOURCE_MAILBOX                3
#define RESOURCE_SERVICE                4
#define RESOURCE_BINARYFORMAT           5
#define RESOURCE_BUS                    6
#define RESOURCE_DRIVER                 7
#define RESOURCE_DEBUGLOG               8
#define RESOURCE_ETHERNETINTERFACE      40
#define RESOURCE_AUDIOINTERFACE         41
#define RESOURCE_VIDEOINTERFACE         42
#define RESOURCE_TIMERINTERFACE         43
#define RESOURCE_CLOCKINTERFACE         44
#define RESOURCE_KEYBOARDINTERFACE      45
#define RESOURCE_MOUSEINTERFACE         46
#define RESOURCE_BLOCKINTERFACE         47
#define RESOURCE_CONSOLEINTERFACE       48

struct resource
{

    struct list_item item;
    unsigned int type;
    void *data;

};

struct resource *resource_foreach(struct resource *resource);
struct resource *resource_foreachtype(struct resource *resource, unsigned int type);
void resource_register(struct resource *resource);
void resource_unregister(struct resource *resource);
void resource_init(struct resource *resource, unsigned int type, void *data);
void resource_setup(void);
