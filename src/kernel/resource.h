#define RESOURCE_TASK                   1
#define RESOURCE_CORE                   2
#define RESOURCE_MAILBOX                3
#define RESOURCE_SERVICEPROTOCOL        4
#define RESOURCE_BINARYFORMAT           5
#define RESOURCE_BUS                    6
#define RESOURCE_DRIVER                 7
#define RESOURCE_DEBUGLOG               8
#define RESOURCE_ETHERNETINTERFACE      40
#define RESOURCE_ETHERNETHOOK           41
#define RESOURCE_IPV4HOOK               42
#define RESOURCE_IPV6HOOK               43
#define RESOURCE_AUDIOINTERFACE         44
#define RESOURCE_VIDEOINTERFACE         45
#define RESOURCE_TIMERINTERFACE         46
#define RESOURCE_CLOCKINTERFACE         47
#define RESOURCE_KEYBOARDINTERFACE      48
#define RESOURCE_MOUSEINTERFACE         49
#define RESOURCE_BLOCKINTERFACE         50
#define RESOURCE_CONSOLEINTERFACE       51

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
