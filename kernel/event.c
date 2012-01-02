#include <lib/memory.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/mmu.h>
#include <kernel/runtime.h>

static struct event_routine routines[EVENT_ROUTINE_SLOTS];
static struct event_buffer buffer;

unsigned int event_register_routine(unsigned int index, struct runtime_task *task, void (*callback)())
{

    struct event_routine *routine = &routines[index];

    if (routine->task)
        return 0;

    routine->task = task;
    routine->callback = callback;

    return 1;

}

unsigned int event_unregister_routine(unsigned int index, struct runtime_task *task)
{

    struct event_routine *routine = &routines[index];

    if (routine->task != task)
        return 0;

    routine->task = 0;
    routine->callback = 0;

    return 1;

}

static struct event_routine *event_buffer_get(struct event_buffer *self)
{

    struct event_routine *routine;

    if (self->head != self->tail)
    {

        routine = self->buffer[self->tail];
        self->tail = ((self->tail + 1) % self->size);

        return routine;

    }

    return 0;

}

static void event_buffer_put(struct event_buffer *self, struct event_routine *routine)
{

    if ((self->head + 1) % self->size != self->tail)
    {

        self->buffer[self->head] = routine;
        self->head = ((self->head + 1) % self->size);

    }

}

void event_raise(unsigned int index)
{

    buffer.size = 8;
    buffer.head = 0;
    buffer.tail = 0;

    event_buffer_put(&buffer, &routines[index]);
    struct event_routine *routine = event_buffer_get(&buffer);

    if (!routine->callback)
        return;

    struct runtime_task *ctask = runtime_get_running_task();

    if (!ctask)
        return;

    struct runtime_task *etask = runtime_get_task(routine->task->id);

    if (etask == ctask)
        return;

    etask->load(etask, routine->callback, 0, 0);

    runtime_activate(etask, ctask);

}

