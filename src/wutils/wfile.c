#include <fudge.h>
#include <abi.h>

static char path[256];

static void updatepath(void)
{

    channel_send_fmt0(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "= path cursor \"2\"\n");
    channel_send_fmt1(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "= path content \"%s\"\n", path);

}

static void updatecontent(void)
{

    struct record records[4];
    unsigned int nrecords;
    unsigned int maxsend = 50;

    channel_send_fmt0(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "- content\n+ listbox id \"content\" in \"main\" mode \"readonly\" overflow \"vscroll\" span \"1\"\n");
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

        channel_send_buffer(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, count, message);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(option_getdecimal("wm-service"), EVENT_WMMAP);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onwmevent(unsigned int source, void *mdata, unsigned int msize)
{

    char *data = (char *)mdata + sizeof (struct event_wmevent);

    if (cstring_match_word(data, 0, "action"))
    {

        if (cstring_match_word(data, 1, "copy"))
        {

        }

        else if (cstring_match_word(data, 1, "cut"))
        {

        }

        else if (cstring_match_word(data, 1, "paste"))
        {

        }

        else if (cstring_match_word(data, 1, "delete"))
        {

        }

    }

    else if (cstring_match_word(data, 0, "up"))
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

    else if (cstring_match_word(data, 0, "volume"))
    {

        cstring_write_fmt1(path, 256, "%s:\\0", 0, cstring_get_word(data, 1));
        updatepath();
        updatecontent();

    }

    else if (cstring_match_word(data, 0, "file"))
    {

        cstring_write_fmt2(path, 256, "%s%s\\0", 0, path, cstring_get_word(data, 1));
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
        "        + textedit id \"path\" in \"pathbox\"\n"
        "      + button in \"top\" label \"Up\" onclick \"up\"\n"
        "    + layout id \"main\" in \"base\" flow \"horizontal-stretch\" padding \"1\" span \"1\"\n"
        "      + listbox id \"content\" in \"main\" mode \"readonly\" overflow \"vscroll\" span \"1\"\n"
        "    + layout id \"bottom\" in \"base\" flow \"horizontal\" padding \"1\"\n"
        "      + button in \"bottom\" label \"Copy\" onclick \"action copy\"\n"
        "      + button in \"bottom\" label \"Cut\" onclick \"action cut\"\n"
        "      + button in \"bottom\" label \"Paste\" onclick \"action paste\"\n"
        "      + button in \"bottom\" label \"Delete\" onclick \"action delete\"\n";

    channel_send_fmt0(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, data);
    cstring_write_fmt0(path, 256, "initrd:\\0", 0);
    updatepath();
    updatecontent();

}

void init(void)
{

    option_add("wm-service", "12345");
    channel_autoclose(EVENT_MAIN, 0);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

