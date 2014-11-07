enum resource_type
{

    RESOURCE_TYPE_CONTAINER             = 1,
    RESOURCE_TYPE_TASK                  = 2,
    RESOURCE_TYPE_BACKEND               = 3,
    RESOURCE_TYPE_CHANNEL               = 4,
    RESOURCE_TYPE_DESCRIPTOR            = 5,
    RESOURCE_TYPE_MOUNT                 = 6,
    RESOURCE_TYPE_BUS                   = 7,
    RESOURCE_TYPE_DRIVER                = 8,
    RESOURCE_TYPE_INTERFACE             = 9,
    RESOURCE_TYPE_PROTOFS               = 10,
    RESOURCE_TYPE_PROTOBINARY           = 11,
    RESOURCE_TYPE_PROTONET              = 12,
    RESOURCE_TYPE_CHANNELNET            = 13

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
