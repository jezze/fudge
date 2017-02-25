struct service_backend
{

    struct resource resource;
    unsigned int id;
    unsigned int (*read)(void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*map)(unsigned int offset, unsigned int count);

};

struct service_descriptor
{

    struct list_item link;
    struct task *task;
    struct container_server *server;
    unsigned int id;
    unsigned int offset;
    unsigned int current;

};

struct service_protocol
{

    struct resource resource;
    unsigned int (*match)(struct service_backend *backend);
    unsigned int (*root)(struct service_backend *backend);
    unsigned int (*parent)(struct service_descriptor *descriptor);
    unsigned int (*child)(struct service_descriptor *descriptor, char *path, unsigned int length);
    unsigned int (*create)(struct service_descriptor *descriptor, char *name, unsigned int length);
    unsigned int (*destroy)(struct service_descriptor *descriptor, char *name, unsigned int length);
    unsigned int (*step)(struct service_descriptor *descriptor);
    unsigned int (*open)(struct service_descriptor *descriptor);
    unsigned int (*close)(struct service_descriptor *descriptor);
    unsigned int (*read)(struct service_descriptor *descriptor, void *buffer, unsigned int count);
    unsigned int (*write)(struct service_descriptor *descriptor, void *buffer, unsigned int count);
    unsigned int (*seek)(struct service_descriptor *descriptor, unsigned int offset);
    unsigned int (*map)(struct service_descriptor *descriptor);

};

struct service_descriptor *service_getdescriptor(struct task *task, unsigned int descriptor);
struct service_backend *service_findbackend(unsigned int id);
struct service_protocol *service_findprotocol(struct service_backend *backend);
void service_initbackend(struct service_backend *backend, unsigned int id, unsigned int (*read)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*map)(unsigned int offset, unsigned int count));
void service_initdescriptor(struct service_descriptor *descriptor, struct task *task);
void service_initprotocol(struct service_protocol *protocol, unsigned int (*match)(struct service_backend *backend), unsigned int (*root)(struct service_backend *backend), unsigned int (*parent)(struct service_descriptor *descriptor), unsigned int (*child)(struct service_descriptor *descriptor, char *path, unsigned int length), unsigned int (*create)(struct service_descriptor *descriptor, char *name, unsigned int length), unsigned int (*destroy)(struct service_descriptor *descriptor, char *name, unsigned int length), unsigned int (*step)(struct service_descriptor *descriptor), unsigned int (*open)(struct service_descriptor *descriptor), unsigned int (*close)(struct service_descriptor *descriptor), unsigned int (*read)(struct service_descriptor *descriptor, void *buffer, unsigned int count), unsigned int (*write)(struct service_descriptor *descriptor, void *buffer, unsigned int count), unsigned int (*seek)(struct service_descriptor *descriptor, unsigned int offset), unsigned int (*map)(struct service_descriptor *descriptor));
void service_setupcpio(void);
