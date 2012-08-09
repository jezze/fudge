#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#define SYSCALL_TABLE_SLOTS   16
#define SYSCALL_INDEX_NULL    0x00
#define SYSCALL_INDEX_OPEN    0x01
#define SYSCALL_INDEX_CLOSE   0x02
#define SYSCALL_INDEX_READ    0x03
#define SYSCALL_INDEX_WRITE   0x04
#define SYSCALL_INDEX_EXECUTE 0x05
#define SYSCALL_INDEX_EXIT    0x06
#define SYSCALL_INDEX_IDLE    0x07
#define SYSCALL_INDEX_LOAD    0x08
#define SYSCALL_INDEX_UNLOAD  0x09
#define SYSCALL_INDEX_ATTACH  0x0A
#define SYSCALL_INDEX_DETACH  0x0B
#define SYSCALL_INDEX_MOUNT   0x0C

struct syscall_attach_args
{

    void *caller;
    unsigned int index;
    unsigned int callback;

};

struct syscall_close_args
{

    void *caller;
    unsigned int index;

};

struct syscall_detach_args
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

struct syscall_idle_args
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
    void *buffer;

};

struct syscall_open_args
{

    void *caller;
    unsigned int index;
    unsigned int count;
    void *buffer;

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

unsigned int syscall_attach(struct runtime_task *task, void *stack);
unsigned int syscall_close(struct runtime_task *task, void *stack);
unsigned int syscall_detach(struct runtime_task *task, void *stack);
unsigned int syscall_execute(struct runtime_task *task, void *stack);
unsigned int syscall_exit(struct runtime_task *task, void *stack);
unsigned int syscall_load(struct runtime_task *task, void *stack);
unsigned int syscall_mount(struct runtime_task *task, void *stack);
unsigned int syscall_open(struct runtime_task *task, void *stack);
unsigned int syscall_read(struct runtime_task *task, void *stack);
unsigned int syscall_unload(struct runtime_task *task, void *stack);
unsigned int syscall_idle(struct runtime_task *task, void *stack);
unsigned int syscall_write(struct runtime_task *task, void *stack);

#endif

