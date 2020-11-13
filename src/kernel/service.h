struct service_state
{

    struct list_item item;
    unsigned int id;

};

struct service_protocol
{

    struct resource resource;
    unsigned int id;
    unsigned int (*root)(void);
    unsigned int (*parent)(unsigned int id);
    unsigned int (*child)(unsigned int id, char *path, unsigned int length);
    unsigned int (*create)(unsigned int id, char *name, unsigned int length);
    unsigned int (*destroy)(unsigned int id, char *name, unsigned int length);
    unsigned int (*step)(unsigned int id, unsigned int current);
    unsigned int (*open)(struct service_state *state, unsigned int id);
    unsigned int (*close)(struct service_state *state, unsigned int id);
    unsigned int (*read)(struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*seek)(unsigned int id, unsigned int offset);
    unsigned int (*map)(unsigned int id);

};

struct service_descriptor
{

    struct service_protocol *protocol;
    struct service_state state;
    unsigned int id;
    unsigned int offset;
    unsigned int current;
    unsigned int count;

};

struct service_mountpoint
{

    struct service_protocol *protocol;
    unsigned int id;

};

struct service_mount
{

    struct list_item item;
    struct service_mountpoint parent;
    struct service_mountpoint child;

};

struct service_protocol *service_findprotocol(unsigned int id);
void service_initstate(struct service_state *state, unsigned int id);
void service_initprotocol(struct service_protocol *protocol, unsigned int id, unsigned int (*root)(void), unsigned int (*parent)(unsigned int id), unsigned int (*child)(unsigned int id, char *path, unsigned int length), unsigned int (*create)(unsigned int id, char *name, unsigned int length), unsigned int (*destroy)(unsigned int id, char *name, unsigned int length), unsigned int (*step)(unsigned int id, unsigned int current), unsigned int (*open)(struct service_state *state, unsigned int id), unsigned int (*close)(struct service_state *state, unsigned int id), unsigned int (*read)(struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*seek)(unsigned int id, unsigned int offset), unsigned int (*map)(unsigned int id));
void service_initdescriptor(struct service_descriptor *descriptor, unsigned int id);
void service_initmountpoint(struct service_mountpoint *mountpoint);
void service_initmount(struct service_mount *mount);
