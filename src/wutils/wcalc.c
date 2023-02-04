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

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    count += cstring_write(buffer, BUFFER_SIZE, "= result content \"", count);
    count += cstring_writevalue(buffer, BUFFER_SIZE, value, 10, 0, count);
    count += cstring_write(buffer, BUFFER_SIZE, "\"\n", count);

    channel_sendbuffer(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, count, buffer);

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

    if (file_walk2(FILE_L0, "system:service/wm"))
        file_notify(FILE_L0, EVENT_WMMAP, 0, 0);
    else
        channel_warning("Could not open window manager service");

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(CHANNEL_DEFAULT, EVENT_WMUNMAP);
    channel_close();

}

static void onwmclick(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmclick *wmclick = mdata;

    if (cstring_match(wmclick->clicked, "button="))
        updatestate(STATE_SUM);
    else if (cstring_match(wmclick->clicked, "button0"))
        updatevalue(0);
    else if (cstring_match(wmclick->clicked, "button1"))
        updatevalue(1);
    else if (cstring_match(wmclick->clicked, "button2"))
        updatevalue(2);
    else if (cstring_match(wmclick->clicked, "button3"))
        updatevalue(3);
    else if (cstring_match(wmclick->clicked, "button4"))
        updatevalue(4);
    else if (cstring_match(wmclick->clicked, "button5"))
        updatevalue(5);
    else if (cstring_match(wmclick->clicked, "button6"))
        updatevalue(6);
    else if (cstring_match(wmclick->clicked, "button7"))
        updatevalue(7);
    else if (cstring_match(wmclick->clicked, "button8"))
        updatevalue(8);
    else if (cstring_match(wmclick->clicked, "button9"))
        updatevalue(9);
    else if (cstring_match(wmclick->clicked, "button+"))
        updatestate(STATE_ADD);
    else if (cstring_match(wmclick->clicked, "button-"))
        updatestate(STATE_SUB);
    else if (cstring_match(wmclick->clicked, "buttonx"))
        updatestate(STATE_MUL);
    else if (cstring_match(wmclick->clicked, "button/"))
        updatestate(STATE_DIV);

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Calculator\"\n"
        "+ layout id \"base\" in \"window\" type \"vertical\"\n"
        "+ layout id \"container\" in \"base\" type \"vertical\" placement \"stretched\" padding \"8\"\n"
        "+ textbox id \"output\" in \"container\" mode \"readonly\"\n"
        "+ text id \"result\" in \"output\" content \"0\" halign \"right\"\n"
        "+ grid id \"buttons\" in \"base\" columns \"4\" placement \"stretched\" padding \"8\"\n"
        "+ button id \"button7\" in \"buttons\" label \"7\"\n"
        "+ button id \"button8\" in \"buttons\" label \"8\"\n"
        "+ button id \"button9\" in \"buttons\" label \"9\"\n"
        "+ button id \"button/\" in \"buttons\" label \"/\"\n"
        "+ button id \"button4\" in \"buttons\" label \"4\"\n"
        "+ button id \"button5\" in \"buttons\" label \"5\"\n"
        "+ button id \"button6\" in \"buttons\" label \"6\"\n"
        "+ button id \"buttonx\" in \"buttons\" label \"x\"\n"
        "+ button id \"button3\" in \"buttons\" label \"3\"\n"
        "+ button id \"button2\" in \"buttons\" label \"2\"\n"
        "+ button id \"button1\" in \"buttons\" label \"1\"\n"
        "+ button id \"button-\" in \"buttons\" label \"-\"\n"
        "+ button id \"button0\" in \"buttons\" label \"0\"\n"
        "+ button id \"button.\" in \"buttons\" label \".\"\n"
        "+ button id \"button+\" in \"buttons\" label \"+\"\n"
        "+ button id \"button=\" in \"buttons\" label \"=\"\n";

    channel_sendfmt0(CHANNEL_DEFAULT, EVENT_WMRENDERDATA, data);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMCLICK, onwmclick);
    channel_bind(EVENT_WMINIT, onwminit);

}

