#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define STATE_SUM                       0
#define STATE_ADD                       1
#define STATE_SUB                       2
#define STATE_MUL                       3
#define STATE_DIV                       4

static unsigned int keypad[10] = {KEYS_KEY_KEYPAD_0, KEYS_KEY_KEYPAD_1, KEYS_KEY_KEYPAD_2, KEYS_KEY_KEYPAD_3, KEYS_KEY_KEYPAD_4, KEYS_KEY_KEYPAD_5, KEYS_KEY_KEYPAD_6, KEYS_KEY_KEYPAD_7, KEYS_KEY_KEYPAD_8, KEYS_KEY_KEYPAD_9};
static unsigned int prevstate;
static int number;
static int accumulator;

static void refresh(unsigned int wm, int value)
{

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= result label \"%i\"\n", &value);

}

static void updatestate(unsigned int wm, int state)
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
        if (number == -1)
            accumulator = -accumulator;
        else
            accumulator = (number) ? accumulator / number : 0;

        number = 0;

        break;

    }

    prevstate = state;

    switch (state)
    {

    case STATE_SUM:
        refresh(wm, accumulator);

        break;

    }

}

static void updatevalue(unsigned int wm, int value)
{

    number *= 10;
    number += value;

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

}

static void onwmkeypress(struct message *message)
{

    struct event_wmkeypress *wmkeypress = message->data;
    unsigned int c = (wmkeypress->length == 1) ? wmkeypress->unicode : 0;
    unsigned int i;

    for (i = 0; i < 10; i++)
    {

        if (wmkeypress->id == keypad[i])
            c = '0' + i;

    }

    if (wmkeypress->id == KEYS_KEY_KEYPAD_PLUS)
        c = '+';

    if (wmkeypress->id == KEYS_KEY_KEYPAD_MINUS)
        c = '-';

    if (wmkeypress->id == KEYS_KEY_KEYPAD_STAR)
        c = '*';

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

