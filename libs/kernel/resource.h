enum
{

    RESOURCE_CONTAINER                  = 1,
    RESOURCE_TASK                       = 2,
    RESOURCE_VFSBACKEND                 = 3,
    RESOURCE_VFSPROTOCOL                = 4,
    RESOURCE_VFSCHANNEL                 = 5,
    RESOURCE_VFSMOUNT                   = 6,
    RESOURCE_VFSDESCRIPTOR              = 7,
    RESOURCE_BINARYPROTOCOL             = 8,
    RESOURCE_BUS                        = 9,
    RESOURCE_DRIVER                     = 10,
    RESOURCE_ETHERNETINTERFACE          = 11,
    RESOURCE_ETHERNETPROTOCOL           = 12,
    RESOURCE_IPV4PROTOCOL               = 13

};

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
void resource_setup();
