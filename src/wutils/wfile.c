#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define ROWS                            512
#define CLIPBOARD_SIZE                  0x4000

static char path[256];
static char names[ROWS][RECORD_NAMESIZE + 2];
static unsigned char selected[ROWS];
static unsigned int rows;
static unsigned int listing;
static char clipboard[CLIPBOARD_SIZE];
static char pasted[CLIPBOARD_SIZE];
static unsigned int clipcount;
static unsigned int clipcut;

static void updatepath(unsigned int wm)
{

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= path label \"%s\"\n", path);

}

static unsigned int countselected(void)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < rows; i++)
        count += selected[i];

    return count;

}

static void setbuttons(unsigned int wm)
{

    char *mode = (countselected()) ? "normal" : "disabled";
    char *pastemode = (listing) ? "normal" : "disabled";

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= copy mode \"%s\"\n= cut mode \"%s\"\n= paste mode \"%s\"\n= delete mode \"%s\"\n", mode, mode, pastemode, mode);

}

static void sendcontent(unsigned int wm)
{

    rows = 0;
    listing = 0;

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "- content\n+ box id \"content\" in \"main\" flow \"vertical-stretch\" overflow \"scroll\" span \"1\" onselect \"q=select&count=\"\n");
    setbuttons(wm);

}

static void listdirectory(unsigned int wm, unsigned int target, unsigned int id)
{

    unsigned char data[MESSAGE_SIZE];
    unsigned char d[MESSAGE_SIZE];
    unsigned int count;
    unsigned int offset = 0;
    unsigned int row = 0;
    unsigned int c = 0;

    sendcontent(wm);

    while ((count = fs_read(1, target, id, data, MESSAGE_SIZE, offset)))
    {

        unsigned int i;

        for (i = 0; i < count; i += sizeof (struct record))
        {

            struct record *record = (struct record *)(data + i);
            char name[RECORD_NAMESIZE + 2];

            cstring_write_fmt(name, RECORD_NAMESIZE + 2, 0, "%w%s\\0", record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "");

            if (c + 512 > MESSAGE_SIZE)
            {

                channel_send(0, wm, EVENT_WMRENDERDATA, c, d);

                c = 0;

            }

            if (row < ROWS)
            {

                cstring_write_fmt(names[row], RECORD_NAMESIZE + 2, 0, "%s\\0", name);

                selected[row] = 0;

            }

            c += cstring_write_fmt(d, MESSAGE_SIZE, c, "+ item id \"row%u\" in \"content\" padding \"4\" spacing \"8\" onenter \"q=relpath&path=%s\" onselect \"q=mark&row=%u&selected=\"\n+ text in \"row%u\" valign \"middle\" mode \"link\" onclick \"q=relpath&path=%s\"", &row, name, &row, &row, name);
            c += cstring_write_fmt(d, MESSAGE_SIZE, c, " label \"%s\"\n", name);
            offset = record->offset;
            row++;

        }

    }

    if (c)
        channel_send(0, wm, EVENT_WMRENDERDATA, c, d);

    rows = (row < ROWS) ? row : ROWS;
    listing = 1;

    setbuttons(wm);

}

static unsigned int getline(char *data, unsigned int count, unsigned int index, char *out, unsigned int size)
{

    char *line = buffer_tindex(data, count, '\n', index);
    unsigned int length;

    if (!line || line >= data + count)
        return 0;

    length = buffer_findbyte(line, data + count - line, '\n');

    if (length >= size)
        length = size - 1;

    buffer_write(out, size, line, length, 0);

    out[length] = '\0';

    return 1;

}

static char *getsuffix(void)
{

    unsigned int length = cstring_length(path);
    unsigned int start = buffer_lastbyte(path, length, '/');
    unsigned int dot = buffer_lastbyte(path + start, length - start, '.');

    return (dot) ? path + start + dot - 1 : 0;

}

static void showfile(unsigned int wm, struct record *record)
{

    unsigned int length = cstring_length(path);
    unsigned int start = buffer_lastbyte(path, length, '/');
    char *suffix = getsuffix();
    char labels[1024];
    unsigned int nlabels = (suffix) ? system_feed(0, 0, labels, 1024, "mq -query \".filetypes[suffix:%s].label\" %s", suffix, option_getstring("config")) : 0;
    char label[64];
    unsigned int i;

    if (!start)
        start = buffer_firstbyte(path, length, ':');

    sendcontent(wm);
    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ layout id \"info\" in \"content\" flow \"vertical\" padding \"8\" spacing \"4\"\n");
    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" weight \"bold\" label \"%s\"\n", path + start);
    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Path: %s\"\n", path);
    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Type: File\"\n");
    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Size: %u bytes\"\n", &record->size);
    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Id: %u\"\n", &record->id);
    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ layout id \"actions\" in \"info\" flow \"horizontal\" spacing \"8\"\n");

    for (i = 0; getline(labels, nlabels, i, label, 64); i++)
        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ button in \"actions\" label \"%s\" onclick \"q=action&index=%u\"\n", label, &i);

}

static void updatecontent(unsigned int wm)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;
    struct record record;

    if (!id)
    {

        sendcontent(wm);
        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ layout id \"info\" in \"content\" flow \"vertical\" padding \"8\" spacing \"4\"\n");
        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ text in \"info\" label \"Path not found\"\n");

    }

    else if (fs_stat(1, target, id, &record) && record.type != RECORD_TYPE_DIRECTORY)
    {

        showfile(wm, &record);

    }

    else
    {

        unsigned int length = cstring_length(path);

        if (length && path[length - 1] != '/' && path[length - 1] != ':')
            cstring_write_fmt(path, 256, length, "/\\0");

        listdirectory(wm, target, id);

    }

}

static void changepath(unsigned int wm, char *relative)
{

    char full[256];

    fs_absolute(full, 256, path, relative);
    cstring_write_fmt(path, 256, 0, "%s\\0", full);
    updatecontent(wm);
    updatepath(wm);

}

static void writeclipboard(unsigned int service, char *data, unsigned int count)
{

    unsigned int offset;

    channel_send(1, service, EVENT_CLIPBOARDWRITE, 0, 0);

    for (offset = 0; offset < count; offset += MESSAGE_SIZE)
        channel_send(1, service, EVENT_CLIPBOARDWRITE, (count - offset < MESSAGE_SIZE) ? count - offset : MESSAGE_SIZE, data + offset);

}

static void clip(unsigned int cut)
{

    unsigned int service = channel_lookup(option_getstring("clipboard-service"));
    unsigned int i;

    clipcount = 0;
    clipcut = cut;

    for (i = 0; i < rows; i++)
    {

        if (!selected[i])
            continue;

        if (clipcount + cstring_length(path) + cstring_length(names[i]) + 1 > CLIPBOARD_SIZE)
            break;

        clipcount += cstring_write_fmt(clipboard, CLIPBOARD_SIZE, clipcount, "%s%s\n", path, names[i]);

    }

    if (service)
        writeclipboard(service, clipboard, clipcount);

}

static void paste(void)
{

    unsigned int service = channel_lookup(option_getstring("clipboard-service"));
    unsigned int count = 0;
    unsigned int cut;
    unsigned int start;
    unsigned int i;
    struct message message;

    if (!service)
        return;

    channel_send(1, service, EVENT_CLIPBOARDREAD, 0, 0);

    while (channel_poll(1, service, EVENT_CLIPBOARDDATA, &message) && message.length)
        count += buffer_write(pasted, CLIPBOARD_SIZE - 1, message.data, message.length, count);

    cut = clipcut && count == clipcount && buffer_match(pasted, clipboard, count);
    pasted[count] = '\0';

    for (start = 0, i = 0; i <= count; i++)
    {

        if (i < count && pasted[i] != '\n')
            continue;

        pasted[i] = '\0';

        if (i > start && buffer_findbyte(pasted + start, i - start, '"') == i - start)
            system_run(0, "%s \"%s\" \"%s\"", (cut) ? "mv" : "cp", pasted + start, path);

        start = i + 1;

    }

    if (cut)
    {

        clipcut = 0;
        clipcount = 0;

        writeclipboard(service, 0, 0);

    }

}

static void delete(void)
{

    unsigned int i;

    for (i = 0; i < rows; i++)
    {

        if (selected[i])
            system_run(0, "rm \"%s%s\"", path, names[i]);

    }

}

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wfile.alfi";

        channel_send(0, wm, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        changepath(wm, "initrd:");
        channel_hold(0);
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwmevent(struct message *message)
{

    struct event_wmevent *event = message->data;

    if (kv_match(event, "q=copy"))
    {

        clip(0);
        setbuttons(message->source);

    }

    else if (kv_match(event, "q=cut"))
    {

        clip(1);
        setbuttons(message->source);

    }

    else if (kv_match(event, "q=paste"))
    {

        paste();
        updatecontent(message->source);

    }

    else if (kv_match(event, "q=delete"))
    {

        delete();
        updatecontent(message->source);

    }

    else if (kv_match(event, "q=mark"))
    {

        unsigned int row = kv_getvalue(event, "row=", 10);

        if (row < rows)
            selected[row] = kv_getvalue(event, "selected=", 10);

    }

    else if (kv_match(event, "q=select"))
    {

        setbuttons(message->source);

    }

    else if (kv_match(event, "q=up"))
    {

        char parent[256];

        cstring_write_fmt(parent, 256, 0, "%s/../\\0", path);
        changepath(message->source, parent);

    }

    else if (kv_match(event, "q=action"))
    {

        unsigned int index = kv_getvalue(event, "index=", 10);
        char *suffix = getsuffix();
        char commands[64];
        char name[64];
        unsigned int ncommands = (suffix) ? system_feed(0, 0, commands, 64, "mq -query \".filetypes[suffix:%s][%u].command\" %s", suffix, &index, option_getstring("config")) : 0;

        if (getline(commands, ncommands, 0, name, 64))
            system_run(0, "%s \"%s\" &", name, path);

    }

    else if (kv_match(event, "q=abspath") || kv_match(event, "q=relpath"))
    {

        changepath(message->source, kv_getstring(event, "path="));

    }

}

static void onwmkeypress(struct message *message)
{

    struct event_wmkeypress *wmkeypress = message->data;

    if (!(wmkeypress->keymod & KEYS_MOD_CTRL))
        return;

    switch (wmkeypress->id)
    {

    case KEYS_KEY_C:
    case KEYS_KEY_X:
        if (countselected())
        {

            clip(wmkeypress->id == KEYS_KEY_X);
            setbuttons(message->source);

        }

        break;

    case KEYS_KEY_V:
        if (listing)
        {

            paste();
            updatecontent(message->source);

        }

        break;

    }

}

void init(void)
{

    option_add("config", "initrd:data/config/filetypes.mq");
    option_add("wm-service", "wm");
    option_add("clipboard-service", "clipboard");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

