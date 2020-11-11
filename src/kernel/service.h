struct service_state
{

    struct list_item item;
    unsigned int id;

};

struct service_backend
{

    struct resource resource;
    unsigned int id;
    unsigned int (*read)(void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*map)(unsigned int offset, unsigned int count);

};

struct service_protocol
{

    struct resource resource;
    unsigned int id;
    unsigned int (*match)(struct service_backend *backend);
    unsigned int (*root)(struct service_backend *backend);
    unsigned int (*parent)(struct service_backend *backend, unsigned int id);
    unsigned int (*child)(struct service_backend *backend, unsigned int id, char *path, unsigned int length);
    unsigned int (*create)(struct service_backend *backend, unsigned int id, char *name, unsigned int length);
    unsigned int (*destroy)(struct service_backend *backend, unsigned int id, char *name, unsigned int length);
    unsigned int (*step)(struct service_backend *backend, unsigned int id, unsigned int current);
    unsigned int (*open)(struct service_backend *backend, struct service_state *state, unsigned int id);
    unsigned int (*close)(struct service_backend *backend, struct service_state *state, unsigned int id);
    unsigned int (*read)(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*seek)(struct service_backend *backend, unsigned int id, unsigned int offset);
    unsigned int (*map)(struct service_backend *backend, unsigned int id);

};

struct service_descriptor
{

    struct service_backend *backend;
    struct service_protocol *protocol;
    struct service_state state;
    unsigned int id;
    unsigned int offset;
    unsigned int current;
    unsigned int count;

};

struct service_mountpoint
{

    struct service_backend *backend;
    struct service_protocol *protocol;
    unsigned int id;

};

struct service_mount
{

    struct list_item item;
    struct service_mountpoint parent;
    struct service_mountpoint child;

};

struct service_backend *service_findbackend(unsigned int id);
struct service_protocol *service_findprotocol(unsigned int id);
void service_initstate(struct service_state *state, unsigned int id);
void service_initbackend(struct service_backend *backend, unsigned int id, unsigned int (*read)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*map)(unsigned int offset, unsigned int count));
void service_initprotocol(struct service_protocol *protocol, unsigned int id, unsigned int (*match)(struct service_backend *backend), unsigned int (*root)(struct service_backend *backend), unsigned int (*parent)(struct service_backend *backend, unsigned int id), unsigned int (*child)(struct service_backend *backend, unsigned int id, char *path, unsigned int length), unsigned int (*create)(struct service_backend *backend, unsigned int id, char *name, unsigned int length), unsigned int (*destroy)(struct service_backend *backend, unsigned int id, char *name, unsigned int length), unsigned int (*step)(struct service_backend *backend, unsigned int id, unsigned int current), unsigned int (*open)(struct service_backend *backend, struct service_state *state, unsigned int id), unsigned int (*close)(struct service_backend *backend, struct service_state *state, unsigned int id), unsigned int (*read)(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*seek)(struct service_backend *backend, unsigned int id, unsigned int offset), unsigned int (*map)(struct service_backend *backend, unsigned int id));
void service_initdescriptor(struct service_descriptor *descriptor, unsigned int id);
void service_initmount(struct service_mount *mount);
