#include <abi.h>
#include <fudge.h>
#include <math/box.h>
#include <keymap/keymap.h>
#include <widget/widget.h>

static struct widget_textbox content;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static char promptdata[FUDGE_BSIZE];
static struct ring prompt;
static char inputdata1[FUDGE_BSIZE];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE];
static struct ring input2;
static char textdata[FUDGE_BSIZE];
static struct ring text;
static unsigned int totalrows;
static unsigned int visiblerows;
static unsigned int rendertarget;

static void updatecontent(struct event_header *iheader)
{

    content.length = ring_count(&text) + ring_count(&prompt) + ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&text) + ring_count(&prompt) + ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_MIDDLE, iheader->target, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, content.size.x, content.size.y, content.size.w, content.size.h);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &text);
    ring_copy(&output, &prompt);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void removecontent(struct event_header *iheader)
{

    widget_remove(&output, &content, WIDGET_Z_MIDDLE, iheader->target);

}

static void removerows(unsigned int count)
{

    while (count--)
    {

        ring_skip(&text, ring_find(&text, '\n') + 1);

        totalrows--;

    }

}

static void copybuffer(void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!ring_avail(&text))
            removerows(1);

        if (b[i] == '\n')
        {

            totalrows++;

            if (totalrows >= visiblerows)
                removerows(totalrows - visiblerows + 1);

        }

        ring_write(&text, &b[i], 1);

    }

}

static void copyring(struct ring *ring)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;

    while ((count = ring_read(ring, buffer, FUDGE_BSIZE)))
        copybuffer(buffer, count);

    ring->head = head;
    ring->tail = tail;

}

static unsigned int complete(struct event_header *iheader, struct event_header *oheader, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);
    unsigned int id;

    if (!file_walk(FILE_CP, "/bin/complete"))
        return 0;

    id = call_spawn();

    if (id)
    {

        event_request(oheader, iheader, EVENT_INIT, id);
        event_send(oheader);
        event_request(oheader, iheader, EVENT_DATA, id);
        event_adddata(oheader, 1);
        event_appenddata(oheader, count, command);
        event_send(oheader);
        event_request(oheader, iheader, EVENT_EXIT, id);
        event_send(oheader);

    }

    return id;

}

static void printnormal(void *buffer, unsigned int count)
{

    copybuffer(buffer, count);

}

static void printcomplete(void *buffer, unsigned int count)
{

    if (memory_findbyte(buffer, count, '\n') < count - 1)
    {

        copyring(&prompt);
        copybuffer("\n", 1);
        copybuffer(buffer, count);

    }

    else
    {

        ring_write(&input1, buffer, count - 1);

    }

}

static unsigned int interpretbuiltin(unsigned int count, char *command)
{

    if (memory_match(command, "cd ", 3))
    {

        command[count - 1] = '\0';

        if (file_walk(FILE_L0, command + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        return 1;

    }

    return 0;

}

static unsigned int interpret(struct event_header *iheader, struct event_header *oheader, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);
    unsigned int id;

    if (count < 2)
        return 0;

    if (interpretbuiltin(count, command))
        return 0;

    if (!file_walk(FILE_CP, "/bin/slang"))
        return 0;

    id = call_spawn();

    if (id)
    {

        event_request(oheader, iheader, EVENT_INIT, id);
        event_send(oheader);
        event_request(oheader, iheader, EVENT_DATA, id);
        event_adddata(oheader, 0);
        event_appenddata(oheader, count, command);
        event_send(oheader);
        event_request(oheader, iheader, EVENT_EXIT, id);
        event_send(oheader);

    }

    return id;

}

static void moveleft(unsigned int steps)
{

    char buffer[FUDGE_BSIZE];

    ring_writereverse(&input2, buffer, ring_readreverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[FUDGE_BSIZE];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

}

static void oninit(struct event_header *iheader, struct event_header *oheader)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&prompt, FUDGE_BSIZE, promptdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    ring_init(&text, FUDGE_BSIZE, textdata);
    widget_inittextbox(&content);
    ring_write(&prompt, "$ ", 2);
    event_request(oheader, iheader, EVENT_WMMAP, EVENT_ADDR_BROADCAST);
    event_send(oheader);

}

static void onexit(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_EXIT);
    event_send(oheader);

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_request(oheader, iheader, EVENT_WMUNMAP, EVENT_ADDR_BROADCAST);
    event_send(oheader);

    quit = 1;

}

static void ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    switch (data->session)
    {

    case 0:
        printnormal(data + 1, data->count);

        break;

    case 1:
        printcomplete(data + 1, data->count);

        break;

    }

    updatecontent(iheader);

}

static void onwmkeypress(struct event_header *iheader, struct event_header *oheader)
{

    struct event_wmkeypress *wmkeypress = event_getdata(iheader);
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, wmkeypress->scancode, keymod);

    keymod = keymap_modkey(wmkeypress->scancode, keymod);

    switch (wmkeypress->scancode)
    {

    case 0x0E:
        ring_skipreverse(&input1, 1);

        break;

    case 0x0F:
        ring_move(&input1, &input2);
        complete(iheader, oheader, &input1);

        break;

    case 0x1C:
        ring_move(&input1, &input2);
        ring_write(&input1, keycode->value, keycode->length);
        copyring(&prompt);
        copyring(&input1);
        interpret(iheader, oheader, &input1);

        break;

    case 0x16:
        if (keymod & KEYMOD_CTRL)
            ring_reset(&input1);
        else
            ring_write(&input1, keycode->value, keycode->length);

        break;

    case 0x25:
        if (keymod & KEYMOD_CTRL)
            ring_reset(&input2);
        else
            ring_write(&input1, keycode->value, keycode->length);

        break;

    case 0x26:
        if (keymod & KEYMOD_CTRL)
            removerows(totalrows);
        else
            ring_write(&input1, keycode->value, keycode->length);

        break;

    case 0x47:
        moveleft(ring_count(&input1));

        break;

    case 0x4B:
        moveleft(1);

        break;

    case 0x4D:
        moveright(1);

        break;

    case 0x4F:
        moveright(ring_count(&input2));

        break;

    default:
        ring_write(&input1, keycode->value, keycode->length);

        break;

    }

    updatecontent(iheader);

}

static void onwmkeyrelease(struct event_header *iheader, struct event_header *oheader)
{

    struct event_wmkeyrelease *wmkeyrelease = event_getdata(iheader);

    keymod = keymap_modkey(wmkeyrelease->scancode, keymod);

}

static void onwmconfigure(struct event_header *iheader, struct event_header *oheader)
{

    struct event_wmconfigure *wmconfigure = event_getdata(iheader);

    rendertarget = wmconfigure->rendertarget;

    box_setsize(&content.size, wmconfigure->x, wmconfigure->y, wmconfigure->w, wmconfigure->h);
    box_resize(&content.size, wmconfigure->padding);

    visiblerows = content.size.h / wmconfigure->lineheight;

    if (totalrows >= visiblerows)
        removerows(totalrows - visiblerows + 1);

}

static void onwmshow(struct event_header *iheader, struct event_header *oheader)
{

    updatecontent(iheader);

}

static void onwmhide(struct event_header *iheader, struct event_header *oheader)
{

    removecontent(iheader);

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_INIT:
            oninit(iheader, oheader);

            break;

        case EVENT_EXIT:
            onexit(iheader, oheader);

            break;

        case EVENT_KILL:
            onkill(iheader, oheader);

            break;

        case EVENT_DATA:
            ondata(iheader, oheader);

            break;

        case EVENT_WMKEYPRESS:
            onwmkeypress(iheader, oheader);

            break;

        case EVENT_WMKEYRELEASE:
            onwmkeyrelease(iheader, oheader);

            break;

        case EVENT_WMCONFIGURE:
            onwmconfigure(iheader, oheader);

            break;

        case EVENT_WMSHOW:
            onwmshow(iheader, oheader);

            break;

        case EVENT_WMHIDE:
            onwmhide(iheader, oheader);

            break;

        }

        if (ring_count(&output))
        {

            event_request(oheader, iheader, EVENT_WMFLUSH, rendertarget);
            event_addwmflush(oheader, ring_count(&output), outputdata);
            event_send(oheader);
            ring_reset(&output);

        }

    }

    event_close();

}

