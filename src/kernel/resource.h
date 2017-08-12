#define RESOURCE_TASK                   1
#define RESOURCE_SERVICEBACKEND         3
#define RESOURCE_SERVICEPROTOCOL        4
#define RESOURCE_BINARYFORMAT           6
#define RESOURCE_BUS                    7
#define RESOURCE_DRIVER                 8
#define RESOURCE_ETHERNETINTERFACE      40
#define RESOURCE_ETHERNETPROTOCOL       41
#define RESOURCE_IPV4PROTOCOL           42
#define RESOURCE_IPV6PROTOCOL           43
#define RESOURCE_AUDIOINTERFACE         44
#define RESOURCE_VIDEOINTERFACE         45
#define RESOURCE_TIMERINTERFACE         46
#define RESOURCE_CLOCKINTERFACE         47
#define RESOURCE_KEYBOARDINTERFACE      48
#define RESOURCE_MOUSEINTERFACE         49
#define RESOURCE_BLOCKINTERFACE         50
#define RESOURCE_CONSOLEINTERFACE       51
#define RESOURCE_DEBUGLOG               60
#define RESOURCE_CON                    61
#define RESOURCE_PART                   62

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
