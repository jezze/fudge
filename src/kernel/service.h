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
    unsigned int (*match)(struct service_backend *backend);
    unsigned int (*root)(struct service_backend *backend);
    unsigned int (*parent)(struct task_descriptor *descriptor);
    unsigned int (*child)(struct task_descriptor *descriptor, char *path, unsigned int length);
    unsigned int (*create)(struct task_descriptor *descriptor, char *name, unsigned int length);
    unsigned int (*destroy)(struct task_descriptor *descriptor, char *name, unsigned int length);
    unsigned int (*step)(struct task_descriptor *descriptor);
    unsigned int (*open)(struct task_descriptor *descriptor);
    unsigned int (*close)(struct task_descriptor *descriptor);
    unsigned int (*read)(struct task_descriptor *descriptor, void *buffer, unsigned int count);
    unsigned int (*write)(struct task_descriptor *descriptor, void *buffer, unsigned int count);
    unsigned int (*seek)(struct task_descriptor *descriptor, unsigned int offset);
    unsigned int (*map)(struct task_descriptor *descriptor);

};

struct service_backend *service_findbackend(unsigned int id);
struct service_protocol *service_findprotocol(struct service_backend *backend);
void service_initbackend(struct service_backend *backend, unsigned int id, unsigned int (*read)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*map)(unsigned int offset, unsigned int count));
void service_initprotocol(struct service_protocol *protocol, unsigned int (*match)(struct service_backend *backend), unsigned int (*root)(struct service_backend *backend), unsigned int (*parent)(struct task_descriptor *descriptor), unsigned int (*child)(struct task_descriptor *descriptor, char *path, unsigned int length), unsigned int (*create)(struct task_descriptor *descriptor, char *name, unsigned int length), unsigned int (*destroy)(struct task_descriptor *descriptor, char *name, unsigned int length), unsigned int (*step)(struct task_descriptor *descriptor), unsigned int (*open)(struct task_descriptor *descriptor), unsigned int (*close)(struct task_descriptor *descriptor), unsigned int (*read)(struct task_descriptor *descriptor, void *buffer, unsigned int count), unsigned int (*write)(struct task_descriptor *descriptor, void *buffer, unsigned int count), unsigned int (*seek)(struct task_descriptor *descriptor, unsigned int offset), unsigned int (*map)(struct task_descriptor *descriptor));
void service_setupcpio(void);
