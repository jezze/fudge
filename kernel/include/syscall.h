#define SYSCALL_ROUTINE_SLOTS           32

#define SYSCALL_INDEX_OPEN              0x01
#define SYSCALL_INDEX_CLOSE             0x02
#define SYSCALL_INDEX_READ              0x03
#define SYSCALL_INDEX_WRITE             0x04
#define SYSCALL_INDEX_MOUNT             0x05
#define SYSCALL_INDEX_EXECUTE           0x06
#define SYSCALL_INDEX_SPAWN             0x07
#define SYSCALL_INDEX_EXIT              0x08
#define SYSCALL_INDEX_LOAD              0x0A
#define SYSCALL_INDEX_UNLOAD            0x0B

struct syscall_close_args
{

    void *caller;
    unsigned int index;

};

struct syscall_execute_args
{

    void *caller;
    unsigned int index;

};

struct syscall_exit_args
{

    void *caller;

};

struct syscall_load_args
{

    void *caller;
    unsigned int index;

};

struct syscall_mount_args
{

    void *caller;
    unsigned int index;
    unsigned int offset;
    unsigned int count;
    char *path;

};

struct syscall_open_args
{

    void *caller;
    unsigned int index;
    unsigned int count;
    char *path;

};

struct syscall_read_args
{

    void *caller;
    unsigned int index;
    unsigned int offset;
    unsigned int count;
    void *buffer;

};

struct syscall_unload_args
{

    void *caller;
    unsigned int index;

};

struct syscall_write_args
{

    void *caller;
    unsigned int index;
    unsigned int offset;
    unsigned int count;
    void *buffer;

};

void syscall_set_routine(unsigned int index, unsigned int (*routine)(struct runtime_task *task, void *stack));
void syscall_unset_routine(unsigned int index);
unsigned int syscall_raise(unsigned int index, struct runtime_task *task, void *stack);
void syscall_setup();
