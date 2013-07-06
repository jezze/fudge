enum syscall_index
{

    SYSCALL_INDEX_OPEN                  = 0x01,
    SYSCALL_INDEX_CLOSE                 = 0x02,
    SYSCALL_INDEX_READ                  = 0x03,
    SYSCALL_INDEX_WRITE                 = 0x04,
    SYSCALL_INDEX_MOUNT                 = 0x05,
    SYSCALL_INDEX_BIND                  = 0x06,
    SYSCALL_INDEX_EXECUTE               = 0x07,
    SYSCALL_INDEX_EXIT                  = 0x08,
    SYSCALL_INDEX_LOAD                  = 0x09,
    SYSCALL_INDEX_UNLOAD                = 0x0A,
    SYSCALL_INDEX_SPAWN                 = 0x0B

};

void syscall_set_routine(enum syscall_index index, unsigned int (*routine)(struct runtime_container *container, struct runtime_task *task, void *stack));
void syscall_unset_routine(enum syscall_index index);
unsigned int syscall_raise(unsigned int index, struct runtime_container *container, struct runtime_task *task);
void syscall_setup();
