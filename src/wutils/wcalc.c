#include <fudge.h>
#include <abi.h>
#include "kv.h"

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

    channel_send_fmt1(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, "= result content \"%i\"\n", &value);

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

    channel_send(option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process());

    channel_send(option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onwmevent(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmevent *event = mdata;

    if (kv_match(event, "q=num"))
        updatevalue(kv_getvalue(event, "value=", 10));
    else if (kv_match(event, "q=sum"))
        updatestate(STATE_SUM);
    else if (kv_match(event, "q=add"))
        updatestate(STATE_ADD);
    else if (kv_match(event, "q=sub"))
        updatestate(STATE_SUB);
    else if (kv_match(event, "q=mul"))
        updatestate(STATE_MUL);
    else if (kv_match(event, "q=div"))
        updatestate(STATE_DIV);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data0 =
        "+ window id \"window\" title \"Calculator\"\n"
        "  + layout id \"base\" in \"window\" flow \"vertical\" padding \"1\"\n"
        "    + layout id \"display\" in \"base\" flow \"horizontal\"\n"
        "      + textbox id \"output\" in \"display\" mode \"readonly\" span \"1\"\n"
        "        + text id \"result\" in \"output\" content \"0\" halign \"right\"\n";
    char *data1 =
        "    + layout id \"buttons\" in \"base\" flow \"vertical\" span \"1\"\n"
        "      + layout id \"row1\" in \"buttons\" flow \"horizontal-stretch\" span \"1\"\n"
        "        + button in \"row1\" label \"7\" span \"1\" onclick \"q=num&value=7\"\n"
        "        + button in \"row1\" label \"8\" span \"1\" onclick \"q=num&value=8\"\n"
        "        + button in \"row1\" label \"9\" span \"1\" onclick \"q=num&value=9\"\n"
        "        + button in \"row1\" label \"/\" span \"1\" onclick \"q=div\"\n"
        "      + layout id \"row2\" in \"buttons\" flow \"horizontal-stretch\" span \"1\"\n"
        "        + button in \"row2\" label \"4\" span \"1\" onclick \"q=num&value=4\"\n"
        "        + button in \"row2\" label \"5\" span \"1\" onclick \"q=num&value=5\"\n"
        "        + button in \"row2\" label \"6\" span \"1\" onclick \"q=num&value=6\"\n"
        "        + button in \"row2\" label \"x\" span \"1\" onclick \"q=mul\"\n"
        "      + layout id \"row3\" in \"buttons\" flow \"horizontal-stretch\" span \"1\"\n"
        "        + button in \"row3\" label \"3\" span \"1\" onclick \"q=num&value=3\"\n"
        "        + button in \"row3\" label \"2\" span \"1\" onclick \"q=num&value=2\"\n"
        "        + button in \"row3\" label \"1\" span \"1\" onclick \"q=num&value=1\"\n"
        "        + button in \"row3\" label \"-\" span \"1\" onclick \"q=sub\"\n"
        "      + layout id \"row4\" in \"buttons\" flow \"horizontal-stretch\" span \"1\"\n"
        "        + button in \"row4\" label \"0\" span \"1\" onclick \"q=num&value=0\"\n"
        "        + button in \"row4\" label \".\" span \"1\" onclick \"q=dot\"\n"
        "        + button in \"row4\" label \"+\" span \"1\" onclick \"q=add\"\n"
        "        + button in \"row4\" label \"=\" span \"1\" onclick \"q=sum\"\n";

    channel_send_fmt0(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, data0);
    channel_send_fmt0(option_getdecimal("wm-service"), EVENT_WMRENDERDATA, data1);

}

static void onwmkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmkeypress *wmkeypress = mdata;

    switch (wmkeypress->id)
    {

    case KEYS_KEY_0:
    case KEYS_KEY_KEYPAD_0:
        updatevalue(0);

        break;

    case KEYS_KEY_1:
    case KEYS_KEY_KEYPAD_1:
        updatevalue(1);

        break;

    case KEYS_KEY_2:
    case KEYS_KEY_KEYPAD_2:
        updatevalue(2);

        break;

    case KEYS_KEY_3:
    case KEYS_KEY_KEYPAD_3:
        updatevalue(3);

        break;

    case KEYS_KEY_4:
    case KEYS_KEY_KEYPAD_4:
        updatevalue(4);

        break;

    case KEYS_KEY_5:
    case KEYS_KEY_KEYPAD_5:
        updatevalue(5);

        break;

    case KEYS_KEY_6:
    case KEYS_KEY_KEYPAD_6:
        updatevalue(6);

        break;

    case KEYS_KEY_7:
    case KEYS_KEY_KEYPAD_7:
        updatevalue(7);

        break;

    case KEYS_KEY_8:
        if (wmkeypress->keymod & KEYS_MOD_SHIFT)
            updatestate(STATE_MUL);
        else
            updatevalue(8);

        break;

    case KEYS_KEY_KEYPAD_8:
        updatevalue(8);

        break;

    case KEYS_KEY_9:
    case KEYS_KEY_KEYPAD_9:
        updatevalue(9);

        break;

    case KEYS_KEY_MINUS:
    case KEYS_KEY_KEYPAD_MINUS:
        updatestate(STATE_SUB);

        break;

    case KEYS_KEY_EQUAL:
        if (wmkeypress->keymod & KEYS_MOD_SHIFT)
            updatestate(STATE_ADD);
        else
            updatestate(STATE_SUM);

        break;

    case KEYS_KEY_KEYPAD_PLUS:
        updatestate(STATE_ADD);

        break;

    case KEYS_KEY_ENTER:
    case KEYS_KEY_KEYPAD_ENTER:
        updatestate(STATE_SUM);

        break;

    case KEYS_KEY_SLASH:
    case KEYS_KEY_KEYPAD_SLASH:
        updatestate(STATE_DIV);

        break;

    }

}

void init(void)
{

    option_add("wm-service", "12345");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

