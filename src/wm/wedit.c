#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "send.h"
#include "keymap.h"

static struct element_text content;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char inputdata1[FUDGE_BSIZE];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE];
static struct ring input2;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static void (*handlers[EVENTS])(struct event_header *header);

static unsigned int inputend()
{

    return ring_count(&input1);

}

static unsigned int rowleft(unsigned int steps)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count = ring_backread(&input1, buffer, steps);

    if (count)
        ring_backwrite(&input2, buffer, count);

    return inputend();

}

static unsigned int rowright(unsigned int steps)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count = ring_read(&input2, buffer, steps);

    if (count)
        ring_write(&input1, buffer, count);

    return inputend();

}

static unsigned int rowhome()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    while ((count = ring_backread(&input1, buffer, 1)))
    {

        if (buffer[0] == '\n')
        {

            ring_write(&input1, buffer, count);

            break;

        }

        ring_backwrite(&input2, buffer, count);

    }

    return inputend();

}

static unsigned int rowend()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    while ((count = ring_read(&input2, buffer, 1)))
    {

        if (buffer[0] == '\n')
        {

            ring_backwrite(&input2, buffer, count);

            break;

        }

        ring_write(&input1, buffer, count);

    }

    return inputend();

}

static unsigned int rowoffset(position)
{

    unsigned int home = rowhome();
    unsigned int offset = position - home;

    rowright(offset);

    return offset;

}

static unsigned int rowup()
{

    unsigned int position = content.cursor;
    unsigned int offset1;
    unsigned int offset2;

    offset1 = rowoffset(position);
    position = rowhome();

    if (!ring_count(&input1))
        return position;

    position = rowleft(1);
    offset2 = rowoffset(position);
    position = rowhome();

    if (!ring_count(&input1))
        return position;

    return rowright(offset1 < offset2 ? offset1 : offset2);

}

static unsigned int rowdown()
{

    unsigned int position = content.cursor;
    unsigned int start;
    unsigned int offset1;
    unsigned int offset2;

    offset1 = rowoffset(position);
    position = rowend();

    if (!ring_count(&input2))
        return position;

    position = rowright(1);
    start = position;
    position = rowend();
    offset2 = position - start;
    position = rowleft(offset2);

    return rowright(offset1 < offset2 ? offset1 : offset2);

}

static void print(struct event_header *header)
{

    print_inserttext(&output, header->destination, &content, 1, ring_count(&input1) + ring_count(&input2));
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);

}

static void onkeypress(struct event_header *header)
{

    struct event_keypress keypress;
    struct keycode *keycode;

    file_readall(CALL_L0, &keypress, sizeof (struct event_keypress));

    if (header->source == 0)
        return;

    switch (keypress.scancode)
    {

    case 0x0E:
        if (!ring_backskip(&input1, 1))
            break;

        content.cursor = inputend();

        print(header);

        break;

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x47:
        content.cursor = rowhome();

        print(header);

        break;

    case 0x48:
        content.cursor = rowup();

        print(header);

        break;

    case 0x4B:
        content.cursor = rowleft(1);

        print(header);

        break;

    case 0x4D:
        content.cursor = rowright(1);

        print(header);

        break;

    case 0x4F:
        content.cursor = rowend();

        print(header);

        break;

    case 0x50:
        content.cursor = rowdown();

        print(header);

        break;

    default:
        keycode = getkeycode(KEYMAP_US, keypress.scancode, keymod);

        if (!ring_write(&input1, &keycode->value, keycode->length))
            break;

        content.cursor = inputend();

        print(header);

        break;

    }

}

static void onkeyrelease(struct event_header *header)
{

    struct event_keyrelease keyrelease;

    file_readall(CALL_L0, &keyrelease, sizeof (struct event_keyrelease));

    if (header->source == 0)
        return;

    switch (keyrelease.scancode)
    {

    case 0x2A:
    case 0x36:
        keymod &= ~KEYMOD_SHIFT;

        break;

    }

}

static void onwmmap(struct event_header *header)
{

}

static void onwmunmap(struct event_header *header)
{

    quit = 1;

}

static void onwmresize(struct event_header *header)
{

    struct event_wmresize wmresize;

    file_readall(CALL_L0, &wmresize, sizeof (struct event_wmresize));
    box_setsize(&content.size, wmresize.x + 12, wmresize.y + 12, wmresize.w - 24, wmresize.h - 24);

}

static void onwmshow(struct event_header *header)
{

    print(header);

}

static void onwmhide(struct event_header *header)
{

    print_removetext(&output, header->destination, &content);

}

void main(void)
{

    struct event_header header;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    ring_init(&output, FUDGE_BSIZE, outputdata);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);

    if (!file_walk(CALL_L0, "/system/event/poll"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    if (!file_walk(CALL_L2, "/system/event/key"))
        return;

    file_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        ring_write(&input2, buffer, count);

    file_close(CALL_PI);
    file_open(CALL_PO);
    file_open(CALL_L0);
    file_open(CALL_L1);
    file_open(CALL_L2);

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_WMMAP] = onwmmap;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    send_wmmap(CALL_L1, 0);

    while ((count = file_readall(CALL_L0, &header, sizeof (struct event_header))))
    {

        if (!handlers[header.type])
            continue;

        handlers[header.type](&header);

        if (ring_count(&output))
        {

            file_writeall(CALL_PO, outputdata, ring_count(&output));
            ring_reset(&output);

        }

        if (quit)
            break;

    }

    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

