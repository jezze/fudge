struct service_backend
{

    struct resource resource;
    unsigned int id;
    unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(unsigned int offset, unsigned int count, void *buffer);
    unsigned long (*getphysical)(void);

};

struct service_protocol
{

    struct resource resource;
    unsigned int (*match)(struct service_backend *backend);
    unsigned int (*root)(struct service_backend *backend);
    unsigned int (*parent)(struct service_backend *backend, unsigned int id);
    unsigned int (*child)(struct service_backend *backend, unsigned int id, unsigned int count, char *path);
    unsigned int (*create)(struct service_backend *backend, unsigned int id, unsigned int count, char *name);
    unsigned int (*destroy)(struct service_backend *backend, unsigned int id, unsigned int count, char *name);
    unsigned int (*open)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id);
    unsigned int (*close)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id);
    unsigned int (*read)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*seek)(struct service_backend *backend, unsigned int id, unsigned int offset);
    unsigned int (*scan)(struct service_backend *backend, unsigned int id, unsigned int index);
    unsigned long (*getphysical)(struct service_backend *backend, unsigned int id);

};

struct service_channel
{

    struct resource resource;
    struct service_backend *backend;
    struct service_protocol *protocol;

};

struct service_backend *service_findbackend(unsigned int id);
struct service_protocol *service_findprotocol(struct service_backend *backend);
void service_initbackend(struct service_backend *backend, unsigned int id, unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(unsigned int offset, unsigned int count, void *buffer), unsigned long (*getphysical)(void));
void service_initprotocol(struct service_protocol *protocol, unsigned int (*match)(struct service_backend *backend), unsigned int (*root)(struct service_backend *backend), unsigned int (*parent)(struct service_backend *backend, unsigned int id), unsigned int (*child)(struct service_backend *backend, unsigned int id, unsigned int count, char *path), unsigned int (*create)(struct service_backend *backend, unsigned int id, unsigned int count, char *name), unsigned int (*destroy)(struct service_backend *backend, unsigned int id, unsigned int count, char *name), unsigned int (*open)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id), unsigned int (*close)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id), unsigned int (*read)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*seek)(struct service_backend *backend, unsigned int id, unsigned int offset), unsigned int (*scan)(struct service_backend *backend, unsigned int id, unsigned int index), unsigned long (*getphysical)(struct service_backend *backend, unsigned int id));
void service_initchannel(struct service_channel *channel);
void service_setupcpio(void);
