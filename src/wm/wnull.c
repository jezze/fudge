#include <abi.h>
#include <fudge.h>
#include <lib/file.h>
#include "box.h"
#include "element.h"
#include "send.h"
#include "keymap.h"

struct client
{

    struct element_text content;
    unsigned int quit;
    unsigned int keymod;
    unsigned char text[FUDGE_BSIZE];
    unsigned int textcount;
    struct box size;

};

static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static void (*handlers[EVENTS])(struct client *client, struct event_header *header, void *data);

static void writeelement(unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, type, source, z, count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &element, sizeof (struct element), datacount);

}

static void writetext(unsigned int source, unsigned int z, struct element_text *text, unsigned int count, void *buffer)
{

    writeelement((unsigned int)text, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, text, sizeof (struct element_text), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static void flush(void)
{

    if (datacount)
    {

        file_writeall(CALL_PO, databuffer, datacount);

        datacount = 0;

    }

}

static void onkeypress(struct client *client, struct event_header *header, void *data)
{

    struct event_keypress *keypress = data;
    struct keycode *keycode;

    switch (keypress->scancode)
    {

    case 0x2A:
    case 0x36:
        client->keymod |= KEYMOD_SHIFT;

        break;

    case 0x0E:
        if (!client->textcount)
            break;

        client->textcount -= 1;

        writetext(header->destination, 1, &client->content, client->textcount, client->text);

        break;

    default:
        keycode = getkeycode(KEYMAP_US, keypress->scancode, client->keymod);
        client->textcount += memory_write(client->text, FUDGE_BSIZE, keycode->value, keycode->length, client->textcount);

        writetext(header->destination, 1, &client->content, client->textcount, client->text);

        break;
    }

}

static void onkeyrelease(struct client *client, struct event_header *header, void *data)
{

    struct event_keyrelease *keyrelease = data;

    switch (keyrelease->scancode)
    {

    case 0x2A:
    case 0x36:
        client->keymod &= ~KEYMOD_SHIFT;

        break;

    }

}

static void onwmunmap(struct client *client, struct event_header *header, void *data)
{

    writetext(header->destination, 0, &client->content, client->textcount, client->text);

    client->quit = 1;

}

static void onwmresize(struct client *client, struct event_header *header, void *data)
{

    struct event_wmresize *wmresize = data;

    box_setsize(&client->size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_setsize(&client->content.size, client->size.x + 12, client->size.y + 12, client->size.w - 24, client->size.h - 24);

}

static void onwmshow(struct client *client, struct event_header *header, void *data)
{

    writetext(header->destination, 1, &client->content, client->textcount, client->text);

}

static void onwmhide(struct client *client, struct event_header *header, void *data)
{

    writetext(header->destination, 0, &client->content, client->textcount, client->text);

}

static void setup(struct client *client)
{

    element_inittext(&client->content, ELEMENT_TEXTTYPE_NORMAL);

    client->quit = 0;
    client->keymod = KEYMOD_NONE;
    client->textcount = 0;

}

void main(void)
{

    struct client client;
    struct event_header header;
    unsigned int count;

    setup(&client);

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    call_open(CALL_PO);
    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);
    send_wmmap(CALL_L2, 0);

    while ((count = file_readall(CALL_L1, &header, sizeof (struct event_header))))
    {

        unsigned char data[512];

        if (header.count)
            file_readall(CALL_L1, data, header.count);

        if (handlers[header.type])
        {

            handlers[header.type](&client, &header, data);
            flush();

        }

        if (client.quit)
            break;

    }

    call_close(CALL_L1);
    call_close(CALL_PO);

}

