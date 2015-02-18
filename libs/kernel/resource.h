enum
{

    RESOURCE_CONTAINER                  = 1,
    RESOURCE_TASK                       = 2,
    RESOURCE_BACKEND                    = 3,
    RESOURCE_CHANNEL                    = 4,
    RESOURCE_DESCRIPTOR                 = 5,
    RESOURCE_MOUNT                      = 6,
    RESOURCE_BUS                        = 7,
    RESOURCE_DRIVER                     = 8,
    RESOURCE_INTERFACE                  = 9,
    RESOURCE_PROTOFS                    = 10,
    RESOURCE_PROTOBINARY                = 11,
    RESOURCE_PROTONET                   = 12,
    RESOURCE_CHANNELNET                 = 13

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
