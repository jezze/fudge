#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "mouse.h"
#include "text.h"
#include "panel.h"
#include "window.h"

static void (*renderers[16])(struct renderable_header *renderable);
static unsigned char data[0x8000];
static unsigned int datacount;

static void printname(char *name)
{

    call_write(CALL_PO, ascii_length(name), name);
    call_write(CALL_PO, 2, ": ");

}

static void printvalue(char *name, unsigned int value)
{

    char num[32];

    printname(name);
    call_write(CALL_PO, ascii_wvalue(num, 32, value, 10, 0), num);

}

static void printstring(char *name, char *value)
{

    printname(name);
    call_write(CALL_PO, 1, "'");
    call_write(CALL_PO, ascii_length(value), value);
    call_write(CALL_PO, 1, "'");

}

static void printdata(char *name, unsigned int count, void *data)
{

    printname(name);
    call_write(CALL_PO, 1, "'");
    call_write(CALL_PO, count, data);
    call_write(CALL_PO, 1, "'");

}

static void printseperator(void)
{

    call_write(CALL_PO, 2, ", ");

}

static void printblockstart(void)
{

    call_write(CALL_PO, 1, "{");

}

static void printblockend(void)
{

    call_write(CALL_PO, 1, "}");

}

static void printnewline(void)
{

    call_write(CALL_PO, 1, "\n");

}

static void printbox(struct box *box)
{

    printblockstart();
    printvalue("x", box->x);
    printseperator();
    printvalue("y", box->y);
    printseperator();
    printvalue("w", box->w);
    printseperator();
    printvalue("h", box->h);
    printblockend();

}

static void printmouse(struct renderable_header *renderable)
{

    printblockstart();
    printvalue("id", renderable->id);
    printseperator();
    printstring("type", "mouse");
    printseperator();
    printvalue("source", renderable->source);
    printseperator();
    printvalue("count", renderable->count);
    printseperator();
    printname("size");
    printbox(&renderable->size);
    printblockend();

}

static void printpanel(struct renderable_header *renderable)
{

    struct panel_header *header = (struct panel_header *)(renderable + 1);

    printblockstart();
    printvalue("id", renderable->id);
    printseperator();
    printstring("type", "panel");
    printseperator();
    printvalue("source", renderable->source);
    printseperator();
    printvalue("count", renderable->count);
    printseperator();
    printname("size");
    printbox(&renderable->size);
    printseperator();
    printvalue("active", header->active);
    printblockend();

}

static void printtext(struct renderable_header *renderable)
{

    struct text_header *header = (struct text_header *)(renderable + 1);

    printblockstart();
    printvalue("id", renderable->id);
    printseperator();
    printstring("type", "text");
    printseperator();
    printvalue("source", renderable->source);
    printseperator();
    printvalue("count", renderable->count);
    printseperator();
    printname("size");
    printbox(&renderable->size);
    printseperator();
    printvalue("type", header->type);
    printseperator();
    printdata("string", renderable->count - sizeof (struct text_header), header + 1);
    printblockend();

}

static void printwindow(struct renderable_header *renderable)
{

    struct window_header *header = (struct window_header *)(renderable + 1);

    printblockstart();
    printvalue("id", renderable->id);
    printseperator();
    printstring("type", "window");
    printseperator();
    printvalue("source", renderable->source);
    printseperator();
    printvalue("count", renderable->count);
    printseperator();
    printname("size");
    printbox(&renderable->size);
    printseperator();
    printvalue("active", header->active);
    printblockend();

}

static struct renderable_header *nextrenderable(unsigned int count, void *data, struct renderable_header *renderable)
{

    if (renderable == 0)
        renderable = data;
    else
        renderable = (struct renderable_header *)((unsigned char *)renderable + sizeof (struct renderable_header) + renderable->count);

    if ((unsigned int)renderable >= (unsigned int)data + count)
        return 0;

    return renderable;

}

static void addrenderable(struct renderable_header *renderable)
{

    struct renderable_header *current = 0;

    while ((current = nextrenderable(datacount, data, current)))
    {

        unsigned int length;

        if (current->source != renderable->source || current->id != renderable->id)
            continue;

        length = sizeof (struct renderable_header) + current->count;

        memory_copy(current, (unsigned char *)current + length, datacount - ((unsigned char *)current - data) - length);

        datacount -= length;

    }

    datacount += memory_write(data, 0x8000, renderable, sizeof (struct renderable_header) + renderable->count, datacount);

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    renderers[RENDERABLE_TYPE_MOUSE] = printmouse;
    renderers[RENDERABLE_TYPE_PANEL] = printpanel;
    renderers[RENDERABLE_TYPE_TEXT] = printtext;
    renderers[RENDERABLE_TYPE_WINDOW] = printwindow;

    call_open(CALL_PI);
    call_open(CALL_PO);

    while ((count = call_read(CALL_PI, FUDGE_BSIZE, buffer)))
    {

        struct renderable_header *renderable;

        renderable = 0;

        while ((renderable = nextrenderable(count, buffer, renderable)))
            addrenderable(renderable);

        renderable = 0;

        while ((renderable = nextrenderable(datacount, data, renderable)))
        {

            renderers[renderable->type](renderable);
            printnewline();

        }

    }

    call_close(CALL_PO);
    call_close(CALL_PI);

}

