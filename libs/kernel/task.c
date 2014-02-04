#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"

static struct list free;
static struct list used;
static struct list blocked;

static struct list_item *get_item_with_state(struct list *list, int state)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->self;

        if (task_test_flag(task, state))
            return current;

    }

    return 0;

}

static struct list_item *get_item_without_state(struct list *list, int state)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->self;

        if (!task_test_flag(task, state))
            return current;

    }

    return 0;

}

static void prepare()
{

    struct list_item *item;

    while ((item = get_item_with_state(&free, TASK_STATE_USED)))
    {

        list_remove(&free, item);
        list_init_item(item, item->self);
        list_add(&used, item);

    }

    while ((item = get_item_without_state(&used, TASK_STATE_USED)))
    {

        list_remove(&used, item);
        list_init_item(item, item->self);
        list_add(&free, item);

    }

    while ((item = get_item_with_state(&used, TASK_STATE_BLOCKED)))
    {

        list_remove(&used, item);
        list_init_item(item, item->self);
        list_add(&blocked, item);

    }

    while ((item = get_item_without_state(&blocked, TASK_STATE_BLOCKED)))
    {

        list_remove(&blocked, item);
        list_init_item(item, item->self);
        list_add(&used, item);

    }

}

struct task *task_sched_find_next_task()
{

    struct list_item *current;

    prepare();

    if (!used.head)
        return 0;

    for (current = used.head; current->next; current = current->next);

    return current->self;

}

struct task *task_sched_find_free_task()
{

    prepare();

    if (!free.head)
        return 0;

    return free.head->self;

}

void task_sched_add(struct task *task)
{

    list_add(&free, &task->item);

}

void task_sched_init()
{

    list_init(&free);
    list_init(&used);
    list_init(&blocked);

}

void task_set_flag(struct task *task, enum task_state state)
{

    task->state |= state;

}

void task_unset_flag(struct task *task, enum task_state state)
{

    task->state &= ~state;

}

unsigned int task_test_flag(struct task *task, enum task_state state)
{

    return task->state & state;

}

void task_init(struct task *task, enum task_state state, unsigned long ip, unsigned long sp)
{

    memory_clear(task, sizeof (struct task));
    list_init_item(&task->item, task);

    task->state = state;
    task->registers.ip = ip;
    task->registers.sp = sp;

}

