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

    char num[CSTRING_NUMSIZE];
    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    count += buffer_write(buffer, BUFFER_SIZE, "= result content \"", 18, count);
    count += buffer_write(buffer, BUFFER_SIZE, num, cstring_wvalue(num, CSTRING_NUMSIZE, value, 10, 0), count);
    count += buffer_write(buffer, BUFFER_SIZE, "\"\n", 2, count);

    file_notify(FILE_G0, EVENT_WMRENDERDATA, count, buffer);

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

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwmevent(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmevent *wmevent = mdata;

    if (wmevent->type == EVENT_WMEVENTCLICK)
    {

        if (cstring_match(wmevent->clicked, "button="))
            updatestate(STATE_SUM);
        else if (cstring_match(wmevent->clicked, "button0"))
            updatevalue(0);
        else if (cstring_match(wmevent->clicked, "button1"))
            updatevalue(1);
        else if (cstring_match(wmevent->clicked, "button2"))
            updatevalue(2);
        else if (cstring_match(wmevent->clicked, "button3"))
            updatevalue(3);
        else if (cstring_match(wmevent->clicked, "button4"))
            updatevalue(4);
        else if (cstring_match(wmevent->clicked, "button5"))
            updatevalue(5);
        else if (cstring_match(wmevent->clicked, "button6"))
            updatevalue(6);
        else if (cstring_match(wmevent->clicked, "button7"))
            updatevalue(7);
        else if (cstring_match(wmevent->clicked, "button8"))
            updatevalue(8);
        else if (cstring_match(wmevent->clicked, "button9"))
            updatevalue(9);
        else if (cstring_match(wmevent->clicked, "button+"))
            updatestate(STATE_ADD);
        else if (cstring_match(wmevent->clicked, "button-"))
            updatestate(STATE_SUB);
        else if (cstring_match(wmevent->clicked, "buttonx"))
            updatestate(STATE_MUL);
        else if (cstring_match(wmevent->clicked, "button/"))
            updatestate(STATE_DIV);

    }

}

static void onwmshow(unsigned int source, void *mdata, unsigned int msize)
{

    char *data =
        "+ window id \"window\" title \"Calculator\"\n"
        "+ container id \"base\" in \"window\" layout \"vertical\"\n"
        "+ container id \"container\" in \"base\" layout \"vertical\" placement \"stretched\" padding \"16\"\n"
        "+ textbox id \"result\" in \"container\" content \"0\" mode \"readonly\"\n"
        "+ grid id \"buttons\" in \"base\" columns \"4\" placement \"stretched\" padding \"16\"\n"
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

    file_notify(FILE_G0, EVENT_WMRENDERDATA, cstring_length(data), data);

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMSHOW, onwmshow);

}

