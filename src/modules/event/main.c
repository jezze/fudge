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
static struct list polllinks;
static struct list keylinks;
static struct list mouselinks;
static struct list ticklinks;
static struct list videolinks;
static struct list wmlinks;

static void unicast(struct list *links, struct event_header *header, unsigned int count)
{

    struct list_item *current;

    for (current = links->head; current; current = current->next)
    {

        struct task *task = current->data;

        if (header->destination != (unsigned int)task)
            continue;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_write(&task->mailbox.buffer, header, count);

    }

}

static void multicast(struct list *links, struct event_header *header, unsigned int count)
{

    struct list_item *current;

    for (current = links->head; current; current = current->next)
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

    multicast(&keylinks, &message.header, sizeof (struct event_header) + sizeof (struct event_keypress));

}

void event_notifykeyrelease(unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.keyrelease.scancode = scancode;

    multicast(&keylinks, &message.header, sizeof (struct event_header) + sizeof (struct event_keyrelease));

}

void event_notifymousemove(char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = 0;
    message.header.destination = 0;
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    multicast(&mouselinks, &message.header, sizeof (struct event_header) + sizeof (struct event_mousemove));

}

void event_notifymousepress(unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.mousepress.button = button;

    multicast(&mouselinks, &message.header, sizeof (struct event_header) + sizeof (struct event_mousepress));

}

void event_notifymouserelease(unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.mouserelease.button = button;

    multicast(&mouselinks, &message.header, sizeof (struct event_header) + sizeof (struct event_mouserelease));

}

void event_notifytick(unsigned int counter)
{

    struct {struct event_header header; struct event_tick tick;} message;

    message.header.type = EVENT_TICK;
    message.header.source = 0;
    message.header.destination = 0;
    message.tick.counter = counter;

    multicast(&ticklinks, &message.header, sizeof (struct event_header) + sizeof (struct event_tick));

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

    multicast(&videolinks, &message.header, sizeof (struct event_header) + sizeof (struct event_videomode));

}

static unsigned int write(struct list *links, struct service_state *state, void *buffer, unsigned int count)
{

    struct event_header *header = buffer;

    header->source = (unsigned int)state->link.data;

    if (header->destination)
        unicast(links, header, count);
    else
        multicast(links, header, count);

    return count;

}

static unsigned int poll_open(struct system_node *self, struct service_state *state)
{

    list_add(&polllinks, &state->link);

    return state->id;

}

static unsigned int poll_close(struct system_node *self, struct service_state *state)
{

    list_remove(&polllinks, &state->link);

    return state->id;

}

static unsigned int poll_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return write(&polllinks, state, buffer, count);

}

static unsigned int key_open(struct system_node *self, struct service_state *state)
{

    list_add(&keylinks, &state->link);

    return state->id;

}

static unsigned int key_close(struct system_node *self, struct service_state *state)
{

    list_remove(&keylinks, &state->link);

    return state->id;

}

static unsigned int key_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return write(&keylinks, state, buffer, count);

}

static unsigned int mouse_open(struct system_node *self, struct service_state *state)
{

    list_add(&mouselinks, &state->link);

    return state->id;

}

static unsigned int mouse_close(struct system_node *self, struct service_state *state)
{

    list_remove(&mouselinks, &state->link);

    return state->id;

}

static unsigned int mouse_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return write(&mouselinks, state, buffer, count);

}

static unsigned int tick_open(struct system_node *self, struct service_state *state)
{

    list_add(&ticklinks, &state->link);

    return state->id;

}

static unsigned int tick_close(struct system_node *self, struct service_state *state)
{

    list_remove(&ticklinks, &state->link);

    return state->id;

}

static unsigned int tick_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return write(&ticklinks, state, buffer, count);

}

static unsigned int video_open(struct system_node *self, struct service_state *state)
{

    list_add(&videolinks, &state->link);

    return state->id;

}

static unsigned int video_close(struct system_node *self, struct service_state *state)
{

    list_remove(&videolinks, &state->link);

    return state->id;

}

static unsigned int video_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return write(&videolinks, state, buffer, count);

}

static unsigned int wm_open(struct system_node *self, struct service_state *state)
{

    list_add(&wmlinks, &state->link);

    return state->id;

}

static unsigned int wm_close(struct system_node *self, struct service_state *state)
{

    list_remove(&wmlinks, &state->link);

    return state->id;

}

static unsigned int wm_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return write(&wmlinks, state, buffer, count);

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

    poll.open = poll_open;
    poll.close = poll_close;
    poll.write = poll_write;
    key.open = key_open;
    key.close = key_close;
    key.write = key_write;
    mouse.open = mouse_open;
    mouse.close = mouse_close;
    mouse.write = mouse_write;
    tick.open = tick_open;
    tick.close = tick_close;
    tick.write = tick_write;
    video.open = video_open;
    video.close = video_close;
    video.write = video_write;
    wm.open = wm_open;
    wm.close = wm_close;
    wm.write = wm_write;

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

