#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define NUM_WIDGETS                     14
#define NUM_RULES                       16
#define NUM_SHADES                      4
#define NUM_PARTS                       6
#define SHADE_NORMAL                    1
#define SHADE_HOVER                     2
#define SHADE_FOCUS                     4
#define SHADE_DISABLED                  8
#define SHADE_ALL                       15
#define PART_SHADOW                     1
#define PART_BODY                       2
#define PART_DARK                       4
#define PART_LIGHT                      8
#define PART_FRAME                      15
#define PART_TEXT                       16
#define PART_ICON                       32
#define NUM_BINDINGS                    32
#define NUM_COLORS                      512
#define NUM_KEYMAPS                     21
#define NUM_LAYOUTS                     2
#define NUM_LANGUAGES                   3
#define NUM_TIMEZONES                   8
#define NUM_RESOLUTIONS                 (1 + MESSAGE_VIDEOMODES)
#define NUM_TRANSPARENCIES              2
#define NUM_CURSORS                     4
#define NUM_DOUBLETAPS                  2
#define NUM_INVERTS                     2
#define NUM_AREAS                       4
#define NUM_NETWORKTYPES                2
#define NUM_SPEEDS                      16
#define NUM_VOLUMES                     11
#define NUM_VIDEOS                      4
#define NUM_DEVICES                     8
#define DEVICENAME                      32

struct entry
{

    char *value;
    char *label;

};

static struct entry layouts[NUM_LAYOUTS] = {
    {"default", "Default"},
    {"tiling", "Tiling"}
};

static struct entry transparencies[NUM_TRANSPARENCIES] = {
    {"on", "On"},
    {"off", "Off"}
};

static struct entry cursors[NUM_CURSORS] = {
    {"100", "100%"},
    {"150", "150%"},
    {"200", "200%"},
    {"300", "300%"}
};

static struct entry languages[NUM_LANGUAGES] = {
    {"en_US", "English (US)"},
    {"en_GB", "English (UK)"},
    {"sv_SE", "Swedish"}
};

static struct entry timezones[NUM_TIMEZONES] = {
    {"UTC", "Coordinated Universal Time (UTC)"},
    {"WET", "Western European Time (WET)"},
    {"CET", "Central European Time (CET)"},
    {"EET", "Eastern European Time (EET)"},
    {"EST", "Eastern Time (EST)"},
    {"CST", "Central Time (CST)"},
    {"MST", "Mountain Time (MST)"},
    {"PST", "Pacific Time (PST)"}
};

static struct entry keymaps[NUM_KEYMAPS] = {
    {"initrd:data/keys/en_US.mq", "English (US)"},
    {"initrd:data/keys/cs_CZ.mq", "Czech"},
    {"initrd:data/keys/da_DK.mq", "Danish"},
    {"initrd:data/keys/en_GB.mq", "English (UK)"},
    {"initrd:data/keys/fi_FI.mq", "Finnish"},
    {"initrd:data/keys/fr_FR.mq", "French"},
    {"initrd:data/keys/fr_BE.mq", "French (Belgium)"},
    {"initrd:data/keys/fr_CA.mq", "French (Canada)"},
    {"initrd:data/keys/fr_CH.mq", "French (Switzerland)"},
    {"initrd:data/keys/de_DE.mq", "German"},
    {"initrd:data/keys/de_CH.mq", "German (Switzerland)"},
    {"initrd:data/keys/hu_HU.mq", "Hungarian"},
    {"initrd:data/keys/it_IT.mq", "Italian"},
    {"initrd:data/keys/nb_NO.mq", "Norwegian"},
    {"initrd:data/keys/pl_PL.mq", "Polish"},
    {"initrd:data/keys/pt_PT.mq", "Portuguese"},
    {"initrd:data/keys/pt_BR.mq", "Portuguese (Brazil)"},
    {"initrd:data/keys/es_ES.mq", "Spanish"},
    {"initrd:data/keys/es_419.mq", "Spanish (Latin America)"},
    {"initrd:data/keys/sv_SE.mq", "Swedish"},
    {"initrd:data/keys/tr_TR.mq", "Turkish"}
};

static struct entry doubletaps[NUM_DOUBLETAPS] = {
    {"drag", "Drag"},
    {"right", "Right click"}
};

static struct entry inverts[NUM_INVERTS] = {
    {"off", "Off"},
    {"on", "On"}
};

static struct entry areas[NUM_AREAS] = {
    {"left", "Left click"},
    {"middle", "Middle click"},
    {"right", "Right click"},
    {"off", "Off"}
};

static struct entry speeds[NUM_SPEEDS] = {
    {"1", "1"},
    {"2", "2"},
    {"3", "3"},
    {"4", "4"},
    {"5", "5"},
    {"6", "6"},
    {"7", "7"},
    {"8", "8"},
    {"9", "9"},
    {"10", "10"},
    {"11", "11"},
    {"12", "12"},
    {"13", "13"},
    {"14", "14"},
    {"15", "15"},
    {"16", "16"}
};

static struct entry volumes[NUM_VOLUMES] = {
    {"0", "0%"},
    {"10", "10%"},
    {"20", "20%"},
    {"30", "30%"},
    {"40", "40%"},
    {"50", "50%"},
    {"60", "60%"},
    {"70", "70%"},
    {"80", "80%"},
    {"90", "90%"},
    {"100", "100%"}
};

static struct entry networktypes[NUM_NETWORKTYPES] = {
    {"static", "Static"},
    {"dynamic", "Dynamic"}
};

static struct entry resolutions[NUM_RESOLUTIONS];
static char resolutionvalues[NUM_RESOLUTIONS][32];

struct rule
{

    char *widget;
    char *palette;
    unsigned int shades;
    unsigned int parts;

};

static struct entry widgets[NUM_WIDGETS] = {
    {"box", "Box"},
    {"button", "Button"},
    {"cycle", "Cycle"},
    {"item", "Item"},
    {"link", "Link"},
    {"mouse", "Mouse"},
    {"panel", "Panel"},
    {"select", "Select"},
    {"swatch", "Swatch"},
    {"tab", "Tab"},
    {"text", "Text"},
    {"window", "Window"},
    {"scrollbar", "Scrollbar (shared with button)"},
    {"cursor", "Cursor (shared with text)"}
};

static struct rule rules[NUM_RULES] = {
    {"box", "box", SHADE_ALL, PART_FRAME},
    {"button", "button", SHADE_ALL, PART_FRAME | PART_TEXT},
    {"cycle", "cycle", SHADE_ALL, PART_FRAME | PART_TEXT | PART_ICON},
    {"item", "item", SHADE_ALL, PART_FRAME},
    {"link", "link", SHADE_NORMAL, PART_TEXT},
    {"mouse", "mouse", SHADE_NORMAL, PART_SHADOW | PART_BODY | PART_LIGHT},
    {"panel", "panel", SHADE_NORMAL, PART_FRAME},
    {"select", "select", SHADE_ALL, PART_FRAME | PART_TEXT | PART_ICON},
    {"swatch", "swatch", SHADE_NORMAL, PART_FRAME},
    {"tab", "tab", SHADE_ALL, PART_FRAME},
    {"tab", "tabchecked", SHADE_ALL, PART_FRAME},
    {"text", "text", SHADE_NORMAL, PART_TEXT},
    {"window", "window", SHADE_NORMAL, PART_FRAME | PART_TEXT | PART_ICON},
    {"window", "window", SHADE_HOVER, PART_ICON},
    {"scrollbar", "button", SHADE_NORMAL, PART_FRAME},
    {"cursor", "text", SHADE_NORMAL, PART_TEXT}
};

struct binding
{

    char key[32];
    char mod[32];
    char action[32];
    char command[128];

};

struct video
{

    char device[DEVICENAME];
    unsigned int configured;
    unsigned int enabled;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;

};

struct device
{

    char name[DEVICENAME];
    unsigned int enabled;
    char map[256];
    char speed[16];
    char scroll[16];
    char invert[8];
    char doubletap[16];
    char bottomleft[8];
    char bottommiddle[8];
    char bottomright[8];
    char volume[16];
    char address[32];
    char route[32];
    char netmask[32];
    char dns[32];
    char type[16];

};

struct devices
{

    struct device list[NUM_DEVICES];
    unsigned int count;
    unsigned int selected;

};

struct color
{

    char name[32];
    char value[16];

};

static struct
{

    char layout[32];
    char background[256];
    char language[32];
    char timezone[32];
    struct video video[NUM_VIDEOS];
    unsigned int nvideos;
    unsigned int selectedvideo;
    struct devices keyboard;
    struct devices mouse;
    struct devices touch;
    struct devices audio;
    struct devices network;
    char transparency[8];
    char cursor[8];
    char bindingpath[256];
    char themepath[256];
    struct binding binding[NUM_BINDINGS];
    unsigned int nbindings;
    struct color colors[NUM_COLORS];
    unsigned int ncolors;

} config;

static char *shadenames[NUM_SHADES] = {"normal", "hover", "focus", "disabled"};
static char *partnames[NUM_PARTS] = {"shadow", "body", "dark", "light", "text", "icon"};
static char colorwidget[32] = "box";
static char out[MESSAGE_SIZE];
static unsigned int outcount;
static unsigned int wm;
static char selecting[32];

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
        channel_send(0, wm, MESSAGE_TYPE_WMRENDERDATA, outcount, out);

    outcount = 0;

}

static void reserve(void)
{

    if (outcount + 1024 > MESSAGE_SIZE)
        flush();

}

static unsigned int query(char *data, unsigned int size, char *file, char *path, char *format)
{

    if (!cstring_length(file))
        return 0;

    return (format) ? system_feed(0, 0, data, size, "mq -query %s -format \"%s\" %s", path, format, file) : system_feed(0, 0, data, size, "mq -query %s %s", path, file);

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

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "= area label \"%s\"\n- content\n+ box id \"content\" in \"main\" flow \"vertical-stretch\" padding \"2\" overflow \"scroll\" span \"1\"\n", label);

}

static void beginrow(char *id)
{

    reserve();

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ layout id \"%s-row\" in \"content\" flow \"horizontal-stretch\" spacing \"2\" padding \"1\"\n", id);

}

static void addlabel(char *id, char *label)
{

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ panel id \"%s-label\" in \"%s-row\" flow \"stretch\" padding \"3\" span \"1\"\n+ text in \"%s-label\" valign \"middle\" label \"%s\"\n+ layout id \"%s-value\" in \"%s-row\" flow \"horizontal-stretch\" spacing \"2\" span \"2\"\n", id, id, id, label, id, id);

}

static void addtextbox(char *parent, char *part, char *id, char *value, unsigned int span, char *halign)
{

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ box id \"%s\" in \"%s-%s\" mode \"edit\" padding \"3\" flow \"stretch\" span \"%u\" onchange \"q=change&id=%s&value=\"\n+ text in \"%s\" halign \"%s\" label \"%s\"\n", id, parent, part, &span, id, id, halign, value);

}

static void pathfield(char *id, char *label, char *value)
{

    beginrow(id);
    addlabel(id, label);

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ box id \"%s-box\" in \"%s-value\" mode \"edit\" padding \"3\" flow \"stretch\" span \"1\" onchange \"q=change&id=%s&value=\"\n+ text id \"%s\" in \"%s-box\" label \"%s\"\n+ button in \"%s-value\" label \"Select\" onclick \"q=open&id=%s\"\n", id, id, id, id, id, value, id, id);

}

static void colorfield(unsigned int index, char *label, char *value)
{

    char id[16];

    cstring_write_fmt(id, 16, 0, "c%u\\0", &index);
    beginrow(id);
    addlabel(id, label);

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ swatch id \"s%u\" in \"c%u-value\" color \"%s\"\n+ box id \"t%u\" in \"c%u-value\" mode \"edit\" padding \"3\" onchange \"q=color&index=%u&value=\"\n+ text in \"t%u\" label \"%s\"\n", &index, &index, value, &index, &index, &index, &index, value);

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

static void addcycle(char *id, char *value, struct entry *options, unsigned int count)
{

    unsigned int i;

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "@- %s\n+ cycle id \"%s\" in \"%s-value\" span \"1\" data \"%s\" onclick \"q=choose&id=%s&value={key}&label={label}\" label \"%s\"\n", id, id, id, id, id, getlabel(options, count, value));

    for (i = 0; i < count; i++)
    {

        reserve();

        outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "@ %s \"%s\" label \"%s\"\n", id, options[i].value, options[i].label);

    }

}

static void choice(char *id, char *label, char *value, struct entry *options, unsigned int count)
{

    beginrow(id);
    addlabel(id, label);
    addcycle(id, value, options, count);

}

static unsigned int getnumber(char *data, unsigned int count, unsigned int index)
{

    char word[16];

    getword(data, count, index, word, 16);

    return cstring_read_value(word, cstring_length(word), 10);

}

static void loadvideo(char *line, unsigned int length)
{

    if (config.nvideos < NUM_VIDEOS)
    {

        struct video *video = &config.video[config.nvideos];
        char enabled[8];

        video->width = getnumber(line, length, 0);
        video->height = getnumber(line, length, 1);
        video->bpp = getnumber(line, length, 2);
        video->configured = 1;
        video->enabled = !cstring_match(getword(line, length, 4, enabled, 8), "off");

        if (!cstring_length(getword(line, length, 3, video->device, DEVICENAME)))
            cstring_write_fmt(video->device, DEVICENAME, 0, "video:%u\\0", &config.nvideos);

        config.nvideos++;

    }

}

static unsigned int finddevice(struct devices *devices, char *name)
{

    unsigned int i;

    for (i = 0; i < devices->count; i++)
    {

        if (cstring_match(devices->list[i].name, name))
            return i;

    }

    return devices->count;

}

static struct device *adddevice(struct devices *devices, char *name, unsigned int enabled)
{

    if (devices->count < NUM_DEVICES && cstring_length(name) && finddevice(devices, name) == devices->count)
    {

        struct device *device = &devices->list[devices->count];

        copystring(device->name, DEVICENAME, name, cstring_length(name));
        copystring(device->map, 256, keymaps[0].value, cstring_length(keymaps[0].value));
        copystring(device->speed, 16, "4", 1);
        copystring(device->scroll, 16, "4", 1);
        copystring(device->invert, 8, inverts[0].value, cstring_length(inverts[0].value));
        copystring(device->doubletap, 16, doubletaps[0].value, cstring_length(doubletaps[0].value));
        copystring(device->bottomleft, 8, areas[0].value, cstring_length(areas[0].value));
        copystring(device->bottommiddle, 8, areas[1].value, cstring_length(areas[1].value));
        copystring(device->bottomright, 8, areas[2].value, cstring_length(areas[2].value));
        copystring(device->volume, 16, "100", 3);
        copystring(device->type, 16, networktypes[0].value, cstring_length(networktypes[0].value));

        device->enabled = enabled;
        devices->count++;

        return device;

    }

    return 0;

}

static struct device *getselected(struct devices *devices)
{

    return (devices->selected < devices->count) ? &devices->list[devices->selected] : 0;

}

static char *getfield(char *data, unsigned int count, unsigned int index, char *field, unsigned int size)
{

    unsigned int start = 0;
    unsigned int i;

    field[0] = '\0';

    for (i = 0; i <= count; i++)
    {

        if (i == count || data[i] == '|')
        {

            if (!index--)
            {

                copystring(field, size, data + start, i - start);

                break;

            }

            start = i + 1;

        }

    }

    return field;

}

static void loadkeyboard(char *line, unsigned int length)
{

    char name[DEVICENAME];
    char map[256];
    char enabled[8];
    struct device *device = adddevice(&config.keyboard, getfield(line, length, 0, name, DEVICENAME), !cstring_match(getfield(line, length, 2, enabled, 8), "off"));

    if (device && cstring_length(getfield(line, length, 1, map, 256)))
        copystring(device->map, 256, map, cstring_length(map));

}

static void loadscroll(struct device *device, char *line, unsigned int length)
{

    char scroll[16];
    char invert[8];

    if (cstring_length(getfield(line, length, 3, scroll, 16)))
        copystring(device->scroll, 16, scroll, cstring_length(scroll));

    if (cstring_length(getfield(line, length, 4, invert, 8)))
        copystring(device->invert, 8, invert, cstring_length(invert));

}

static void loadmouse(char *line, unsigned int length)
{

    char name[DEVICENAME];
    char speed[16];
    char enabled[8];
    struct device *device = adddevice(&config.mouse, getfield(line, length, 0, name, DEVICENAME), !cstring_match(getfield(line, length, 2, enabled, 8), "off"));

    if (device && cstring_length(getfield(line, length, 1, speed, 16)))
        copystring(device->speed, 16, speed, cstring_length(speed));

    if (device)
        loadscroll(device, line, length);

}

static void loadaudio(char *line, unsigned int length)
{

    char name[DEVICENAME];
    char volume[16];
    char enabled[8];
    struct device *device = adddevice(&config.audio, getfield(line, length, 0, name, DEVICENAME), !cstring_match(getfield(line, length, 2, enabled, 8), "off"));

    if (device && cstring_length(getfield(line, length, 1, volume, 16)))
        copystring(device->volume, 16, volume, cstring_length(volume));

}

static void loadnetwork(char *line, unsigned int length)
{

    char name[DEVICENAME];
    char type[16];
    struct device *device = adddevice(&config.network, getfield(line, length, 0, name, DEVICENAME), 1);

    if (device)
    {

        if (cstring_length(getfield(line, length, 1, type, 16)))
            copystring(device->type, 16, type, cstring_length(type));

        getfield(line, length, 2, device->address, 32);
        getfield(line, length, 3, device->route, 32);
        getfield(line, length, 4, device->netmask, 32);
        getfield(line, length, 5, device->dns, 32);

    }

}

static void loadarea(char *area, char *line, unsigned int length, unsigned int index)
{

    char value[8];

    if (cstring_length(getfield(line, length, index, value, 8)))
        copystring(area, 8, value, cstring_length(value));

}

static void loadtouch(char *line, unsigned int length)
{

    char name[DEVICENAME];
    char doubletap[16];
    char speed[16];
    char enabled[8];
    struct device *device = adddevice(&config.touch, getfield(line, length, 0, name, DEVICENAME), !cstring_match(getfield(line, length, 2, enabled, 8), "off"));

    if (device && cstring_length(getfield(line, length, 1, doubletap, 16)))
        copystring(device->doubletap, 16, doubletap, cstring_length(doubletap));

    if (device)
    {

        loadscroll(device, line, length);
        loadarea(device->bottomleft, line, length, 5);
        loadarea(device->bottommiddle, line, length, 6);
        loadarea(device->bottomright, line, length, 7);

        if (cstring_length(getfield(line, length, 8, speed, 16)))
            copystring(device->speed, 16, speed, cstring_length(speed));

    }

}

static void finddevices(struct devices *devices, char *kind)
{

    unsigned int i;

    for (i = 0; i < NUM_DEVICES; i++)
    {

        char name[DEVICENAME];

        cstring_write_fmt(name, DEVICENAME, 0, "%s:%u\\0", kind, &i);

        if (!channel_lookup(name))
            break;

        adddevice(devices, name, 1);

    }

}

static unsigned int hasvideo(char *name)
{

    unsigned int i;

    for (i = 0; i < config.nvideos; i++)
    {

        if (cstring_match(config.video[i].device, name))
            return 1;

    }

    return 0;

}

static void findvideos(void)
{

    unsigned int i;

    for (i = 0; i < NUM_VIDEOS && config.nvideos < NUM_VIDEOS; i++)
    {

        char name[DEVICENAME];
        unsigned int channel;

        cstring_write_fmt(name, DEVICENAME, 0, "video:%u\\0", &i);

        channel = channel_lookup(name);

        if (channel && !hasvideo(name))
        {

            struct video *video = &config.video[config.nvideos];
            struct message_videoinfo videoinfo;

            copystring(video->device, DEVICENAME, name, cstring_length(name));

            video->configured = 0;
            video->enabled = channel_send(1, channel, MESSAGE_TYPE_INFO, 0, 0) && channel_wait(1, channel, MESSAGE_TYPE_VIDEOINFO, sizeof (struct message_videoinfo), &videoinfo) && videoinfo.connected;
            video->width = 0;
            video->height = 0;
            video->bpp = 4;

            config.nvideos++;

        }

    }

}

static void loadbinding(char *line, unsigned int length)
{

    struct binding *binding = &config.binding[config.nbindings];

    if (config.nbindings < NUM_BINDINGS)
    {

        unsigned int start;
        unsigned int spaces;

        getword(line, length, 0, binding->key, 32);
        getword(line, length, 1, binding->mod, 32);
        getword(line, length, 2, binding->action, 32);

        for (start = 0, spaces = 0; start < length && spaces < 3; start++)
        {

            if (line[start] == ' ')
                spaces++;

        }

        copystring(binding->command, 128, line + start, (spaces == 3) ? length - start : 0);

        config.nbindings++;

    }

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

static void loadlines(char *file, char *path, char *format, void (*load)(char *line, unsigned int length))
{

    static char data[0x4000];
    unsigned int count = query(data, 0x4000, file, path, format);
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
    unsigned int count;

    count = query(data, 256, option_getstring("config"), ".layout", 0);

    getword(data, count, 0, config.layout, 32);

    count = query(data, 256, option_getstring("config"), ".background", 0);

    getword(data, count, 0, config.background, 256);

    count = query(data, 256, option_getstring("localization-config"), ".language", 0);

    getword(data, count, 0, config.language, 32);

    count = query(data, 256, option_getstring("localization-config"), ".timezone", 0);

    getword(data, count, 0, config.timezone, 32);

    count = query(data, 256, option_getstring("config"), ".transparency", 0);

    getword(data, count, 0, config.transparency, 8);

    if (!cstring_match(config.transparency, "off"))
        copystring(config.transparency, 8, "on", 2);

    count = query(data, 256, option_getstring("config"), ".cursor", 0);

    getword(data, count, 0, config.cursor, 8);

    if (!config.cursor[0])
        copystring(config.cursor, 8, "100", 3);

    count = query(data, 256, option_getstring("config"), ".binding", 0);

    getword(data, count, 0, config.bindingpath, 256);

    count = query(data, 256, option_getstring("config"), ".theme", 0);

    getword(data, count, 0, config.themepath, 256);

    config.nbindings = 0;
    config.ncolors = 0;
    config.nvideos = 0;
    config.selectedvideo = 0;
    config.keyboard.count = 0;
    config.keyboard.selected = 0;
    config.mouse.count = 0;
    config.mouse.selected = 0;
    config.touch.count = 0;
    config.touch.selected = 0;
    config.audio.count = 0;
    config.audio.selected = 0;
    config.network.count = 0;
    config.network.selected = 0;

    loadlines(option_getstring("video-config"), ".devices", "{width} {height} {bpp} {device} {enabled}", loadvideo);
    loadlines(option_getstring("keyboard-config"), ".devices", "{device}|{map}|{enabled}", loadkeyboard);
    loadlines(option_getstring("mouse-config"), ".devices", "{device}|{speed}|{enabled}|{scroll}|{invert}", loadmouse);
    loadlines(option_getstring("touch-config"), ".devices", "{device}|{doubletap}|{enabled}|{scroll}|{invert}|{bottomleft}|{bottommiddle}|{bottomright}|{speed}", loadtouch);
    findvideos();
    finddevices(&config.keyboard, "keyboard");
    finddevices(&config.mouse, "mouse");
    finddevices(&config.touch, "touch");
    loadlines(option_getstring("audio-config"), ".devices", "{device}|{volume}|{enabled}", loadaudio);
    finddevices(&config.audio, "audio");
    loadlines(option_getstring("network-config"), ".networks", "{device}|{type}|{address}|{route}|{netmask}|{dns}", loadnetwork);
    finddevices(&config.network, "ethernet");

    loadlines(config.bindingpath, ".binding", "{key} {mod} {action} {command}", loadbinding);
    loadlines(config.themepath, ".colors", "{name} {value}", loadcolor);

}

static unsigned int writeconfig(char *data, unsigned int size)
{

    return cstring_write_fmt(data, size, 0, "{\n  layout: %s\n  theme: \"%s\"\n  background: \"%s\"\n  transparency: %s\n  cursor: %s\n  binding: \"%s\"\n}\n", config.layout, config.themepath, config.background, config.transparency, config.cursor, config.bindingpath);

}

static unsigned int writevideo(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  devices: [\n");

    for (i = 0; i < config.nvideos; i++)
    {

        struct video *video = &config.video[i];

        if (video->configured)
            count += cstring_write_fmt(data, size, count, "    { device: %s width: %u height: %u bpp: %u enabled: %s }\n", video->device, &video->width, &video->height, &video->bpp, (video->enabled) ? "on" : "off");

    }

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int writekeyboard(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  devices: [\n");

    for (i = 0; i < config.keyboard.count; i++)
        count += cstring_write_fmt(data, size, count, "    { device: %s map: \"%s\" enabled: %s }\n", config.keyboard.list[i].name, config.keyboard.list[i].map, (config.keyboard.list[i].enabled) ? "on" : "off");

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int writemouse(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  devices: [\n");

    for (i = 0; i < config.mouse.count; i++)
        count += cstring_write_fmt(data, size, count, "    { device: %s speed: %s enabled: %s scroll: %s invert: %s }\n", config.mouse.list[i].name, config.mouse.list[i].speed, (config.mouse.list[i].enabled) ? "on" : "off", config.mouse.list[i].scroll, config.mouse.list[i].invert);

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int writetouch(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  devices: [\n");

    for (i = 0; i < config.touch.count; i++)
        count += cstring_write_fmt(data, size, count, "    { device: %s doubletap: %s enabled: %s scroll: %s invert: %s bottomleft: %s bottommiddle: %s bottomright: %s speed: %s }\n", config.touch.list[i].name, config.touch.list[i].doubletap, (config.touch.list[i].enabled) ? "on" : "off", config.touch.list[i].scroll, config.touch.list[i].invert, config.touch.list[i].bottomleft, config.touch.list[i].bottommiddle, config.touch.list[i].bottomright, config.touch.list[i].speed);

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int writeaudio(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  devices: [\n");

    for (i = 0; i < config.audio.count; i++)
        count += cstring_write_fmt(data, size, count, "    { device: %s volume: %s enabled: %s }\n", config.audio.list[i].name, config.audio.list[i].volume, (config.audio.list[i].enabled) ? "on" : "off");

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int writelocalization(char *data, unsigned int size)
{

    return cstring_write_fmt(data, size, 0, "{\n  language: %s\n  timezone: %s\n}\n", config.language, config.timezone);

}

static unsigned int writenetwork(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  networks: [\n");

    for (i = 0; i < config.network.count; i++)
    {

        struct device *device = &config.network.list[i];

        if (cstring_match(device->type, "dynamic"))
            count += cstring_write_fmt(data, size, count, "    {\n      device: %s\n      type: %s\n    }\n", device->name, device->type);
        else if (cstring_length(device->address))
            count += cstring_write_fmt(data, size, count, "    {\n      device: %s\n      type: %s\n      address: %s\n      route: %s\n      netmask: %s\n      dns: %s\n    }\n", device->name, device->type, device->address, device->route, device->netmask, device->dns);

    }

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int writeaudiolines(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "devices\n");

    for (i = 0; i < config.audio.count; i++)
        count += cstring_write_fmt(data, size, count, "device %s %s %s\n", config.audio.list[i].name, config.audio.list[i].volume, (config.audio.list[i].enabled) ? "on" : "off");

    return count;

}

static unsigned int writebindings(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  binding:\n  [\n");

    for (i = 0; i < config.nbindings; i++)
    {

        struct binding *binding = &config.binding[i];

        if (!cstring_length(binding->key) || !cstring_length(binding->action))
            continue;

        if (cstring_length(binding->command))
            count += cstring_write_fmt(data, size, count, "    { key: %s mod: %s action: %s command: \"%s\" }\n", binding->key, (cstring_length(binding->mod)) ? binding->mod : "none", binding->action, binding->command);
        else
            count += cstring_write_fmt(data, size, count, "    { key: %s mod: %s action: %s }\n", binding->key, (cstring_length(binding->mod)) ? binding->mod : "none", binding->action);

    }

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int writetheme(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "{\n  colors:\n  [\n");

    for (i = 0; i < config.ncolors; i++)
    {

        struct color *color = &config.colors[i];
        unsigned int pad = (cstring_length(color->name) < 25) ? 25 - cstring_length(color->name) : 0;

        count += cstring_write_fmt(data, size, count, "    { name: %s ", color->name);

        while (pad-- && count < size)
            data[count++] = ' ';

        count += cstring_write_fmt(data, size, count, "value: %s }\n", color->value);

    }

    count += cstring_write_fmt(data, size, count, "  ]\n}\n");

    return count;

}

static unsigned int writelines(char *data, unsigned int size)
{

    unsigned int count = 0;
    unsigned int i;

    count += cstring_write_fmt(data, size, count, "layout %s\nbackground %s\n", config.layout, config.background);
    count += cstring_write_fmt(data, size, count, "transparency %s\ncursor %s\nvideos\n", config.transparency, config.cursor);

    for (i = 0; i < config.nvideos; i++)
    {

        struct video *video = &config.video[i];

        if (video->configured)
            count += cstring_write_fmt(data, size, count, "video %u %u %u %s %s\n", &video->width, &video->height, &video->bpp, video->device, (video->enabled) ? "on" : "off");

    }

    count += cstring_write_fmt(data, size, count, "inputs\n");

    for (i = 0; i < config.keyboard.count; i++)
        count += cstring_write_fmt(data, size, count, "keyboard %s %s %s\n", config.keyboard.list[i].name, config.keyboard.list[i].map, (config.keyboard.list[i].enabled) ? "on" : "off");

    for (i = 0; i < config.mouse.count; i++)
        count += cstring_write_fmt(data, size, count, "mouse %s %s %s %s %s\n", config.mouse.list[i].name, config.mouse.list[i].speed, (config.mouse.list[i].enabled) ? "on" : "off", config.mouse.list[i].scroll, config.mouse.list[i].invert);

    for (i = 0; i < config.touch.count; i++)
        count += cstring_write_fmt(data, size, count, "touch %s %s %s %s %s %s %s %s %s\n", config.touch.list[i].name, config.touch.list[i].doubletap, (config.touch.list[i].enabled) ? "on" : "off", config.touch.list[i].scroll, config.touch.list[i].invert, config.touch.list[i].bottomleft, config.touch.list[i].bottommiddle, config.touch.list[i].bottomright, config.touch.list[i].speed);

    for (i = 0; i < config.ncolors; i++)
        count += cstring_write_fmt(data, size, count, "color %s %s\n", config.colors[i].name, config.colors[i].value);

    count += cstring_write_fmt(data, size, count, "bindings\n");

    for (i = 0; i < config.nbindings; i++)
    {

        struct binding *binding = &config.binding[i];

        if (cstring_length(binding->key) && cstring_length(binding->action))
            count += cstring_write_fmt(data, size, count, "binding %s %s %s %s\n", binding->key, (cstring_length(binding->mod)) ? binding->mod : "none", binding->action, binding->command);

    }

    return count;

}

static void sendlines(unsigned int target, unsigned int event, char *data, unsigned int count)
{

    unsigned int offset = 0;

    while (offset < count)
    {

        unsigned int length = count - offset;

        if (length > MESSAGE_SIZE)
        {

            length = MESSAGE_SIZE;

            while (length && data[offset + length - 1] != '\n')
                length--;

            if (!length)
                break;

        }

        channel_send(0, target, event, length, data + offset);

        offset += length;

    }

}

static unsigned int savefile(char *path, char *data, unsigned int count)
{

    unsigned int target = fs_auth(path);
    unsigned int parent = (target) ? fs_walkparent(1, target, path) : 0;
    unsigned int id = (parent) ? fs_walk(1, target, 0, path) : 0;

    if (parent && (!id || fs_remove(1, target, parent, id)))
    {

        char *name = path + fs_dirlength(path);

        id = fs_create(1, target, parent, name, cstring_length(name));

        return id && fs_write_all(1, target, id, data, count, 0) == count;

    }

    return 0;

}

static void setresolution(struct video *video, char *value)
{

    unsigned int width = cstring_read_value(value, buffer_findbyte(value, cstring_length(value), 'x'), 10);
    char *height = value + buffer_findbyte(value, cstring_length(value), 'x') + 1;
    char *bits = height + buffer_findbyte(height, cstring_length(height), 'x') + 1;

    video->width = width;
    video->height = cstring_read_value(height, buffer_findbyte(height, cstring_length(height), 'x'), 10);
    video->bpp = cstring_read_value(bits, cstring_length(bits), 10) / 8;
    video->configured = 1;

}

static void setchoice(char *id, char *value)
{

    if (cstring_match(id, "general-layout"))
        copystring(config.layout, 32, value, cstring_length(value));
    else if (cstring_match(id, "localization-language"))
        copystring(config.language, 32, value, cstring_length(value));
    else if (cstring_match(id, "localization-timezone"))
        copystring(config.timezone, 32, value, cstring_length(value));
    else if (cstring_match(id, "keyboard-layout") && getselected(&config.keyboard))
        copystring(getselected(&config.keyboard)->map, 256, value, cstring_length(value));
    else if (cstring_match(id, "network-type") && getselected(&config.network))
        copystring(getselected(&config.network)->type, 16, value, cstring_length(value));
    else if (cstring_match(id, "touch-doubletap") && getselected(&config.touch))
        copystring(getselected(&config.touch)->doubletap, 16, value, cstring_length(value));
    else if (cstring_match(id, "video-resolution") && config.selectedvideo < config.nvideos)
        setresolution(&config.video[config.selectedvideo], value);
    else if (cstring_match(id, "general-transparency"))
        copystring(config.transparency, 8, value, cstring_length(value));
    else if (cstring_match(id, "mouse-cursor"))
        copystring(config.cursor, 8, value, cstring_length(value));
    else if (cstring_match(id, "mouse-speed") && getselected(&config.mouse))
        copystring(getselected(&config.mouse)->speed, 16, value, cstring_length(value));
    else if (cstring_match(id, "mouse-scroll") && getselected(&config.mouse))
        copystring(getselected(&config.mouse)->scroll, 16, value, cstring_length(value));
    else if (cstring_match(id, "mouse-invert") && getselected(&config.mouse))
        copystring(getselected(&config.mouse)->invert, 8, value, cstring_length(value));
    else if (cstring_match(id, "touch-scroll") && getselected(&config.touch))
        copystring(getselected(&config.touch)->scroll, 16, value, cstring_length(value));
    else if (cstring_match(id, "touch-invert") && getselected(&config.touch))
        copystring(getselected(&config.touch)->invert, 8, value, cstring_length(value));
    else if (cstring_match(id, "touch-bottomleft") && getselected(&config.touch))
        copystring(getselected(&config.touch)->bottomleft, 8, value, cstring_length(value));
    else if (cstring_match(id, "touch-bottommiddle") && getselected(&config.touch))
        copystring(getselected(&config.touch)->bottommiddle, 8, value, cstring_length(value));
    else if (cstring_match(id, "touch-bottomright") && getselected(&config.touch))
        copystring(getselected(&config.touch)->bottomright, 8, value, cstring_length(value));
    else if (cstring_match(id, "touch-speed") && getselected(&config.touch))
        copystring(getselected(&config.touch)->speed, 16, value, cstring_length(value));
    else if (cstring_match(id, "audio-volume") && getselected(&config.audio))
        copystring(getselected(&config.audio)->volume, 16, value, cstring_length(value));

}

static void setfield(char *id, char *value)
{

    unsigned int length = cstring_length(value);

    if (buffer_match(id, "network-", 8) && getselected(&config.network))
    {

        struct device *device = getselected(&config.network);

        if (cstring_match(id, "network-address"))
            copystring(device->address, 32, value, length);
        else if (cstring_match(id, "network-route"))
            copystring(device->route, 32, value, length);
        else if (cstring_match(id, "network-netmask"))
            copystring(device->netmask, 32, value, length);
        else if (cstring_match(id, "network-dns"))
            copystring(device->dns, 32, value, length);

    }

    else if (cstring_match(id, "general-theme"))
    {

        copystring(config.themepath, 256, value, length);

    }

    else if (cstring_match(id, "general-background"))
    {

        copystring(config.background, 256, value, length);

    }

    else if (buffer_match(id, "binding", 7))
    {

        unsigned int digits = buffer_findbyte(id + 7, cstring_length(id + 7), '-');
        unsigned int index = cstring_read_value(id + 7, digits, 10);
        char *part = id + 7 + digits + 1;

        if (index < config.nbindings)
        {

            struct binding *binding = &config.binding[index];

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
    choice("localization-language", "Language", config.language, languages, NUM_LANGUAGES);
    choice("localization-timezone", "Timezone", config.timezone, timezones, NUM_TIMEZONES);

}

static void rendergeneral(void)
{

    beginarea("General");
    choice("general-layout", "Layout", config.layout, layouts, NUM_LAYOUTS);
    pathfield("general-theme", "Theme", config.themepath);
    pathfield("general-background", "Background", config.background);
    choice("general-transparency", "Transparency", config.transparency, transparencies, NUM_TRANSPARENCIES);

}

static void devicechoice(char *id, char *value, struct entry *options, unsigned int count)
{

    if (count)
        choice(id, "Device", value, options, count);
    else
        choice(id, "Device", "None found", options, 0);

}

static void enabledfield(char *kind, unsigned int enabled)
{

    char id[32];

    cstring_write_fmt(id, 32, 0, "%s-enabled\\0", kind);
    beginrow(id);
    addlabel(id, "Enabled");

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "@ onoff 0 label \"Off\"\n@ onoff 1 label \"On\"\n");
    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ cycle id \"%s\" in \"%s-value\" span \"1\" data \"onoff\" onclick \"q=enable&kind=%s&value={key}\" label \"%s\"\n", id, id, kind, (enabled) ? "On" : "Off");

}

static void renderdevicechoice(struct devices *devices, char *kind)
{

    struct entry options[NUM_DEVICES];
    struct device *device = getselected(devices);
    char id[32];
    unsigned int i;

    for (i = 0; i < devices->count; i++)
    {

        options[i].value = devices->list[i].name;
        options[i].label = devices->list[i].name;

    }

    cstring_write_fmt(id, 32, 0, "%s-device\\0", kind);
    devicechoice(id, (device) ? device->name : "", options, devices->count);

}

static unsigned int loadresolutions(char *device)
{

    struct message_videomodes videomodes;
    unsigned int channel = channel_lookup(device);
    unsigned int count = 0;

    resolutions[count].value = "0x0x32";
    resolutions[count].label = "Preferred";
    count++;

    if (channel && channel_send(1, channel, MESSAGE_TYPE_VIDEOMODEREQUEST, 0, 0) && channel_wait(1, channel, MESSAGE_TYPE_VIDEOMODERESPONSE, sizeof (struct message_videomodes), &videomodes))
    {

        unsigned int i;

        for (i = 0; i < videomodes.count && count < NUM_RESOLUTIONS; i++)
        {

            struct message_videoconf *mode = &videomodes.modes[i];
            unsigned int bits = mode->bpp * 8;

            cstring_write_fmt(resolutionvalues[count], 32, 0, "%ux%ux%u\\0", &mode->width, &mode->height, &bits);

            resolutions[count].value = resolutionvalues[count];
            resolutions[count].label = resolutionvalues[count];
            count++;

        }

    }

    return count;

}

static void rendervideo(void)
{

    struct entry options[NUM_VIDEOS];
    unsigned int i;

    beginarea("Video");

    for (i = 0; i < config.nvideos; i++)
    {

        options[i].value = config.video[i].device;
        options[i].label = config.video[i].device;

    }

    devicechoice("video-device", (config.selectedvideo < config.nvideos) ? config.video[config.selectedvideo].device : "", options, config.nvideos);

    if (config.selectedvideo < config.nvideos)
    {

        struct video *video = &config.video[config.selectedvideo];
        char resolution[64];
        unsigned int bits = video->bpp * 8;

        cstring_write_fmt(resolution, 64, 0, "%ux%ux%u\\0", &video->width, &video->height, &bits);
        enabledfield("video", video->enabled);
        choice("video-resolution", "Resolution", resolution, resolutions, loadresolutions(video->device));

    }

}

static void renderkeyboard(void)
{

    struct device *device = getselected(&config.keyboard);

    beginarea("Keyboard");
    renderdevicechoice(&config.keyboard, "keyboard");

    if (device)
        enabledfield("keyboard", device->enabled);

    if (device)
        choice("keyboard-layout", "Keyboard layout", device->map, keymaps, NUM_KEYMAPS);

}

static void rendermouse(void)
{

    struct device *device = getselected(&config.mouse);

    beginarea("Mouse");
    renderdevicechoice(&config.mouse, "mouse");

    if (device)
    {

        enabledfield("mouse", device->enabled);
        choice("mouse-speed", "Movement speed", device->speed, speeds, NUM_SPEEDS);
        choice("mouse-scroll", "Scroll speed", device->scroll, speeds, NUM_SPEEDS);
        choice("mouse-invert", "Invert scroll", device->invert, inverts, NUM_INVERTS);

    }

    choice("mouse-cursor", "Cursor size", config.cursor, cursors, NUM_CURSORS);

}

static void rendertouch(void)
{

    struct device *device = getselected(&config.touch);

    beginarea("Touch");
    renderdevicechoice(&config.touch, "touch");

    if (device)
    {

        enabledfield("touch", device->enabled);
        choice("touch-doubletap", "Double click", device->doubletap, doubletaps, NUM_DOUBLETAPS);
        choice("touch-speed", "Movement speed", device->speed, speeds, NUM_SPEEDS);
        choice("touch-scroll", "Scroll speed", device->scroll, speeds, NUM_SPEEDS);
        choice("touch-invert", "Invert scroll", device->invert, inverts, NUM_INVERTS);
        choice("touch-bottomleft", "Bottom left", device->bottomleft, areas, NUM_AREAS);
        choice("touch-bottommiddle", "Bottom middle", device->bottommiddle, areas, NUM_AREAS);
        choice("touch-bottomright", "Bottom right", device->bottomright, areas, NUM_AREAS);

    }

}

static void renderaudio(void)
{

    struct device *device = getselected(&config.audio);

    beginarea("Audio");
    renderdevicechoice(&config.audio, "audio");

    if (!device)
        return;

    enabledfield("audio", device->enabled);
    choice("audio-volume", "Volume", device->volume, volumes, NUM_VOLUMES);

}

static void networkfield(char *id, char *label, char *value)
{

    beginrow(id);
    addlabel(id, label);
    addtextbox(id, "value", id, value, 1, "left");

}

static unsigned int writenetworkrows(char *data, unsigned int size, unsigned int offset, char *type)
{

    char *display = (cstring_match(type, "dynamic")) ? "none" : "block";

    return cstring_write_fmt(data, size, offset, "= network-address-row display \"%s\"\n= network-route-row display \"%s\"\n= network-netmask-row display \"%s\"\n= network-dns-row display \"%s\"\n", display, display, display, display);

}

static void rendernetwork(void)
{

    struct device *device = getselected(&config.network);

    beginarea("Network");
    renderdevicechoice(&config.network, "network");

    if (!device)
        return;

    choice("network-type", "Type", device->type, networktypes, NUM_NETWORKTYPES);
    networkfield("network-address", "Address", device->address);
    networkfield("network-route", "Router", device->route);
    networkfield("network-netmask", "Netmask", device->netmask);
    networkfield("network-dns", "DNS", device->dns);

    outcount += writenetworkrows(out, MESSAGE_SIZE, outcount, device->type);

}

static void renderbindings(void)
{

    unsigned int i;

    beginarea("Bindings");
    beginrow("bindings-header");

    outcount += cstring_write_fmt(out, MESSAGE_SIZE, outcount, "+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Key\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Modifier\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Action\"\n+ text in \"bindings-header-row\" weight \"bold\" span \"1\" label \"Command\"\n");

    for (i = 0; i < config.nbindings; i++)
    {

        struct binding *binding = &config.binding[i];
        char row[32];
        char id[64];

        cstring_write_fmt(row, 32, 0, "binding%u\\0", &i);
        beginrow(row);
        cstring_write_fmt(id, 64, 0, "binding%u-key\\0", &i);
        addtextbox(row, "row", id, binding->key, 1, "left");
        cstring_write_fmt(id, 64, 0, "binding%u-mod\\0", &i);
        addtextbox(row, "row", id, binding->mod, 1, "left");
        cstring_write_fmt(id, 64, 0, "binding%u-action\\0", &i);
        addtextbox(row, "row", id, binding->action, 1, "left");
        cstring_write_fmt(id, 64, 0, "binding%u-command\\0", &i);
        addtextbox(row, "row", id, binding->command, 1, "left");

    }

}

static void rendercolor(char *palette, char *shade, char *part)
{

    char name[32];
    unsigned int i;

    cstring_write_fmt(name, 32, 0, "%s-%s-%s\\0", palette, shade, part);

    for (i = 0; i < config.ncolors; i++)
    {

        if (cstring_match(config.colors[i].name, name))
            colorfield(i, config.colors[i].name, config.colors[i].value);

    }

}

static void rendercolors(void)
{

    unsigned int i;

    beginarea("Colors");
    choice("colors-widget", "Widget", colorwidget, widgets, NUM_WIDGETS);

    for (i = 0; i < NUM_RULES; i++)
    {

        struct rule *rule = &rules[i];
        unsigned int shade;
        unsigned int part;

        if (!cstring_match(rule->widget, colorwidget))
            continue;

        for (shade = 0; shade < NUM_SHADES; shade++)
        {

            for (part = 0; part < NUM_PARTS; part++)
            {

                if ((rule->shades & (1 << shade)) && (rule->parts & (1 << part)))
                    rendercolor(rule->palette, shadenames[shade], partnames[part]);

            }

        }

    }

}

static void renderarea(char *name)
{

    if (cstring_match(name, "localization"))
        renderlocalization();
    else if (cstring_match(name, "video"))
        rendervideo();
    else if (cstring_match(name, "keyboard"))
        renderkeyboard();
    else if (cstring_match(name, "mouse"))
        rendermouse();
    else if (cstring_match(name, "touch"))
        rendertouch();
    else if (cstring_match(name, "audio"))
        renderaudio();
    else if (cstring_match(name, "network"))
        rendernetwork();
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

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wsettings.alfi";

        loadconfig();
        channel_send(0, wm, MESSAGE_TYPE_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        renderarea("general");

        channel_hold(0);
        channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);

    }

}

static void onpath(struct message *message)
{

    char *path = message->data;

    if (cstring_match(selecting, "general-theme"))
    {

        copystring(config.themepath, 256, path, cstring_length(path));

        config.ncolors = 0;

        loadlines(config.themepath, ".colors", "{name} {value}", loadcolor);

    }

    else if (cstring_match(selecting, "general-background"))
    {

        copystring(config.background, 256, path, cstring_length(path));

    }

    else
    {

        return;

    }

    channel_send_fmt(0, wm, MESSAGE_TYPE_WMRENDERDATA, "= save mode \"normal\"\n= %s label \"%s\"\n", selecting, path);

}

static struct devices *getdevices(char *kind)
{

    if (cstring_match(kind, "keyboard"))
        return &config.keyboard;

    if (cstring_match(kind, "mouse"))
        return &config.mouse;

    if (cstring_match(kind, "touch"))
        return &config.touch;

    if (cstring_match(kind, "audio"))
        return &config.audio;

    if (cstring_match(kind, "network"))
        return &config.network;

    return 0;

}

static unsigned int selectdevice(char *id, char *name)
{

    unsigned int i;

    if (cstring_match(id, "video-device"))
    {

        for (i = 0; i < config.nvideos; i++)
        {

            if (cstring_match(config.video[i].device, name))
                config.selectedvideo = i;

        }

        renderarea("video");

        return 1;

    }

    if (cstring_match(id, "keyboard-device") || cstring_match(id, "mouse-device") || cstring_match(id, "touch-device") || cstring_match(id, "audio-device") || cstring_match(id, "network-device"))
    {

        char kind[16];
        struct devices *devices;

        copystring(kind, 16, id, buffer_findbyte(id, cstring_length(id), '-'));

        devices = getdevices(kind);

        if (finddevice(devices, name) < devices->count)
            devices->selected = finddevice(devices, name);

        renderarea(kind);

        return 1;

    }

    return 0;

}

static void onwmevent(struct message *message)
{

    struct message_wmevent *event = message_validate(MESSAGE_TYPE_WMEVENT, message->data, message->length);

    if (event)
    {

        if (kv_match(event, "q=area"))
        {

            renderarea(kv_getstring(event, "name="));

        }

        else if (kv_match(event, "q=choose") && cstring_match(kv_getstring(event, "id="), "colors-widget"))
        {

            char *value = kv_getstring(event, "value=");

            copystring(colorwidget, 32, value, cstring_length(value));
            renderarea("colors");

        }

        else if (kv_match(event, "q=choose") && !selectdevice(kv_getstring(event, "id="), kv_getstring(event, "value=")))
        {

            setchoice(kv_getstring(event, "id="), kv_getstring(event, "value="));

            if (cstring_match(kv_getstring(event, "id="), "network-type"))
            {

                char data[256];

                channel_send(0, message->source, MESSAGE_TYPE_WMRENDERDATA, writenetworkrows(data, 256, 0, kv_getstring(event, "value=")), data);

            }

            channel_send_fmt(0, message->source, MESSAGE_TYPE_WMRENDERDATA, "= %s label \"%s\"\n= save mode \"normal\"\n", kv_getstring(event, "id="), kv_getstring(event, "label="));

        }

        else if (kv_match(event, "q=change"))
        {

            char *id = kv_getstring(event, "id=");

            setfield(id, kv_getstring(event, "value="));

            channel_send_fmt(0, message->source, MESSAGE_TYPE_WMRENDERDATA, "= save mode \"normal\"\n");

        }

        else if (kv_match(event, "q=enable"))
        {

            char *kind = kv_getstring(event, "kind=");
            unsigned int enabled = kv_getvalue(event, "value=", 10);
            struct devices *devices = getdevices(kind);

            if (cstring_match(kind, "video") && config.selectedvideo < config.nvideos)
            {

                config.video[config.selectedvideo].enabled = enabled;
                config.video[config.selectedvideo].configured = 1;

            }

            else if (devices && getselected(devices))
                getselected(devices)->enabled = enabled;

            channel_send_fmt(0, message->source, MESSAGE_TYPE_WMRENDERDATA, "= %s-enabled label \"%s\"\n= save mode \"normal\"\n", kind, (enabled) ? "On" : "Off");

        }

        else if (kv_match(event, "q=color"))
        {

            unsigned int index = kv_getvalue(event, "index=", 10);
            char *value = kv_getstring(event, "value=");

            if (index < config.ncolors)
                copystring(config.colors[index].value, 16, value, cstring_length(value));

            channel_send_fmt(0, message->source, MESSAGE_TYPE_WMRENDERDATA, "= s%u color \"%s\"\n= save mode \"normal\"\n", &index, value);

        }

        else if (kv_match(event, "q=save"))
        {

            static char data[0x8000];

            if (savefile(option_getstring("config"), data, writeconfig(data, 0x8000)) && savefile(option_getstring("video-config"), data, writevideo(data, 0x8000)) && savefile(option_getstring("keyboard-config"), data, writekeyboard(data, 0x8000)) && savefile(option_getstring("mouse-config"), data, writemouse(data, 0x8000)) && savefile(option_getstring("touch-config"), data, writetouch(data, 0x8000)) && savefile(config.bindingpath, data, writebindings(data, 0x8000)) && savefile(config.themepath, data, writetheme(data, 0x8000)) && savefile(option_getstring("audio-config"), data, writeaudio(data, 0x8000)) && savefile(option_getstring("localization-config"), data, writelocalization(data, 0x8000)) && savefile(option_getstring("network-config"), data, writenetwork(data, 0x8000)))
                channel_send_fmt(0, message->source, MESSAGE_TYPE_WMRENDERDATA, "= save mode \"disabled\"\n");

        }

        else if (kv_match(event, "q=open"))
        {

            unsigned int target = fs_spawn(1, "initrd:bin/wfile");
            char *id = kv_getstring(event, "id=");

            copystring(selecting, 32, id, cstring_length(id));

            if (target)
            {

                struct message_pipe pipe;

                channel_send_fmt(0, target, MESSAGE_TYPE_OPTION, "mode\\0select\\0wm-service\\0%s\\0", option_getstring("wm-service"));
                message_initpipe(&pipe);
                channel_send(0, target, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
                channel_send(0, target, MESSAGE_TYPE_TERM, 0, 0);

            }

        }

        else if (kv_match(event, "q=apply"))
        {

            static char data[0x8000];
            unsigned int mixer = channel_lookup(option_getstring("mixer-service"));

            sendlines(message->source, MESSAGE_TYPE_CONFIG, data, writelines(data, 0x8000));

            if (mixer)
                sendlines(mixer, MESSAGE_TYPE_CONFIG, data, writeaudiolines(data, 0x8000));

        }

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    option_add("config", "initrd:data/config/wm.mq");
    option_add("video-config", "initrd:data/config/video.mq");
    option_add("keyboard-config", "initrd:data/config/keyboard.mq");
    option_add("mouse-config", "initrd:data/config/mouse.mq");
    option_add("touch-config", "initrd:data/config/touch.mq");
    option_add("audio-config", "initrd:data/config/audio.mq");
    option_add("localization-config", "initrd:data/config/localization.mq");
    option_add("network-config", "initrd:data/config/network.mq");
    option_add("mixer-service", "mixer");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_PATH, onpath);
    channel_bind(MESSAGE_TYPE_WMEVENT, onwmevent);

}

