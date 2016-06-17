#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "service.h"
#include "container.h"
#include "kernel.h"

static void copysession(struct task *target, struct container_session *tsession, struct container_session *ssession)
{

    list_inititem(&tsession->state.link, target);

    tsession->backend = (ssession) ? ssession->backend : 0;
    tsession->protocol = (ssession) ? ssession->protocol : 0;
    tsession->state.id = (ssession) ? ssession->state.id : 0;

}

void kernel_copysessions(struct container *container, struct task *source, struct task *target)
{

    unsigned int sid = source->id * TASK_DESCRIPTORS;
    unsigned int tid = target->id * TASK_DESCRIPTORS;
    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        copysession(target, &container->sessions[tid + i + 0x00], &container->sessions[sid + i + 0x08]);
        copysession(target, &container->sessions[tid + i + 0x08], &container->sessions[sid + i + 0x08]);
        copysession(target, &container->sessions[tid + i + 0x10], 0);
        copysession(target, &container->sessions[tid + i + 0x18], 0);

    }

}

unsigned int kernel_setupbinary(struct container *container, struct task *task, unsigned int sp)
{

    struct container_session *session = &container->sessions[task->id * TASK_DESCRIPTORS];

    if (!session->state.id)
        return 0;

    if (!session->protocol->map(session->backend, &session->state, &session->node))
        return 0;

    task->format = binary_findformat(&session->node);

    if (!task->format)
        return 0;

    task_resume(task, task->format->findentry(&session->node), sp);

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
    root->backend = mount->parent.backend;
    root->protocol = mount->parent.protocol;
    root->state.id = mount->parent.id;
    init->backend = mount->parent.backend;
    init->protocol = mount->parent.protocol;
    init->state.id = mount->parent.id;
    init->state.id = init->protocol->child(init->backend, &init->state, 4, "bin/");
    init->state.id = init->protocol->child(init->backend, &init->state, 4, "init");

    return init->state.id;

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

