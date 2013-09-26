#include <fudge/kernel.h>
#include "error.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

#define KERNEL_BINARY_PROTOCOLS         1
#define KERNEL_VFS_PROTOCOLS            2

static struct
{

    struct container container;
    struct {struct binary_protocol protocols[KERNEL_BINARY_PROTOCOLS];} binary;
    struct {struct vfs_protocol protocols[KERNEL_VFS_PROTOCOLS];} vfs;

} state;

void kernel_lock(struct kernel_lock *lock)
{

    lock->state = LOCK_STATE_LOCKED;

}

void kernel_unlock(struct kernel_lock *lock)
{

    lock->state = LOCK_STATE_UNLOCKED;

}

void kernel_rendezvous_sleep(struct kernel_rendezvous *rendezvous, unsigned int condition)
{

    if (!condition)
        return;

    rendezvous->task = state.container.current;

/*
    rendezvous->task->state |= TASK_STATE_BLOCKED;
*/

}

void kernel_rendezvous_unsleep(struct kernel_rendezvous *rendezvous, unsigned int condition)
{

    if (!condition)
        return;

/*
    rendezvous->task->state &= ~TASK_STATE_BLOCKED;
*/

    rendezvous->task = 0;

}

void kernel_setup_modules(unsigned int count, struct kernel_module *modules)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct vfs_session session;
        struct binary_protocol *protocol;
        unsigned int id;

        session.backend = &modules[i].base;
        session.protocol = vfs_find_protocol(session.backend);

        if (!session.protocol)
            continue;

        id = session.protocol->root(session.backend);

        if (!id)
            continue;

        id = session.protocol->walk(session.backend, id, 8, "bin/init");

        if (!id)
            continue;

        protocol = binary_find_protocol(&session, id);

        if (!protocol)
            continue;

        state.container.mounts[0x01].parent.session.backend = session.backend;
        state.container.mounts[0x01].parent.session.protocol = session.protocol;
        state.container.mounts[0x01].parent.id = session.protocol->root(session.backend);
        state.container.mounts[0x01].child.session.backend = session.backend;
        state.container.mounts[0x01].child.session.protocol = session.protocol;
        state.container.mounts[0x01].child.id = session.protocol->root(session.backend);
        state.container.current->descriptors[0x01].session.backend = session.backend;
        state.container.current->descriptors[0x01].session.protocol = session.protocol;
        state.container.current->descriptors[0x01].id = session.protocol->root(session.backend);
        state.container.current->registers.ip = protocol->copy_program(&session, id);

        error_assert(state.container.current->registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

        return;

    }

    error_panic("Failed to locate init", __FILE__, __LINE__);

}

struct container *kernel_setup()
{

    vfs_setup();
    vfs_init_cpio(&state.vfs.protocols[0]);
    vfs_register_protocol(&state.vfs.protocols[0]);
    vfs_init_tar(&state.vfs.protocols[1]);
    vfs_register_protocol(&state.vfs.protocols[1]);
    binary_setup();
    binary_init_elf(&state.binary.protocols[0]);
    binary_register_protocol(&state.binary.protocols[0]);
    container_init(&state.container);

    return &state.container;

}

