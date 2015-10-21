#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

static void (*renderers[16])(struct element *element);
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

static void printmouse(struct element *element)
{

    printblockstart();
    printvalue("id", element->id);
    printseperator();
    printstring("type", "mouse");
    printseperator();
    printvalue("source", element->source);
    printseperator();
    printvalue("count", element->count);
    printseperator();
    printname("size");
    printbox(&element->size);
    printblockend();

}

static void printpanel(struct element *element)
{

    struct element_panel *panel = (struct element_panel *)element;

    printblockstart();
    printvalue("id", element->id);
    printseperator();
    printstring("type", "panel");
    printseperator();
    printvalue("source", element->source);
    printseperator();
    printvalue("count", element->count);
    printseperator();
    printname("size");
    printbox(&element->size);
    printseperator();
    printvalue("active", panel->active);
    printblockend();

}

static void printtext(struct element *element)
{

    struct element_text *text = (struct element_text *)element;

    printblockstart();
    printvalue("id", element->id);
    printseperator();
    printstring("type", "text");
    printseperator();
    printvalue("source", element->source);
    printseperator();
    printvalue("count", element->count);
    printseperator();
    printname("size");
    printbox(&element->size);
    printseperator();
    printvalue("type", text->type);
    printseperator();
    printdata("string", element->count - (sizeof (struct element_text) - sizeof (struct element)), text + 1);
    printblockend();

}

static void printwindow(struct element *element)
{

    struct element_window *window = (struct element_window *)element;

    printblockstart();
    printvalue("id", element->id);
    printseperator();
    printstring("type", "window");
    printseperator();
    printvalue("source", element->source);
    printseperator();
    printvalue("count", element->count);
    printseperator();
    printname("size");
    printbox(&element->size);
    printseperator();
    printvalue("active", window->active);
    printblockend();

}

static struct element *nextelement(unsigned int count, void *data, struct element *element)
{

    if (element == 0)
        element = data;
    else
        element = (struct element *)((unsigned char *)element + sizeof (struct element) + element->count);

    if ((unsigned int)element >= (unsigned int)data + count)
        return 0;

    return element;

}

static void addelement(struct element *element)
{

    struct element *current = 0;

    while ((current = nextelement(datacount, data, current)))
    {

        unsigned int length;

        if (current->source != element->source || current->id != element->id)
            continue;

        length = sizeof (struct element) + current->count;

        memory_copy(current, (unsigned char *)current + length, datacount - ((unsigned char *)current - data) - length);

        datacount -= length;

    }

    datacount += memory_write(data, 0x8000, element, sizeof (struct element) + element->count, datacount);

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    renderers[ELEMENT_TYPE_MOUSE] = printmouse;
    renderers[ELEMENT_TYPE_PANEL] = printpanel;
    renderers[ELEMENT_TYPE_TEXT] = printtext;
    renderers[ELEMENT_TYPE_WINDOW] = printwindow;

    call_open(CALL_PI);
    call_open(CALL_PO);

    while ((count = call_read(CALL_PI, FUDGE_BSIZE, buffer)))
    {

        struct element *element;

        element = 0;

        while ((element = nextelement(count, buffer, element)))
            addelement(element);

        element = 0;

        while ((element = nextelement(datacount, data, element)))
        {

            renderers[element->type](element);
            printnewline();

        }

    }

    call_close(CALL_PO);
    call_close(CALL_PI);

}

