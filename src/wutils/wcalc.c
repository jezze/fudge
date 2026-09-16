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

static void refresh(unsigned int wm, int value)
{

    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "= result label \"%i\"\n", &value);

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
        accumulator /= number;
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

        channel_send(0, wm, EVENT_WMMAP, 0, 0);

        while (channel_process(0) != EVENT_WMCLOSE);

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

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wcalc.alfi";

    channel_send(0, message->source, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);

}

static void onwmkeypress(struct message *message)
{

    struct event_wmkeypress *wmkeypress = message->data;

    switch (wmkeypress->id)
    {

    case KEYS_KEY_0:
    case KEYS_KEY_KEYPAD_0:
        updatevalue(message->source, 0);

        break;

    case KEYS_KEY_1:
    case KEYS_KEY_KEYPAD_1:
        updatevalue(message->source, 1);

        break;

    case KEYS_KEY_2:
    case KEYS_KEY_KEYPAD_2:
        updatevalue(message->source, 2);

        break;

    case KEYS_KEY_3:
    case KEYS_KEY_KEYPAD_3:
        updatevalue(message->source, 3);

        break;

    case KEYS_KEY_4:
    case KEYS_KEY_KEYPAD_4:
        updatevalue(message->source, 4);

        break;

    case KEYS_KEY_5:
    case KEYS_KEY_KEYPAD_5:
        updatevalue(message->source, 5);

        break;

    case KEYS_KEY_6:
    case KEYS_KEY_KEYPAD_6:
        updatevalue(message->source, 6);

        break;

    case KEYS_KEY_7:
    case KEYS_KEY_KEYPAD_7:
        updatevalue(message->source, 7);

        break;

    case KEYS_KEY_8:
        if (wmkeypress->keymod & KEYS_MOD_SHIFT)
            updatestate(message->source, STATE_MUL);
        else
            updatevalue(message->source, 8);

        break;

    case KEYS_KEY_KEYPAD_8:
        updatevalue(message->source, 8);

        break;

    case KEYS_KEY_9:
    case KEYS_KEY_KEYPAD_9:
        updatevalue(message->source, 9);

        break;

    case KEYS_KEY_MINUS:
    case KEYS_KEY_KEYPAD_MINUS:
        updatestate(message->source, STATE_SUB);

        break;

    case KEYS_KEY_EQUAL:
        if (wmkeypress->keymod & KEYS_MOD_SHIFT)
            updatestate(message->source, STATE_ADD);
        else
            updatestate(message->source, STATE_SUM);

        break;

    case KEYS_KEY_KEYPAD_PLUS:
        updatestate(message->source, STATE_ADD);

        break;

    case KEYS_KEY_ENTER:
    case KEYS_KEY_KEYPAD_ENTER:
        updatestate(message->source, STATE_SUM);

        break;

    case KEYS_KEY_SLASH:
    case KEYS_KEY_KEYPAD_SLASH:
        updatestate(message->source, STATE_DIV);

        break;

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);

    while (channel_process(0));

}

