#include <fudge.h>
#include <abi.h>
#include "kv.h"

static char *layouts[2] = {"default", "tiling"};
static char *keyboards[2] = {"qwerty_US", "qwerty_SE"};
static char out[MESSAGE_SIZE];
static unsigned int outcount;
static unsigned int wm;

static void flush(void)
{

    if (outcount)
        channel_send(0, wm, EVENT_WMRENDERDATA, outcount, out);

    outcount = 0;

}

static void reserve(void)
{

    if (outcount + 1024 > MESSAGE_SIZE)
        flush();

}

static unsigned int query(char *data, unsigned int size, char *path, char *format)
{

    return (format) ? system_feed(0, 0, data, size, "mq -query %s -format \"%s\" %s", path, format, option_getstring("config")) : system_feed(0, 0, data, size, "mq -query %s %s", path, option_getstring("config"));

}

static char *getword(char *data, unsigned int count, unsigned int index, char *word, unsigned int size)
{

    unsigned int start = 0;
    unsigned int i;

    word[0] = '\0';

    for (i = 0; i <= count; i++)
    {

        if (i == count || data[i] == ' ' || data[i] == '\n')
        {

            if (i > start && !index--)
            {

                unsigned int length = i - start;

                if (length < size)
                    cstring_write_fmt(word, size, 0, "%w\\0", data + start, &length);

                break;

            }

            start = i + 1;

        }

    }

    return word;

}

static void beginarea(char *label)
{

    reserve();

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "= area label \"%s\"\n- content\n+ listbox id \"content\" in \"base\" mode \"readonly\" flow \"vertical-stretch\" overflow \"vscroll\" span \"1\"\n", label);

}

static void beginrow(char *id)
{

    reserve();

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ layout id \"%s-row\" in \"content\" flow \"horizontal-stretch\" spacing \"8\" padding \"4\"\n", id);

}

static void addlabel(char *id, char *label)
{

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ text in \"%s-row\" valign \"middle\" span \"1\" label \"%s\"\n", id, label);

}

static void addtextbox(char *row, char *id, char *value)
{

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ textbox id \"%s\" in \"%s-row\" span \"1\"\n+ text in \"%s\" label \"%s\"\n", id, row, id, value);

}

static void field(char *id, char *label, char *value)
{

    beginrow(id);
    addlabel(id, label);
    addtextbox(id, id, value);

}

static void choice(char *id, char *label, char *value, char **options, unsigned int count)
{

    unsigned int i;

    beginrow(id);
    addlabel(id, label);

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ select id \"%s\" in \"%s-row\" label \"%s\"\n+ panel id \"%s-list\" in \"%s\" flow \"vertical-stretch\" padding \"4\"\n", id, id, value, id, id);

    for (i = 0; i < count; i++)
        outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ item id \"%s-%u\" in \"%s-list\" padding \"16\" onclick \"q=choose&id=%s&value=%s\"\n+ text in \"%s-%u\" label \"%s\"\n", id, &i, id, id, options[i], id, &i, options[i]);

}

static void toggle(char *id, char *label, unsigned int checked)
{

    beginrow(id);
    addlabel(id, label);

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ checkbox id \"%s\" in \"%s-row\" checked \"%u\"\n", id, id, &checked);

}

static void renderlocalization(void)
{

    char data[256];
    char word[64];
    unsigned int count = query(data, 256, ".localization", "{language} {time} {timezone}");

    beginarea("Localization");
    field("localization-language", "Language", getword(data, count, 0, word, 64));
    field("localization-time", "Time format", getword(data, count, 1, word, 64));
    field("localization-timezone", "Timezone", getword(data, count, 2, word, 64));

}

static void renderscreen(void)
{

    char data[256];
    char word[64];
    unsigned int count = query(data, 256, ".screen.resolution", "{width} {height} {bpp}");

    beginarea("Screen");
    field("screen-width", "Width", getword(data, count, 0, word, 64));
    field("screen-height", "Height", getword(data, count, 1, word, 64));
    field("screen-bpp", "Bytes per pixel", getword(data, count, 2, word, 64));

}

static void renderlayout(void)
{

    char data[256];
    char word[64];
    unsigned int count = query(data, 256, ".layout", 0);

    beginarea("Layout");
    choice("layout", "Window layout", getword(data, count, 0, word, 64), layouts, 2);

}

static void renderkeyboard(void)
{

    char data[256];
    char word[64];
    unsigned int count = query(data, 256, ".keyboard.layout", 0);

    beginarea("Keyboard");
    choice("keyboard-layout", "Keyboard layout", getword(data, count, 0, word, 64), keyboards, 2);

}

static void rendermouse(void)
{

    char data[256];
    char word[64];
    unsigned int count = query(data, 256, ".mouse", "{speed} {acceleration}");

    beginarea("Mouse");
    field("mouse-speed", "Speed", getword(data, count, 0, word, 64));
    toggle("mouse-acceleration", "Acceleration", cstring_match(getword(data, count, 1, word, 64), "true"));

}

static void renderbindings(void)
{

    char data[MESSAGE_SIZE];
    unsigned int count = query(data, MESSAGE_SIZE, ".bindings", "{key} {mod} {action} {command}");
    unsigned int length;
    unsigned int i = 0;
    char *line;

    beginarea("Bindings");
    beginrow("bindings-header");

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Key\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Modifier\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Action\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Command\"\n");

    for (line = data; line < data + count; line += length + 1, i++)
    {

        char row[32];
        char id[64];
        char word[128];
        unsigned int start;
        unsigned int spaces;
        unsigned int command;

        length = buffer_findbyte(line, data + count - line, '\n');

        cstring_write_fmt(row, 32, 0, "binding%u\\0", &i);
        beginrow(row);
        cstring_write_fmt(id, 64, 0, "binding%u-key\\0", &i);
        addtextbox(row, id, getword(line, length, 0, word, 128));
        cstring_write_fmt(id, 64, 0, "binding%u-mod\\0", &i);
        addtextbox(row, id, getword(line, length, 1, word, 128));
        cstring_write_fmt(id, 64, 0, "binding%u-action\\0", &i);
        addtextbox(row, id, getword(line, length, 2, word, 128));

        /* the command is the rest of the line after the third space and can hold spaces */
        for (start = 0, spaces = 0; start < length && spaces < 3; start++)
        {

            if (line[start] == ' ')
                spaces++;

        }

        command = length - start;
        word[0] = '\0';

        if (spaces == 3 && command < 128)
            cstring_write_fmt(word, 128, 0, "%w\\0", line + start, &command);

        cstring_write_fmt(id, 64, 0, "binding%u-command\\0", &i);
        addtextbox(row, id, word);

    }

}

static void rendercolors(void)
{

    char data[MESSAGE_SIZE];
    unsigned int count = query(data, MESSAGE_SIZE, ".colors", "{name} {value}");
    unsigned int length;
    char *line;

    beginarea("Colors");

    for (line = data; line < data + count; line += length + 1)
    {

        char id[96];
        char name[64];
        char value[16];

        length = buffer_findbyte(line, data + count - line, '\n');

        getword(line, length, 0, name, 64);
        getword(line, length, 1, value, 16);
        cstring_write_fmt(id, 96, 0, "color-%s\\0", name);
        field(id, name, value);

    }

}

static void renderarea(char *name)
{

    if (cstring_match(name, "localization"))
        renderlocalization();
    else if (cstring_match(name, "layout"))
        renderlayout();
    else if (cstring_match(name, "keyboard"))
        renderkeyboard();
    else if (cstring_match(name, "mouse"))
        rendermouse();
    else if (cstring_match(name, "bindings"))
        renderbindings();
    else if (cstring_match(name, "colors"))
        rendercolors();
    else
        renderscreen();

    flush();

}

static void onmain(struct message *message)
{

    unsigned int service = channel_lookup(option_getstring("wm-service"));

    if (service)
    {

        channel_send(0, service, EVENT_WMMAP, 0, 0);
        channel_hold(0);
        channel_send(0, service, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwmevent(struct message *message)
{

    struct event_wmevent *event = message->data;

    if (kv_match(event, "q=area"))
    {

        renderarea(kv_getstring(event, "name="));

    }

    else if (kv_match(event, "q=choose"))
    {

        channel_send_fmt(0, message->source, EVENT_WMRENDERDATA, "= %s label \"%s\"\n", kv_getstring(event, "id="), kv_getstring(event, "value="));

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wsettings.alfi";

    wm = message->source;

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
    renderarea("screen");

}

void init(void)
{

    option_add("wm-service", "wm");
    option_add("config", "initrd:data/config/wm.mq");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}
