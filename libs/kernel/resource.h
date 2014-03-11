enum resource_type
{

    RESOURCE_TYPE_ALL                   = 1,
    RESOURCE_TYPE_CONTAINER             = 2,
    RESOURCE_TYPE_TASK                  = 3,
    RESOURCE_TYPE_VFS                   = 5,
    RESOURCE_TYPE_VFSBACKEND            = 6,
    RESOURCE_TYPE_VFSPROTOCOL           = 7,
    RESOURCE_TYPE_BINARY                = 8,
    RESOURCE_TYPE_BINARYPROTOCOL        = 9

};

struct resource_id
{

    enum resource_type type;

};

struct resource_item
{

    struct list_item item;
    struct resource_id id;
    void *data;

};

struct resource_iterator
{

    unsigned int (*match)(struct resource_item *item); 
    unsigned int (*read)(struct resource_item *item, unsigned int offset, unsigned int count, void *buffer);

};

struct resource_item *resource_find_item(struct resource_iterator *iterator, struct resource_item *item);
void resource_register_item(struct resource_item *item);
void resource_init_item(struct resource_item *item, enum resource_type type, void *data);
void resource_init_iterator(struct resource_iterator *iterator, unsigned int (*match)(struct resource_item *item), unsigned int (*read)(struct resource_item *item, unsigned int offset, unsigned int count, void *buffer));
void resource_setup();
