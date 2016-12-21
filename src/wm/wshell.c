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
static char textdata[FUDGE_BSIZE];
static struct ring text;
static void (*handlers[EVENTS])(struct event_header *header);

static void readback()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(CALL_CO);

    while ((count = file_read(CALL_CO, buffer, FUDGE_BSIZE)))
        ring_overwrite(&text, buffer, count);

    file_close(CALL_CO);

}

static void interpret(struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    if (count < 2)
        return;

    /* This is a temporary fix */
    if (memory_match(command, "cd ", 3))
    {

        if (count < 4)
            return;

        command[count - 1] = '\0';

        if (file_walk(CALL_L8, command + 3))
        {

            file_duplicate(CALL_PW, CALL_L8);
            file_duplicate(CALL_CW, CALL_L8);

        }

        return;

    }

    if (!file_walk(CALL_CP, "/bin/slang"))
        return;

    if (!file_walk(CALL_L8, "/system/pipe/clone/"))
        return;

    file_walkfrom(CALL_CI, CALL_L8, "0");
    file_walkfrom(CALL_CO, CALL_L8, "1");
    file_open(CALL_CI);
    file_writeall(CALL_CI, command, count);
    call_spawn();
    file_close(CALL_CI);
    readback();

}

static unsigned int inputend()
{

    return ring_count(&text) + ring_count(&input1);

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

    return rowleft(ring_count(&input1));

}

static unsigned int rowend()
{

    return rowright(ring_count(&input2));

}

static void print(struct event_header *header)
{

    print_inserttext(&output, header->destination, &content, 1, ring_count(&text) + ring_count(&input1) + ring_count(&input2) + 1);
    ring_copy(&output, &text);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

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

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x0E:
        if (!ring_backskip(&input1, 1))
            break;

        content.cursor = inputend();

        print(header);

        break;

    case 0x1C:
        ring_move(&input1, &input2);

        keycode = getkeycode(KEYMAP_US, keypress.scancode, keymod);

        if (!ring_write(&input1, &keycode->value, keycode->length))
            break;

        ring_overcopy(&text, &input1);
        interpret(&input1);
        ring_overwrite(&text, "$ ", 2);

        content.cursor = inputend();

        print(header);

        break;

    case 0x47:
        content.cursor = rowhome();

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
    unsigned int count;

    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&text, FUDGE_BSIZE, textdata);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);
    ring_overwrite(&text, "$ ", 2);

    content.cursor = inputend();

    if (!file_walk(CALL_L0, "/system/event/poll"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    if (!file_walk(CALL_L2, "/system/event/key"))
        return;

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

