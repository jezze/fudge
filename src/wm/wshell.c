#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "send.h"
#include "keymap.h"

static struct element_text content;
static unsigned int contentrows;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static unsigned char text[FUDGE_BSIZE];
static unsigned int textcount;
static unsigned char inputbuffer[FUDGE_BSIZE];
static struct buffer input;
static struct box size;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static void (*handlers[EVENTS])(struct event_header *header, void *data);

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

static void inserttext(unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned char *b = buffer;

    for (i = 0; i < count; i++)
    {

        if (b[i] == '\n')
        {

            contentrows++;

            if (contentrows > 64)
            {

                unsigned int offset = memory_findbyte(text, textcount, '\n');

                memory_copy(text, text + offset, textcount - offset);

                textcount -= offset;

            }

        }

        textcount += memory_write(text, FUDGE_BSIZE, b + i, 1, textcount);

    }

    content.cursor = textcount - 1;

}

static void removetext(unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!textcount)
            break;

        textcount -= 1;

        if (text[textcount] == '\n')
            contentrows--;

    }

    content.cursor = textcount - 1;

}

static void interpret(void)
{

    char command[FUDGE_BSIZE];
    unsigned int count = buffer_rcfifo(&input, FUDGE_BSIZE, command);

    /* This is a temporary fix */
    if (memory_match(command, "cd ", 3))
    {

        unsigned int ok;

        if (count < 4)
            return;

        if (command[3] == '/')
            ok = call_walk(CALL_L1, CALL_PR, count - 5, command + 4);
        else
            ok = call_walk(CALL_L1, CALL_PW, count - 4, command + 3);

        if (ok)
        {

            call_walk(CALL_PW, CALL_L1, 0, 0);
            call_walk(CALL_CW, CALL_L1, 0, 0);

        }

        return;

    }

    if (!call_walk(CALL_CP, CALL_PR, 9, "bin/slang"))
        return;

    if (!call_walk(CALL_L1, CALL_PR, 18, "system/pipe/clone/"))
        return;

    call_walk(CALL_L2, CALL_L1, 1, "0");
    call_walk(CALL_CI, CALL_L1, 1, "1");
    call_walk(CALL_CO, CALL_L1, 1, "1");
    call_open(CALL_L2);
    file_writeall(CALL_L2, command, count);
    call_close(CALL_L2);
    call_spawn();
    call_open(CALL_L2);

    while ((count = file_read(CALL_L2, command, FUDGE_BSIZE)))
        inserttext(count, command);

    call_close(CALL_L2);

}

static void onkeypress(struct event_header *header, void *data)
{

    struct event_keypress *keypress = data;
    struct keycode *keycode;

    switch (keypress->scancode)
    {

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x0E:
        if (!buffer_ecfifo(&input, 1))
            break;

        removetext(2);
        inserttext(1, "\n");
        writetext(header->destination, 1, &content, textcount, text);

        break;

    case 0x1C:
        keycode = getkeycode(KEYMAP_US, keypress->scancode, keymod);

        if (!buffer_wcfifo(&input, keycode->length, &keycode->value))
            break;

        interpret();
        inserttext(3, "$ \n");
        writetext(header->destination, 1, &content, textcount, text);

        break;

    default:
        keycode = getkeycode(KEYMAP_US, keypress->scancode, keymod);

        if (!buffer_wcfifo(&input, keycode->length, &keycode->value))
            break;

        removetext(1);
        inserttext(keycode->length, &keycode->value);
        inserttext(1, "\n");
        writetext(header->destination, 1, &content, textcount, text);

        break;

    }

}

static void onkeyrelease(struct event_header *header, void *data)
{

    struct event_keyrelease *keyrelease = data;

    switch (keyrelease->scancode)
    {

    case 0x2A:
    case 0x36:
        keymod &= ~KEYMOD_SHIFT;

        break;

    }

}

static void onwmunmap(struct event_header *header, void *data)
{

    writetext(header->destination, 0, &content, textcount, text);

    quit = 1;

}

static void onwmresize(struct event_header *header, void *data)
{

    struct event_wmresize *wmresize = data;

    box_setsize(&size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_setsize(&content.size, size.x + 12, size.y + 12, size.w - 24, size.h - 24);

}

static void onwmshow(struct event_header *header, void *data)
{

    writetext(header->destination, 1, &content, textcount, text);

}

static void onwmhide(struct event_header *header, void *data)
{

    writetext(header->destination, 0, &content, textcount, text);

}

static void setup(void)
{

    buffer_init(&input, FUDGE_BSIZE, inputbuffer);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);

    inserttext(3, "$ \n");

}

void main(void)
{

    struct event_header header;
    unsigned int count;

    setup();

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    if (!call_walk(CALL_L0, CALL_PR, 17, "system/event/poll"))
        return;

    call_open(CALL_PO);
    call_open(CALL_L0);
    send_wmmap(CALL_L0);

    while ((count = file_readall(CALL_L0, &header, sizeof (struct event_header))))
    {

        unsigned char data[512];

        if (header.count)
            file_readall(CALL_L0, data, header.count);

        if (handlers[header.type])
            handlers[header.type](&header, data);

        if (datacount)
        {

            file_writeall(CALL_PO, databuffer, datacount);

            datacount = 0;

        }

        if (quit)
            break;

    }

    call_close(CALL_L0);
    call_close(CALL_PO);

}

