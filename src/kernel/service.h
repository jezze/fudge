struct service_link
{

    struct list_item item;
    unsigned int id;

};

struct service_protocol
{

    char *name;
    struct resource resource;
    unsigned int (*root)(void);
    unsigned int (*parent)(unsigned int id);
    unsigned int (*child)(unsigned int id, char *path, unsigned int length);
    unsigned int (*create)(unsigned int id, char *name, unsigned int length);
    unsigned int (*destroy)(unsigned int id, char *name, unsigned int length);
    unsigned int (*step)(unsigned int id, unsigned int current);
    unsigned int (*open)(unsigned int id);
    unsigned int (*close)(unsigned int id);
    unsigned int (*read)(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*seek)(unsigned int id, unsigned int offset);
    unsigned int (*map)(unsigned int id);
    unsigned int (*link)(unsigned int id, struct service_link *link);
    unsigned int (*unlink)(unsigned int id, struct service_link *link);
    unsigned int (*notify)(unsigned int id, struct service_link *link, struct message_header *header, void *data);

};

struct service_descriptor
{

    struct service_protocol *protocol;
    struct service_link link;
    unsigned int id;
    unsigned int offset;
    unsigned int current;
    unsigned int count;

};

struct service_protocol *service_findprotocol(unsigned int count, char *name);
void service_initlink(struct service_link *link, unsigned int id);
void service_initprotocol(struct service_protocol *protocol, char *name, unsigned int (*root)(void), unsigned int (*parent)(unsigned int id), unsigned int (*child)(unsigned int id, char *path, unsigned int length), unsigned int (*create)(unsigned int id, char *name, unsigned int length), unsigned int (*destroy)(unsigned int id, char *name, unsigned int length), unsigned int (*step)(unsigned int id, unsigned int current), unsigned int (*open)(unsigned int id), unsigned int (*close)(unsigned int id), unsigned int (*read)(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*seek)(unsigned int id, unsigned int offset), unsigned int (*map)(unsigned int id), unsigned int (*link)(unsigned int id, struct service_link *link), unsigned int (*unlink)(unsigned int id, struct service_link *link), unsigned int (*notify)(unsigned int id, struct service_link *link, struct message_header *header, void *data));
void service_initdescriptor(struct service_descriptor *descriptor, unsigned int id);
