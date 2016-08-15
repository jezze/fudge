#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;
static struct system_node poll;
static struct system_node key;
static struct system_node mouse;
static struct system_node tick;
static struct system_node video;
static struct system_node wm;

static void unicast(struct list *list, struct event_header *header, unsigned int count)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        if (header->destination != (unsigned int)task)
            continue;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_write(&task->mailbox.buffer, header, count);

    }

}

static void multicast(struct list *list, struct event_header *header, unsigned int count)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        header->destination = (unsigned int)task;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_write(&task->mailbox.buffer, header, count);

    }

}

void event_notifykeypress(unsigned char scancode)
{

    struct {struct event_header header; struct event_keypress keypress;} message;

    message.header.type = EVENT_KEYPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.keypress.scancode = scancode;

    multicast(&key.links, &message.header, sizeof (struct event_header) + sizeof (struct event_keypress));

}

void event_notifykeyrelease(unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.keyrelease.scancode = scancode;

    multicast(&key.links, &message.header, sizeof (struct event_header) + sizeof (struct event_keyrelease));

}

void event_notifymousemove(char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = 0;
    message.header.destination = 0;
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    multicast(&mouse.links, &message.header, sizeof (struct event_header) + sizeof (struct event_mousemove));

}

void event_notifymousepress(unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.mousepress.button = button;

    multicast(&mouse.links, &message.header, sizeof (struct event_header) + sizeof (struct event_mousepress));

}

void event_notifymouserelease(unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.mouserelease.button = button;

    multicast(&mouse.links, &message.header, sizeof (struct event_header) + sizeof (struct event_mouserelease));

}

void event_notifytick(unsigned int counter)
{

    struct {struct event_header header; struct event_tick tick;} message;

    message.header.type = EVENT_TICK;
    message.header.source = 0;
    message.header.destination = 0;
    message.tick.counter = counter;

    multicast(&tick.links, &message.header, sizeof (struct event_header) + sizeof (struct event_tick));

}

void event_notifyvideomode(unsigned int w, unsigned int h, unsigned int bpp)
{

    struct {struct event_header header; struct event_videomode videomode;} message;

    message.header.type = EVENT_VIDEOMODE;
    message.header.source = 0;
    message.header.destination = 0;
    message.videomode.w = w;
    message.videomode.h = h;
    message.videomode.bpp = bpp;

    multicast(&video.links, &message.header, sizeof (struct event_header) + sizeof (struct event_videomode));

}

static unsigned int write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct event_header *header = buffer;

    header->source = (unsigned int)state->link.data;

    if (header->destination)
        unicast(&self->links, header, count);
    else
        multicast(&self->links, header, count);

    return count;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "event");
    system_initnode(&poll, SYSTEM_NODETYPE_MAILBOX, "poll");
    system_initnode(&key, SYSTEM_NODETYPE_MAILBOX, "key");
    system_initnode(&mouse, SYSTEM_NODETYPE_MAILBOX, "mouse");
    system_initnode(&tick, SYSTEM_NODETYPE_MAILBOX, "tick");
    system_initnode(&video, SYSTEM_NODETYPE_MAILBOX, "video");
    system_initnode(&wm, SYSTEM_NODETYPE_MAILBOX, "wm");

    poll.write = write;
    key.write = write;
    mouse.write = write;
    tick.write = write;
    video.write = write;
    wm.write = write;

    system_addchild(&root, &poll);
    system_addchild(&root, &key);
    system_addchild(&root, &mouse);
    system_addchild(&root, &tick);
    system_addchild(&root, &video);
    system_addchild(&root, &wm);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

