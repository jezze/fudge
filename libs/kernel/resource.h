enum resource_type
{

    RESOURCE_TYPE_ALL                   = 1,
    RESOURCE_TYPE_CONTAINER             = 2,
    RESOURCE_TYPE_TASK                  = 3,
    RESOURCE_TYPE_VFSBACKEND            = 4,
    RESOURCE_TYPE_VFSPROTOCOL           = 5,
    RESOURCE_TYPE_BINARYPROTOCOL        = 6

};

struct resource_id
{

    enum resource_type type;
    unsigned int size;
    const char *text;

};

struct resource_item
{

    struct list_item item;

};

struct resource_list
{

    struct list_item item;
    struct list list;
    struct resource_id id;

};

struct resource_list *resource_find_list(enum resource_type type);
void resource_register_item(struct resource_item *item, enum resource_type type);
void resource_register_list(struct resource_list *list);
void resource_unregister_item(struct resource_item *item, enum resource_type type);
void resource_unregister_list(struct resource_list *list);
void resource_init_item(struct resource_item *item, void *data);
void resource_init_list(struct resource_list *list, enum resource_type type, unsigned int size, const char *text);
struct list *resource_setup();
