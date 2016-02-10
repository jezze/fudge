#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "service.h"
#include "binary.h"
#include "container.h"
#include "kernel.h"

void kernel_copysessions(struct container *container, struct task *source, struct task *target)
{

    unsigned int sid = source->id * TASK_DESCRIPTORS;
    unsigned int tid = target->id * TASK_DESCRIPTORS;
    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        container->sessions[tid + i + 0x00].backend = container->sessions[sid + i + 0x08].backend;
        container->sessions[tid + i + 0x00].protocol = container->sessions[sid + i + 0x08].protocol;
        container->sessions[tid + i + 0x00].state.id = container->sessions[sid + i + 0x08].state.id;
        container->sessions[tid + i + 0x08].backend = container->sessions[sid + i + 0x08].backend;
        container->sessions[tid + i + 0x08].protocol = container->sessions[sid + i + 0x08].protocol;
        container->sessions[tid + i + 0x08].state.id = container->sessions[sid + i + 0x08].state.id;
        container->sessions[tid + i + 0x10].backend = 0;
        container->sessions[tid + i + 0x10].protocol = 0;
        container->sessions[tid + i + 0x10].state.id = 0;
        container->sessions[tid + i + 0x18].backend = 0;
        container->sessions[tid + i + 0x18].protocol = 0;
        container->sessions[tid + i + 0x18].state.id = 0;

    }

}

unsigned int kernel_setupbinary(struct container *container, struct task *task, unsigned int sp)
{

    struct container_session *session = &container->sessions[task->id * TASK_DESCRIPTORS];
    struct binary_format *format;

    if (!session->state.id)
        return 0;

    format = binary_findformat(session->protocol, session->backend, session->state.id);

    if (!format)
        return 0;

    task_resume(task, format->findentry(session->protocol, session->backend, session->state.id), sp);

    return task->state.registers.ip;

}

unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend)
{

    struct container_mount *mount = &container->mounts[0x00];
    struct container_session *init = &container->sessions[0x08];
    struct container_session *root = &container->sessions[0x09];

    mount->parent.backend = backend;
    mount->parent.protocol = service_findprotocol(backend);
    mount->parent.id = mount->parent.protocol->root(backend);
    mount->child.backend = mount->parent.backend;
    mount->child.protocol = mount->parent.protocol;
    mount->child.id = mount->parent.id;

    if (!mount->parent.id)
        return 0;

    root->backend = mount->parent.backend;
    root->protocol = mount->parent.protocol;
    root->state.id = mount->parent.id;
    init->backend = mount->parent.backend;
    init->protocol = mount->parent.protocol;
    init->state.id = init->protocol->child(init->backend, root->state.id, 4, "bin/");

    if (!init->state.id)
        return 0;

    return init->state.id = init->protocol->child(init->backend, init->state.id, 4, "init");

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

