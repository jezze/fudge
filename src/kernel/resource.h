#define RESOURCE_TASK                   0x01
#define RESOURCE_CORE                   0x02
#define RESOURCE_MAILBOX                0x03
#define RESOURCE_NODE                   0x04
#define RESOURCE_BINARYFORMAT           0x05
#define RESOURCE_BUS                    0x06
#define RESOURCE_DRIVER                 0x07
#define RESOURCE_SERVICE                0x08
#define RESOURCE_DEBUGLOG               0x09
#define RESOURCE_ETHERNETINTERFACE      0x10
#define RESOURCE_AUDIOINTERFACE         0x11
#define RESOURCE_VIDEOINTERFACE         0x12
#define RESOURCE_TIMERINTERFACE         0x13
#define RESOURCE_CLOCKINTERFACE         0x14
#define RESOURCE_KEYBOARDINTERFACE      0x15
#define RESOURCE_MOUSEINTERFACE         0x16
#define RESOURCE_BLOCKINTERFACE         0x17
#define RESOURCE_CONSOLEINTERFACE       0x18

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
