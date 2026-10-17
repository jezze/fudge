#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define STATE_SUM                       0
#define STATE_ADD                       1
#define STATE_SUB                       2
#define STATE_MUL                       3
#define STATE_DIV                       4
#define INTMAX                          ((int)(~0u >> 1))
#define INTMIN                          (-INTMAX - 1)

static unsigned int prevstate;
static int number;
static unsigned int typed;
static int accumulator;

static void refresh(unsigned int wm, int value)
{

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= result label \"%i\"\n", &value);

}

static void clear(void)
{

    prevstate = STATE_SUM;
    number = 0;
    typed = 0;
    accumulator = 0;

}

static void fail(unsigned int wm)
{

    clear();
    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= result label \"Error\"\n");

}

static unsigned int invalid(unsigned int state, int a, int b)
{

    switch (state)
    {

    case STATE_ADD:
        return a > INTMAX - b;

    case STATE_SUB:
        return a < INTMIN + b;

    case STATE_MUL:
        return b && (a > INTMAX / b || a < INTMIN / b);

    case STATE_DIV:
        return !b;

    }

    return 0;

}

static void updatestate(unsigned int wm, int state)
{

    if (invalid(prevstate, accumulator, number))
    {

        fail(wm);

        return;

    }

    switch (prevstate)
    {

    case STATE_SUM:
        if (typed)
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
    typed = 0;

    switch (state)
    {

    case STATE_SUM:
        refresh(wm, accumulator);

        break;

    }

}

static void reset(unsigned int wm)
{

    clear();
    refresh(wm, 0);

}

static void updatevalue(unsigned int wm, int value)
{

    if (number > (INTMAX - value) / 10)
    {

        fail(wm);

        return;

    }

    number *= 10;
    number += value;
    typed = 1;

    refresh(wm, number);

}

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wcalc.alfi";

        channel_send(0, wm, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        channel_hold(0);
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onwmevent(struct message *message)
{

    struct event_wmevent *event = message->data;

    if (kv_match(event, "q=num"))
        updatevalue(message->source, kv_getvalue(event, "value=", 10));
    else if (kv_match(event, "q=sum"))
        updatestate(message->source, STATE_SUM);
    else if (kv_match(event, "q=add"))
        updatestate(message->source, STATE_ADD);
    else if (kv_match(event, "q=sub"))
        updatestate(message->source, STATE_SUB);
    else if (kv_match(event, "q=mul"))
        updatestate(message->source, STATE_MUL);
    else if (kv_match(event, "q=div"))
        updatestate(message->source, STATE_DIV);
    else if (kv_match(event, "q=clear"))
        reset(message->source);

}

static void onwmkeypress(struct message *message)
{

    struct event_wmkeypress *wmkeypress = message->data;
    unsigned int c = (wmkeypress->length == 1) ? wmkeypress->unicode : 0;

    switch (c)
    {

    case '+':
        updatestate(message->source, STATE_ADD);

        break;

    case '-':
        updatestate(message->source, STATE_SUB);

        break;

    case '*':
        updatestate(message->source, STATE_MUL);

        break;

    case '/':
        updatestate(message->source, STATE_DIV);

        break;

    case 'c':
        reset(message->source);

        break;

    case '=':
    case '\n':
        updatestate(message->source, STATE_SUM);

        break;

    default:
        if (c >= '0' && c <= '9')
            updatevalue(message->source, c - '0');

        break;

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

}

