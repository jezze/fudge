#include <fudge.h>
#include <abi.h>

static char path[256];

static void updatepath(void)
{

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= path content \"%s\"\n", path);

}

static void updatecontent(void)
{

    struct record records[4];
    unsigned int nrecords;
    unsigned int maxsend = 50;

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "- content\n+ listbox id \"content\" in \"main\" mode \"readonly\" overflow \"vscroll\" span \"1\"\n");
    call_walk_absolute(FILE_PW, path);
    call_walk_duplicate(FILE_L0, FILE_PW);

    while ((nrecords = call_list(FILE_PW, FILE_L0, 4, records)))
    {

        char message[MESSAGE_SIZE];
        unsigned int count = 0;
        unsigned int i;

        for (i = 0; i < nrecords; i++)
        {

            struct record *record = &records[i];

            count += cstring_write_fmt6(message, MESSAGE_SIZE, "+ textbutton in \"content\" label \"%w%s\" onclick \"file %w%s\"\n", count, record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "", record->name, &record->length, record->type == RECORD_TYPE_DIRECTORY ? "/" : "");

            if (!--maxsend)
                return;

        }

        channel_send_buffer(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, count, message);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!call_walk_absolute(FILE_L0, "system:service/wm"))
        PANIC();

    call_notify(FILE_L0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(CHANNEL_DEFAULT, EVENT_WMUNMAP);
    channel_close();

}

static void onwmevent(unsigned int source, void *mdata, unsigned int msize)
{

    char *data = (char *)mdata + sizeof (struct event_wmevent);

    if (cstring_match_substring(data, "action "))
    {

        if (cstring_match(data + 7, "copy"))
        {

        }

        else if (cstring_match(data + 7, "cut"))
        {

        }

        else if (cstring_match(data + 7, "paste"))
        {

        }

        else if (cstring_match(data + 7, "delete"))
        {

        }

    }

    else if (cstring_match_substring(data, "up"))
    {

        if (cstring_length(path))
        {

            unsigned int l = cstring_length(path);
            unsigned int p;

            if (path[l - 1] == '/')
                l--;

            p = buffer_lastbyte(path, l, '/');

            if (!p)
                p = buffer_firstbyte(path, l, ':');

            if (p)
                path[p] = 0;

        }

        updatepath();
        updatecontent();

    }

    else if (cstring_match_substring(data, "volume "))
    {

        cstring_write_fmt1(path, 256, "%s:\\0", 0, data + 7);
        updatepath();
        updatecontent();

    }

    else if (cstring_match_substring(data, "file "))
    {

        cstring_write_fmt2(path, 256, "%s%s\\0", 0, path, data + 5);
        updatepath();
        updatecontent();

    }

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"File Manager\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical\"\n"
        "    + layout id \"top\" in \"base\" flow \"horizontal\" padding \"1\"\n"
        "      + select id \"volume\" in \"top\" label \"Volume\"\n"
        "        + layout id \"volume-list\" in \"volume\" flow \"vertical\"\n"
        "          + choice in \"volume-list\" label \"initrd:\" onclick \"volume initrd\"\n"
        "          + choice in \"volume-list\" label \"system:\" onclick \"volume system\"\n"
        "      + textbox id \"pathbox\" in \"top\" span \"1\"\n"
        "        + text id \"path\" in \"pathbox\"\n"
        "      + button in \"top\" label \"Up\" onclick \"up\"\n"
        "    + layout id \"main\" in \"base\" flow \"vertical\" padding \"1\" span \"1\"\n"
        "      + listbox id \"content\" in \"main\" mode \"readonly\" overflow \"vscroll\" span \"1\"\n"
        "    + layout id \"bottom\" in \"base\" flow \"horizontal\" padding \"1\"\n"
        "      + button in \"bottom\" label \"Copy\" onclick \"action copy\"\n"
        "      + button in \"bottom\" label \"Cut\" onclick \"action cut\"\n"
        "      + button in \"bottom\" label \"Paste\" onclick \"action paste\"\n"
        "      + button in \"bottom\" label \"Delete\" onclick \"action delete\"\n";

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data);
    cstring_write_fmt0(path, 256, "initrd:\\0", 0);
    updatepath();
    updatecontent();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

