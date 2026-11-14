#define RESOURCE_MAILBOX                0x03
#define RESOURCE_BINARYFORMAT           0x04
#define RESOURCE_BUS                    0x05
#define RESOURCE_DRIVER                 0x06
#define RESOURCE_SERVICE                0x07
#define RESOURCE_DEBUGLOG               0x08
#define RESOURCE_ETHERNETINTERFACE      0x10
#define RESOURCE_AUDIOINTERFACE         0x11
#define RESOURCE_VIDEOINTERFACE         0x12
#define RESOURCE_TIMERINTERFACE         0x13
#define RESOURCE_CLOCKINTERFACE         0x14
#define RESOURCE_KEYBOARDINTERFACE      0x15
#define RESOURCE_MOUSEINTERFACE         0x16
#define RESOURCE_BLOCKINTERFACE         0x17
#define RESOURCE_CONSOLEINTERFACE       0x18
#define RESOURCE_TOUCHINTERFACE         0x19

struct resource_operands
{

    unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data);

};

struct resource
{

    struct list_item item;
    unsigned int type;
    void *data;
    unsigned int inode;
    struct list links;
    struct resource_operands *operands;

};

struct resource *resource_foreachtype_unsafe(struct resource *resource, unsigned int type);
struct resource *resource_foreachtype(struct resource *resource, unsigned int type);
void resource_lock(void);
void resource_unlock(void);
void resource_register(struct resource *resource);
void resource_unregister(struct resource *resource);
void resource_operands_init(struct resource_operands *operands, unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data));
void resource_init(struct resource *resource, unsigned int type, void *data, struct resource_operands *operands);
void resource_setup(void);
