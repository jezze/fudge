#include <fudge.h>
#include <abi.h>

#define STATE_SUM                       0
#define STATE_ADD                       1
#define STATE_SUB                       2
#define STATE_MUL                       3
#define STATE_DIV                       4

static unsigned int prevstate;
static int number;
static int accumulator;

static void refresh(int value)
{

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, "= result content \"%i\"\n", &value);

}

static void updatestate(int state)
{

    switch (prevstate)
    {

    case STATE_SUM:
        accumulator = number;
        number = 0;

        break;

    case STATE_ADD:
        accumulator += number;
        number = 0;

        break;

    case STATE_SUB:
        accumulator -= number;
        number = 0;

        break;

    case STATE_MUL:
        accumulator *= number;
        number = 0;

        break;

    case STATE_DIV:
        accumulator /= number;
        number = 0;

        break;

    }

    prevstate = state;

    switch (state)
    {

    case STATE_SUM:
        refresh(accumulator);

        break;

    }

}

static void updatevalue(int value)
{

    number *= 10;
    number += value;

    refresh(number);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_L0, "system:service/wm"))
        channel_panic();

    file_notify(FILE_L0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(CHANNEL_DEFAULT, EVENT_WMUNMAP);
    channel_close();

}

static void onwmevent(unsigned int source, void *mdata, unsigned int msize)
{

    char *data = (char *)mdata + sizeof (struct event_wmevent);

    if (cstring_submatch(data, "eq"))
        updatestate(STATE_SUM);
    else if (cstring_submatch(data, "0"))
        updatevalue(0);
    else if (cstring_submatch(data, "1"))
        updatevalue(1);
    else if (cstring_submatch(data, "2"))
        updatevalue(2);
    else if (cstring_submatch(data, "3"))
        updatevalue(3);
    else if (cstring_submatch(data, "4"))
        updatevalue(4);
    else if (cstring_submatch(data, "5"))
        updatevalue(5);
    else if (cstring_submatch(data, "6"))
        updatevalue(6);
    else if (cstring_submatch(data, "7"))
        updatevalue(7);
    else if (cstring_submatch(data, "8"))
        updatevalue(8);
    else if (cstring_submatch(data, "9"))
        updatevalue(9);
    else if (cstring_submatch(data, "add"))
        updatestate(STATE_ADD);
    else if (cstring_submatch(data, "sub"))
        updatestate(STATE_SUB);
    else if (cstring_submatch(data, "mul"))
        updatestate(STATE_MUL);
    else if (cstring_submatch(data, "div"))
        updatestate(STATE_DIV);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data0 =
        "+ window id \"window\" title \"CALCULATOR\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical-stretch\" span \"1\"\n"
        "    + layout id \"container\" in \"base\" flow \"horizontal-stretch\" padding \"8\" span \"1\"\n"
        "      + textbox id \"output\" in \"container\" mode \"readonly\" span \"1\"\n"
        "        + text id \"result\" in \"output\" content \"0\" halign \"right\"\n";
    char *data1 =
        "    + layout id \"row1\" in \"base\" flow \"horizontal-stretch\" padding \"8\" span \"1\"\n"
        "      + button in \"row1\" label \"7\" span \"1\" onclick \"7\"\n"
        "      + button in \"row1\" label \"8\" span \"1\" onclick \"8\"\n"
        "      + button in \"row1\" label \"9\" span \"1\" onclick \"9\"\n"
        "      + button in \"row1\" label \"/\" span \"1\" onclick \"div\"\n"
        "    + layout id \"row2\" in \"base\" flow \"horizontal-stretch\" padding \"8\" span \"1\"\n"
        "      + button in \"row2\" label \"4\" span \"1\" onclick \"4\"\n"
        "      + button in \"row2\" label \"5\" span \"1\" onclick \"5\"\n"
        "      + button in \"row2\" label \"6\" span \"1\" onclick \"6\"\n"
        "      + button in \"row2\" label \"x\" span \"1\" onclick \"mul\"\n";
    char *data2 =
        "    + layout id \"row3\" in \"base\" flow \"horizontal-stretch\" padding \"8\" span \"1\"\n"
        "      + button in \"row3\" label \"3\" span \"1\" onclick \"3\"\n"
        "      + button in \"row3\" label \"2\" span \"1\" onclick \"2\"\n"
        "      + button in \"row3\" label \"1\" span \"1\" onclick \"1\"\n"
        "      + button in \"row3\" label \"-\" span \"1\" onclick \"sub\"\n"
        "    + layout id \"row4\" in \"base\" flow \"horizontal-stretch\" padding \"8\" span \"1\"\n"
        "      + button in \"row4\" label \"0\" span \"1\" onclick \"0\"\n"
        "      + button in \"row4\" label \".\" span \"1\" onclick \"point\"\n"
        "      + button in \"row4\" label \"+\" span \"1\" onclick \"add\"\n"
        "      + button in \"row4\" label \"=\" span \"1\" onclick \"eq\"\n";

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data0);
    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data1);
    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data2);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

