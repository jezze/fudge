enum request
{

    REQUEST_WALK                        = 0x01,
    REQUEST_CREATE                      = 0x02,
    REQUEST_DESTROY                     = 0x03,
    REQUEST_OPEN                        = 0x04,
    REQUEST_CLOSE                       = 0x05,
    REQUEST_READ                        = 0x06,
    REQUEST_WRITE                       = 0x07,
    REQUEST_AUTH                        = 0x08,
    REQUEST_MOUNT                       = 0x09,
    REQUEST_BIND                        = 0x0A,
    REQUEST_EXECUTE                     = 0x0B,
    REQUEST_LOAD                        = 0x0C,
    REQUEST_UNLOAD                      = 0x0D,
    REQUEST_SPAWN                       = 0x0E,
    REQUEST_DESPAWN                     = 0x0F

};

unsigned int request_call(unsigned int index, struct container *container, struct task *task, void *stack);
void request_setup(unsigned int (*spawn)(struct container *self, struct task *task, void *stack), unsigned int (*despawn)(struct container *self, struct task *task, void *stack));
