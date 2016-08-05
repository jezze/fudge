#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;
static struct system_node poll;
static struct system_node keypress;
static struct system_node keyrelease;
static struct system_node mousepress;
static struct system_node mouserelease;
static struct system_node mousemove;
static struct system_node wmmap;
static struct system_node wmunmap;
static struct system_node wmresize;
static struct system_node wmshow;
static struct system_node wmhide;
static struct system_node tick;

static void unicast(struct list *list, struct event_header *header)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        if (header->destination != (unsigned int)task)
            continue;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_wcfifo(&task->mailbox.buffer, sizeof (struct event_header) + header->count, header);

    }

}

static void multicast(struct list *list, struct event_header *header)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        header->destination = (unsigned int)task;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_wcfifo(&task->mailbox.buffer, sizeof (struct event_header) + header->count, header);

    }

}

void event_notifykeypress(unsigned char scancode)
{

    struct {struct event_header header; struct event_keypress keypress;} message;

    message.header.type = EVENT_KEYPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_keypress);
    message.keypress.scancode = scancode;

    multicast(&keypress.links, &message.header);

}

void event_notifykeyrelease(unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_keyrelease);
    message.keyrelease.scancode = scancode;

    multicast(&keyrelease.links, &message.header);

}

void event_notifymousemove(char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_mousemove);
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    multicast(&mousemove.links, &message.header);

}

void event_notifymousepress(unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_mousepress);
    message.mousepress.button = button;

    multicast(&mousepress.links, &message.header);

}

void event_notifymouserelease(unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_mouserelease);
    message.mouserelease.button = button;

    multicast(&mouserelease.links, &message.header);

}

void event_notifytick(unsigned int counter)
{

    struct {struct event_header header; struct event_tick tick;} message;

    message.header.type = EVENT_TICK;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_tick);
    message.tick.counter = counter;

    multicast(&tick.links, &message.header);

}

static unsigned int write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct event_header *header = buffer;

    header->source = (unsigned int)state->link.data;

    if (header->destination)
        unicast(&self->links, header);
    else
        multicast(&self->links, header);

    return count;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "event");
    system_initnode(&poll, SYSTEM_NODETYPE_MAILBOX, "poll");
    system_initnode(&keypress, SYSTEM_NODETYPE_MAILBOX, "keypress");
    system_initnode(&keyrelease, SYSTEM_NODETYPE_MAILBOX, "keyrelease");
    system_initnode(&mousepress, SYSTEM_NODETYPE_MAILBOX, "mousepress");
    system_initnode(&mouserelease, SYSTEM_NODETYPE_MAILBOX, "mouserelease");
    system_initnode(&mousemove, SYSTEM_NODETYPE_MAILBOX, "mousemove");
    system_initnode(&wmmap, SYSTEM_NODETYPE_MAILBOX, "wmmap");
    system_initnode(&wmunmap, SYSTEM_NODETYPE_MAILBOX, "wmunmap");
    system_initnode(&wmresize, SYSTEM_NODETYPE_MAILBOX, "wmresize");
    system_initnode(&wmshow, SYSTEM_NODETYPE_MAILBOX, "wmshow");
    system_initnode(&wmhide, SYSTEM_NODETYPE_MAILBOX, "wmhide");
    system_initnode(&tick, SYSTEM_NODETYPE_MAILBOX, "tick");

    poll.write = write;
    keypress.write = write;
    keyrelease.write = write;
    mousepress.write = write;
    mouserelease.write = write;
    mousemove.write = write;
    wmmap.write = write;
    wmunmap.write = write;
    wmresize.write = write;
    wmshow.write = write;
    wmhide.write = write;
    tick.write = write;

    system_addchild(&root, &poll);
    system_addchild(&root, &keypress);
    system_addchild(&root, &keyrelease);
    system_addchild(&root, &mousepress);
    system_addchild(&root, &mouserelease);
    system_addchild(&root, &mousemove);
    system_addchild(&root, &wmmap);
    system_addchild(&root, &wmunmap);
    system_addchild(&root, &wmresize);
    system_addchild(&root, &wmshow);
    system_addchild(&root, &wmhide);
    system_addchild(&root, &tick);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

