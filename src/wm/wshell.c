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
static char textdata[FUDGE_BSIZE];
static struct buffer text;
static char inputdata[FUDGE_BSIZE];
static struct buffer input;
static char outputdata[FUDGE_BSIZE];
static struct buffer output;
static struct box size;
static void (*handlers[EVENTS])(struct event_header *header);

static void inserttext(void *buffer, unsigned int count)
{

    buffer_overwrite(&text, buffer, count);

    content.cursor = text.count - 1;

}

static void removetext(unsigned int count)
{

    buffer_erase(&text, count);

    content.cursor = text.count - 1;

}

static void interpret(void)
{

    char command[FUDGE_BSIZE];
    unsigned int count = buffer_read(&input, command, FUDGE_BSIZE);

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
    file_open(CALL_CO);
    file_open(CALL_CI);
    file_writeall(CALL_CI, command, count);
    call_spawn();
    file_close(CALL_CI);

    while ((count = file_read(CALL_CO, command, FUDGE_BSIZE)))
        inserttext(command, count);

    file_close(CALL_CO);

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
        if (!buffer_erase(&input, 1))
            break;

        removetext(2);
        inserttext("\n", 1);
        print_inserttext(&output, header->destination, &content, 1, text.memory, text.count);

        break;

    case 0x1C:
        keycode = getkeycode(KEYMAP_US, keypress.scancode, keymod);

        if (!buffer_write(&input, &keycode->value, keycode->length))
            break;

        interpret();
        inserttext("$ \n", 3);
        print_inserttext(&output, header->destination, &content, 1, text.memory, text.count);

        break;

    default:
        keycode = getkeycode(KEYMAP_US, keypress.scancode, keymod);

        if (!buffer_write(&input, &keycode->value, keycode->length))
            break;

        removetext(1);
        inserttext(&keycode->value, keycode->length);
        inserttext("\n", 1);
        print_inserttext(&output, header->destination, &content, 1, text.memory, text.count);

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
    box_setsize(&size, wmresize.x, wmresize.y, wmresize.w, wmresize.h);
    box_setsize(&content.size, size.x + 12, size.y + 12, size.w - 24, size.h - 24);

}

static void onwmshow(struct event_header *header)
{

    print_inserttext(&output, header->destination, &content, 1, text.memory, text.count);

}

static void onwmhide(struct event_header *header)
{

    print_removetext(&output, header->destination, &content);

}

void main(void)
{

    struct event_header header;
    unsigned int count;

    buffer_init(&input, FUDGE_BSIZE, inputdata);
    buffer_init(&output, FUDGE_BSIZE, outputdata);
    buffer_init(&text, FUDGE_BSIZE, textdata);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);
    inserttext("$ \n", 3);

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

        if (output.count)
        {

            file_writeall(CALL_PO, output.memory, output.count);
            buffer_init(&output, FUDGE_BSIZE, outputdata);

        }

        if (quit)
            break;

    }

    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

