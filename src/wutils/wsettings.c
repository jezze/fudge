#include <fudge.h>
#include <abi.h>
#include "kv.h"

struct entry
{

    char *value;
    char *label;

};

static struct entry layouts[2] = {
    {"default", "Default"},
    {"tiling", "Tiling"}
};

static struct entry languages[3] = {
    {"en_US", "English (US)"},
    {"en_GB", "English (UK)"},
    {"sv_SE", "Swedish"}
};

static struct entry timezones[8] = {
    {"UTC", "Coordinated Universal Time (UTC)"},
    {"WET", "Western European Time (WET)"},
    {"CET", "Central European Time (CET)"},
    {"EET", "Eastern European Time (EET)"},
    {"EST", "Eastern Time (EST)"},
    {"CST", "Central Time (CST)"},
    {"MST", "Mountain Time (MST)"},
    {"PST", "Pacific Time (PST)"}
};

static struct entry keyboards[2] = {
    {"qwerty_US", "English (US)"},
    {"qwerty_SE", "Swedish"}
};

static struct entry resolutions[9] = {
    {"640x480x32", "640x480x32"},
    {"800x600x32", "800x600x32"},
    {"1024x768x32", "1024x768x32"},
    {"1280x720x32", "1280x720x32"},
    {"1280x1024x32", "1280x1024x32"},
    {"1366x768x32", "1366x768x32"},
    {"1600x900x32", "1600x900x32"},
    {"1920x1080x32", "1920x1080x32"},
    {"2560x1440x32", "2560x1440x32"}
};
#define NUM_BINDINGS                    32
#define NUM_COLORS                      128

struct binding
{

    char key[32];
    char mod[32];
    char action[32];
    char command[128];

};

struct color
{

    char name[32];
    char value[16];

};

/* everything in the configuration file, so changes in any area are kept until they are saved */
static struct
{

    char layout[32];
    char language[32];
    char time[32];
    char timezone[32];
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    char keyboard[32];
    char speed[16];
    unsigned int acceleration;
    struct binding bindings[NUM_BINDINGS];
    unsigned int nbindings;
    struct color colors[NUM_COLORS];
    unsigned int ncolors;

} config;

static char out[MESSAGE_SIZE];
static unsigned int outcount;
static unsigned int wm;

static void copystring(char *out, unsigned int size, char *in, unsigned int length)
{

    if (length >= size)
        length = size - 1;

    buffer_write(out, size, in, length, 0);

    out[length] = '\0';

}

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

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "= area label \"%s\"\n- content\n+ listbox id \"content\" in \"base\" mode \"readonly\" flow \"vertical-stretch\" padding \"8\" overflow \"scroll\" span \"1\"\n", label);

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

static void addtextbox(char *row, char *id, char *value, unsigned int span, char *halign)
{

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ textbox id \"%s\" in \"%s-row\" flow \"stretch\" span \"%u\" onchange \"q=change&id=%s&value=\"\n+ text in \"%s\" halign \"%s\" label \"%s\"\n", id, row, &span, id, id, halign, value);

}

static void field(char *id, char *label, char *value, char *halign)
{

    beginrow(id);
    addlabel(id, label);
    addtextbox(id, id, value, 0, halign);

}

static void colorfield(char *id, char *label, char *value)
{

    beginrow(id);
    addlabel(id, label);

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ swatch id \"%s-swatch\" in \"%s-row\" color \"%s\"\n+ textbox id \"%s\" in \"%s-row\" onchange \"q=color&id=%s&value=\"\n+ text in \"%s\" label \"%s\"\n", id, id, value, id, id, id, id, value);

}

static char *getlabel(struct entry *options, unsigned int count, char *value)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (cstring_match(options[i].value, value))
            return options[i].label;

    }

    return value;

}

static void choice(char *id, char *label, char *value, struct entry *options, unsigned int count)
{

    unsigned int i;

    beginrow(id);
    addlabel(id, label);

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ select id \"%s\" in \"%s-row\" label \"%s\"\n+ panel id \"%s-list\" in \"%s\" flow \"vertical-stretch\" padding \"4\"\n", id, id, getlabel(options, count, value), id, id);

    for (i = 0; i < count; i++)
        outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ item id \"%s-%u\" in \"%s-list\" padding \"16\" onclick \"q=choose&id=%s&value=%s&label=%s\"\n+ text in \"%s-%u\" label \"%s\"\n", id, &i, id, id, options[i].value, options[i].label, id, &i, options[i].label);

}

static void toggle(char *id, char *label, unsigned int checked)
{

    beginrow(id);
    addlabel(id, label);

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ checkbox id \"%s\" in \"%s-row\" checked \"%u\" onclick \"q=toggle&id=%s\"\n", id, id, &checked, id);

}

static unsigned int getnumber(char *data, unsigned int count, unsigned int index)
{

    char word[16];

    getword(data, count, index, word, 16);

    return cstring_read_value(word, cstring_length(word), 10);

}

static void loadbinding(char *line, unsigned int length)
{

    struct binding *binding = &config.bindings[config.nbindings];
    unsigned int start;
    unsigned int spaces;

    if (config.nbindings >= NUM_BINDINGS)
        return;

    getword(line, length, 0, binding->key, 32);
    getword(line, length, 1, binding->mod, 32);
    getword(line, length, 2, binding->action, 32);

    /* the command is the rest of the line after the third space and can hold spaces */
    for (start = 0, spaces = 0; start < length && spaces < 3; start++)
    {

        if (line[start] == ' ')
            spaces++;

    }

    copystring(binding->command, 128, line + start, (spaces == 3) ? length - start : 0);

    config.nbindings++;

}

static void loadcolor(char *line, unsigned int length)
{

    struct color *color = &config.colors[config.ncolors];

    if (config.ncolors >= NUM_COLORS)
        return;

    getword(line, length, 0, color->name, 32);
    getword(line, length, 1, color->value, 16);

    config.ncolors++;

}

static void loadlines(char *path, char *format, void (*load)(char *line, unsigned int length))
{

    char data[MESSAGE_SIZE];
    unsigned int count = query(data, MESSAGE_SIZE, path, format);
    unsigned int length;
    char *line;

    for (line = data; line < data + count; line += length + 1)
    {

        length = buffer_findbyte(line, data + count - line, '\n');

        if (length)
            load(line, length);

    }

}

static void loadconfig(void)
{

    char data[256];
    char word[16];
    unsigned int count;

    count = query(data, 256, ".general.layout", 0);
    getword(data, count, 0, config.layout, 32);

    count = query(data, 256, ".localization", "{language} {time} {timezone}");
    getword(data, count, 0, config.language, 32);
    getword(data, count, 1, config.time, 32);
    getword(data, count, 2, config.timezone, 32);

    count = query(data, 256, ".screen.resolution", "{width} {height} {bpp}");
    config.width = getnumber(data, count, 0);
    config.height = getnumber(data, count, 1);
    config.bpp = getnumber(data, count, 2);

    count = query(data, 256, ".keyboard.layout", 0);
    getword(data, count, 0, config.keyboard, 32);

    count = query(data, 256, ".mouse", "{speed} {acceleration}");
    getword(data, count, 0, config.speed, 16);
    config.acceleration = cstring_match(getword(data, count, 1, word, 16), "true");

    config.nbindings = 0;
    config.ncolors = 0;

    loadlines(".bindings", "{key} {mod} {action} {command}", loadbinding);
    loadlines(".colors", "{name} {value}", loadcolor);

}

static unsigned int writeconfig(char *data, unsigned int size)
{

    unsigned int count = 0;
    char *acceleration = (config.acceleration) ? "true" : "false";
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  general:\n  {\n    layout: %s\n  }\n", config.layout);
    count += cstring_write_fmt(data, size, count, "  localization:\n  {\n    language: %s\n    time: %s\n    timezone: %s\n  }\n", config.language, config.time, config.timezone);
    count += cstring_write_fmt(data, size, count, "  screen:\n  {\n    resolution:\n    {\n      width: %u\n      height: %u\n      bpp: %u\n    }\n  }\n", &config.width, &config.height, &config.bpp);
    count += cstring_write_fmt(data, size, count, "  keyboard:\n  {\n    layout: %s\n  }\n", config.keyboard);
    count += cstring_write_fmt(data, size, count, "  mouse:\n  {\n    speed: %s\n    acceleration: %s\n  }\n", config.speed, acceleration);
    count += cstring_write_fmt(data, size, count, "  bindings:\n  [\n");

    for (i = 0; i < config.nbindings; i++)
    {

        struct binding *binding = &config.bindings[i];

        if (!cstring_length(binding->key) || !cstring_length(binding->action))
            continue;

        if (cstring_length(binding->command))
            count += cstring_write_fmt(data, size, count, "    { key: %s mod: %s action: %s command: \"%s\" }\n", binding->key, (cstring_length(binding->mod)) ? binding->mod : "none", binding->action, binding->command);
        else
            count += cstring_write_fmt(data, size, count, "    { key: %s mod: %s action: %s }\n", binding->key, (cstring_length(binding->mod)) ? binding->mod : "none", binding->action);

    }

    count += cstring_write_fmt(data, size, count, "  ]\n  colors:\n  [\n");

    for (i = 0; i < config.ncolors; i++)
    {

        struct color *color = &config.colors[i];
        unsigned int pad = (cstring_length(color->name) < 23) ? 23 - cstring_length(color->name) : 0;

        count += cstring_write_fmt(data, size, count, "    { name: %s ", color->name);

        while (pad-- && count < size)
            data[count++] = ' ';

        count += cstring_write_fmt(data, size, count, "value: %s }\n", color->value);

    }

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int savefile(char *path, char *data, unsigned int count)
{

    unsigned int target = fs_auth(path);
    unsigned int parent = (target) ? fs_walkparent(1, target, path) : 0;
    unsigned int id = (parent) ? fs_walk(1, target, 0, path) : 0;
    char *name = path + fs_dirlength(path);

    if (!parent || (id && !fs_remove(1, target, parent, id)))
        return 0;

    id = fs_create(1, target, parent, name, cstring_length(name));

    return id && fs_write_all(1, target, id, data, count, 0) == count;

}

static void setresolution(char *value)
{

    unsigned int width = cstring_read_value(value, buffer_findbyte(value, cstring_length(value), 'x'), 10);
    char *height = value + buffer_findbyte(value, cstring_length(value), 'x') + 1;
    char *bits = height + buffer_findbyte(height, cstring_length(height), 'x') + 1;

    config.width = width;
    config.height = cstring_read_value(height, buffer_findbyte(height, cstring_length(height), 'x'), 10);
    config.bpp = cstring_read_value(bits, cstring_length(bits), 10) / 8;

}

static void setchoice(char *id, char *value)
{

    if (cstring_match(id, "general-layout"))
        copystring(config.layout, 32, value, cstring_length(value));
    else if (cstring_match(id, "localization-language"))
        copystring(config.language, 32, value, cstring_length(value));
    else if (cstring_match(id, "localization-time"))
        copystring(config.time, 32, value, cstring_length(value));
    else if (cstring_match(id, "localization-timezone"))
        copystring(config.timezone, 32, value, cstring_length(value));
    else if (cstring_match(id, "keyboard-layout"))
        copystring(config.keyboard, 32, value, cstring_length(value));
    else if (cstring_match(id, "screen-resolution"))
        setresolution(value);

}

static void setfield(char *id, char *value)
{

    unsigned int length = cstring_length(value);

    if (cstring_match(id, "mouse-speed"))
    {

        copystring(config.speed, 16, value, length);

    }

    else if (buffer_match(id, "color-", 6))
    {

        unsigned int i;

        for (i = 0; i < config.ncolors; i++)
        {

            if (cstring_match(config.colors[i].name, id + 6))
                copystring(config.colors[i].value, 16, value, length);

        }

    }

    else if (buffer_match(id, "binding", 7))
    {

        unsigned int digits = buffer_findbyte(id + 7, cstring_length(id + 7), '-');
        unsigned int index = cstring_read_value(id + 7, digits, 10);
        char *part = id + 7 + digits + 1;

        if (index < config.nbindings)
        {

            struct binding *binding = &config.bindings[index];

            if (cstring_match(part, "key"))
                copystring(binding->key, 32, value, length);
            else if (cstring_match(part, "mod"))
                copystring(binding->mod, 32, value, length);
            else if (cstring_match(part, "action"))
                copystring(binding->action, 32, value, length);
            else if (cstring_match(part, "command"))
                copystring(binding->command, 128, value, length);

        }

    }

}

static void renderlocalization(void)
{

    beginarea("Localization");
    choice("localization-language", "Language", config.language, languages, 3);
    choice("localization-time", "Time format", config.time, languages, 3);
    choice("localization-timezone", "Timezone", config.timezone, timezones, 8);

}

static void rendergeneral(void)
{

    beginarea("General");
    choice("general-layout", "Window layout", config.layout, layouts, 2);

}

static void renderscreen(void)
{

    char resolution[64];
    unsigned int bits = config.bpp * 8;

    cstring_write_fmt(resolution, 64, 0, "%ux%ux%u\\0", &config.width, &config.height, &bits);
    beginarea("Screen");
    choice("screen-resolution", "Resolution", resolution, resolutions, 9);

}

static void renderkeyboard(void)
{

    beginarea("Keyboard");
    choice("keyboard-layout", "Keyboard layout", config.keyboard, keyboards, 2);

}

static void rendermouse(void)
{

    beginarea("Mouse");
    field("mouse-speed", "Speed", config.speed, "right");
    toggle("mouse-acceleration", "Acceleration", config.acceleration);

}

static void renderbindings(void)
{

    unsigned int i;

    beginarea("Bindings");
    beginrow("bindings-header");

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Key\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Modifier\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Action\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Command\"\n");

    for (i = 0; i < config.nbindings; i++)
    {

        struct binding *binding = &config.bindings[i];
        char row[32];
        char id[64];

        cstring_write_fmt(row, 32, 0, "binding%u\\0", &i);
        beginrow(row);
        cstring_write_fmt(id, 64, 0, "binding%u-key\\0", &i);
        addtextbox(row, id, binding->key, 1, "left");
        cstring_write_fmt(id, 64, 0, "binding%u-mod\\0", &i);
        addtextbox(row, id, binding->mod, 1, "left");
        cstring_write_fmt(id, 64, 0, "binding%u-action\\0", &i);
        addtextbox(row, id, binding->action, 1, "left");
        cstring_write_fmt(id, 64, 0, "binding%u-command\\0", &i);
        addtextbox(row, id, binding->command, 1, "left");

    }

}

static void rendercolors(void)
{

    unsigned int i;

    beginarea("Colors");

    for (i = 0; i < config.ncolors; i++)
    {

        char id[64];

        cstring_write_fmt(id, 64, 0, "color-%s\\0", config.colors[i].name);
        colorfield(id, config.colors[i].name, config.colors[i].value);

    }

}

static void renderarea(char *name)
{

    if (cstring_match(name, "localization"))
        renderlocalization();
    else if (cstring_match(name, "screen"))
        renderscreen();
    else if (cstring_match(name, "keyboard"))
        renderkeyboard();
    else if (cstring_match(name, "mouse"))
        rendermouse();
    else if (cstring_match(name, "bindings"))
        renderbindings();
    else if (cstring_match(name, "colors"))
        rendercolors();
    else
        rendergeneral();

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

        setchoice(kv_getstring(event, "id="), kv_getstring(event, "value="));
        channel_send_fmt(0, message->source, EVENT_WMRENDERDATA, "= %s label \"%s\"\n", kv_getstring(event, "id="), kv_getstring(event, "label="));

    }

    else if (kv_match(event, "q=change"))
    {

        setfield(kv_getstring(event, "id="), kv_getstring(event, "value="));

    }

    else if (kv_match(event, "q=color"))
    {

        setfield(kv_getstring(event, "id="), kv_getstring(event, "value="));
        channel_send_fmt(0, message->source, EVENT_WMRENDERDATA, "= %s-swatch color \"%s\"\n", kv_getstring(event, "id="), kv_getstring(event, "value="));

    }

    else if (kv_match(event, "q=toggle"))
    {

        config.acceleration = !config.acceleration;

    }

    else if (kv_match(event, "q=save"))
    {

        static char data[0x2000];

        savefile(option_getstring("config"), data, writeconfig(data, 0x2000));

    }

    else if (kv_match(event, "q=apply"))
    {

        channel_send_fmt(0, message->source, EVENT_OPTION, "config\\0%s\\0", option_getstring("config"));

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wsettings.alfi";

    wm = message->source;

    loadconfig();
    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
    renderarea("general");

}

void init(void)
{

    option_add("wm-service", "wm");
    option_add("config", "initrd:data/config/wm.mq");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}
