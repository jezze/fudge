#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

static void send(unsigned int descriptor, unsigned int count, void *buffer)
{

    call_walk(descriptor, CALL_PR, 17, "system/event/send");
    call_open(descriptor);
    file_writeall(descriptor, buffer, count);
    call_close(descriptor);

}

static void setheader(struct event_header *header, unsigned int destination, unsigned int type, unsigned int count)
{

    header->destination = destination;
    header->type = type;
    header->count = count;

}

void send_keypress(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct event_keypress keypress;

    setheader(&keypress.header, destination, EVENT_KEYPRESS, sizeof (struct event_keypress) - sizeof (struct event_header));

    keypress.scancode = scancode;

    send(descriptor, sizeof (struct event_keypress), &keypress);

}

void send_keyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct event_keyrelease keyrelease;

    setheader(&keyrelease.header, destination, EVENT_KEYRELEASE, sizeof (struct event_keyrelease) - sizeof (struct event_header));

    keyrelease.scancode = scancode;

    send(descriptor, sizeof (struct event_keyrelease), &keyrelease);

}

void send_wmmap(unsigned int descriptor, unsigned int destination)
{

    struct event_wmmap wmmap;

    setheader(&wmmap.header, destination, EVENT_WMMAP, sizeof (struct event_wmmap) - sizeof (struct event_header));
    send(descriptor, sizeof (struct event_wmmap), &wmmap);

}

void send_wmunmap(unsigned int descriptor, unsigned int destination)
{

    struct event_wmunmap wmunmap;

    setheader(&wmunmap.header, destination, EVENT_WMUNMAP, sizeof (struct event_wmunmap) - sizeof (struct event_header));
    send(descriptor, sizeof (struct event_wmunmap), &wmunmap);

}

void send_wmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmresize wmresize;

    setheader(&wmresize.header, destination, EVENT_WMRESIZE, sizeof (struct event_wmresize) - sizeof (struct event_header));

    wmresize.x = x;
    wmresize.y = y;
    wmresize.w = w;
    wmresize.h = h;

    send(descriptor, sizeof (struct event_wmresize), &wmresize);

}

void send_wmshow(unsigned int descriptor, unsigned int destination)
{

    struct event_wmshow wmshow;

    setheader(&wmshow.header, destination, EVENT_WMSHOW, sizeof (struct event_wmshow) - sizeof (struct event_header));
    send(descriptor, sizeof (struct event_wmshow), &wmshow);

}

void send_wmhide(unsigned int descriptor, unsigned int destination)
{

    struct event_wmhide wmhide;

    setheader(&wmhide.header, destination, EVENT_WMHIDE, sizeof (struct event_wmhide) - sizeof (struct event_header));
    send(descriptor, sizeof (struct event_wmhide), &wmhide);

}

