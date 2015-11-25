#include <abi.h>
#include <fudge.h>
#include <lib/file.h>
#include "box.h"
#include "element.h"
#include "send.h"

struct client
{

    struct element_text content;
    unsigned int quit;
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
    client->textcount = file_read(CALL_PI, client->text, FUDGE_BSIZE);

}

void main(void)
{

    struct client client;
    struct event_header header;
    unsigned int count;

    setup(&client);

    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    if (!call_walk(CALL_L1, CALL_PR, 17, "system/event/poll"))
        return;

    if (!call_walk(CALL_L2, CALL_PR, 17, "system/event/send"))
        return;

    call_open(CALL_PO);
    call_open(CALL_L1);
    call_open(CALL_L2);
    send_wmmap(CALL_L2);

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

    call_close(CALL_L2);
    call_close(CALL_L1);
    call_close(CALL_PO);

}

