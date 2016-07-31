#include <abi.h>
#include <fudge.h>

static void send(unsigned int descriptor, unsigned int destination, unsigned int type, unsigned int datacount, void *databuffer)
{

    unsigned char buffer[512];
    struct event_header header;

    header.destination = destination;
    header.type = type;
    header.count = datacount;

    memory_write(buffer, 512, &header, sizeof (struct event_header), 0);
    memory_write(buffer, 512, databuffer, datacount, sizeof (struct event_header));

    file_writeall(descriptor, buffer, sizeof (struct event_header) + datacount);

}

void send_keypress(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct event_keypress keypress;

    keypress.scancode = scancode;

    send(descriptor, destination, EVENT_KEYPRESS, sizeof (struct event_keypress), &keypress);

}

void send_keyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct event_keyrelease keyrelease;

    keyrelease.scancode = scancode;

    send(descriptor, destination, EVENT_KEYRELEASE, sizeof (struct event_keyrelease), &keyrelease);

}

void send_wmmap(unsigned int descriptor)
{

    send(descriptor, 0, EVENT_WMMAP, 0, 0);

}

void send_wmunmap(unsigned int descriptor, unsigned int destination)
{

    send(descriptor, destination, EVENT_WMUNMAP, 0, 0);

}

void send_wmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmresize wmresize;

    wmresize.x = x;
    wmresize.y = y;
    wmresize.w = w;
    wmresize.h = h;

    send(descriptor, destination, EVENT_WMRESIZE, sizeof (struct event_wmresize), &wmresize);

}

void send_wmshow(unsigned int descriptor, unsigned int destination)
{

    send(descriptor, destination, EVENT_WMSHOW, 0, 0);

}

void send_wmhide(unsigned int descriptor, unsigned int destination)
{

    send(descriptor, destination, EVENT_WMHIDE, 0, 0);

}

