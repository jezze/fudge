#include <fudge.h>
#include <abi.h>
#include "bind.h"

#define NUM_KEYS                        123
#define NUM_MODS                        4
#define NUM_ACTIONS                     10
#define NUM_BINDINGS                    32

struct name
{

    unsigned int value;
    char *name;

};

static struct name keys[NUM_KEYS] = {
    {KEYS_KEY_ESCAPE, "escape"},
    {KEYS_KEY_1, "1"},
    {KEYS_KEY_2, "2"},
    {KEYS_KEY_3, "3"},
    {KEYS_KEY_4, "4"},
    {KEYS_KEY_5, "5"},
    {KEYS_KEY_6, "6"},
    {KEYS_KEY_7, "7"},
    {KEYS_KEY_8, "8"},
    {KEYS_KEY_9, "9"},
    {KEYS_KEY_0, "0"},
    {KEYS_KEY_MINUS, "minus"},
    {KEYS_KEY_EQUAL, "equal"},
    {KEYS_KEY_BACKSPACE, "backspace"},
    {KEYS_KEY_TAB, "tab"},
    {KEYS_KEY_Q, "q"},
    {KEYS_KEY_W, "w"},
    {KEYS_KEY_E, "e"},
    {KEYS_KEY_R, "r"},
    {KEYS_KEY_T, "t"},
    {KEYS_KEY_Y, "y"},
    {KEYS_KEY_U, "u"},
    {KEYS_KEY_I, "i"},
    {KEYS_KEY_O, "o"},
    {KEYS_KEY_P, "p"},
    {KEYS_KEY_LBRACKET, "lbracket"},
    {KEYS_KEY_RBRACKET, "rbracket"},
    {KEYS_KEY_ENTER, "enter"},
    {KEYS_KEY_LCONTROL, "lcontrol"},
    {KEYS_KEY_A, "a"},
    {KEYS_KEY_S, "s"},
    {KEYS_KEY_D, "d"},
    {KEYS_KEY_F, "f"},
    {KEYS_KEY_G, "g"},
    {KEYS_KEY_H, "h"},
    {KEYS_KEY_J, "j"},
    {KEYS_KEY_K, "k"},
    {KEYS_KEY_L, "l"},
    {KEYS_KEY_SEMICOLON, "semicolon"},
    {KEYS_KEY_SINGLEQUOTE, "singlequote"},
    {KEYS_KEY_BACKTICK, "backtick"},
    {KEYS_KEY_LSHIFT, "lshift"},
    {KEYS_KEY_BACKSLASH, "backslash"},
    {KEYS_KEY_Z, "z"},
    {KEYS_KEY_X, "x"},
    {KEYS_KEY_C, "c"},
    {KEYS_KEY_V, "v"},
    {KEYS_KEY_B, "b"},
    {KEYS_KEY_N, "n"},
    {KEYS_KEY_M, "m"},
    {KEYS_KEY_COMMA, "comma"},
    {KEYS_KEY_PERIOD, "period"},
    {KEYS_KEY_SLASH, "slash"},
    {KEYS_KEY_RSHIFT, "rshift"},
    {KEYS_KEY_KEYPAD_STAR, "keypad_star"},
    {KEYS_KEY_LALT, "lalt"},
    {KEYS_KEY_SPACE, "space"},
    {KEYS_KEY_CAPSLOCK, "capslock"},
    {KEYS_KEY_F1, "f1"},
    {KEYS_KEY_F2, "f2"},
    {KEYS_KEY_F3, "f3"},
    {KEYS_KEY_F4, "f4"},
    {KEYS_KEY_F5, "f5"},
    {KEYS_KEY_F6, "f6"},
    {KEYS_KEY_F7, "f7"},
    {KEYS_KEY_F8, "f8"},
    {KEYS_KEY_F9, "f9"},
    {KEYS_KEY_F10, "f10"},
    {KEYS_KEY_NUMLOCK, "numlock"},
    {KEYS_KEY_SCROLLLOCK, "scrolllock"},
    {KEYS_KEY_KEYPAD_7, "keypad_7"},
    {KEYS_KEY_KEYPAD_8, "keypad_8"},
    {KEYS_KEY_KEYPAD_9, "keypad_9"},
    {KEYS_KEY_KEYPAD_MINUS, "keypad_minus"},
    {KEYS_KEY_KEYPAD_4, "keypad_4"},
    {KEYS_KEY_KEYPAD_5, "keypad_5"},
    {KEYS_KEY_KEYPAD_6, "keypad_6"},
    {KEYS_KEY_KEYPAD_PLUS, "keypad_plus"},
    {KEYS_KEY_KEYPAD_1, "keypad_1"},
    {KEYS_KEY_KEYPAD_2, "keypad_2"},
    {KEYS_KEY_KEYPAD_3, "keypad_3"},
    {KEYS_KEY_KEYPAD_0, "keypad_0"},
    {KEYS_KEY_KEYPAD_PERIOD, "keypad_period"},
    {KEYS_KEY_F11, "f11"},
    {KEYS_KEY_F12, "f12"},
    {KEYS_KEY_PREVIOUSTRACK, "previoustrack"},
    {KEYS_KEY_NEXTTRACK, "nexttrack"},
    {KEYS_KEY_KEYPAD_ENTER, "keypad_enter"},
    {KEYS_KEY_RCONTROL, "rcontrol"},
    {KEYS_KEY_MUTE, "mute"},
    {KEYS_KEY_CALCULATOR, "calculator"},
    {KEYS_KEY_PLAY, "play"},
    {KEYS_KEY_STOP, "stop"},
    {KEYS_KEY_VOLUMEDOWN, "volumedown"},
    {KEYS_KEY_VOLUMEUP, "volumeup"},
    {KEYS_KEY_WEBHOME, "webhome"},
    {KEYS_KEY_KEYPAD_SLASH, "keypad_slash"},
    {KEYS_KEY_RALT, "ralt"},
    {KEYS_KEY_HOME, "home"},
    {KEYS_KEY_CURSORUP, "cursorup"},
    {KEYS_KEY_PAGEUP, "pageup"},
    {KEYS_KEY_CURSORLEFT, "cursorleft"},
    {KEYS_KEY_CURSORRIGHT, "cursorright"},
    {KEYS_KEY_END, "end"},
    {KEYS_KEY_CURSORDOWN, "cursordown"},
    {KEYS_KEY_PAGEDOWN, "pagedown"},
    {KEYS_KEY_INSERT, "insert"},
    {KEYS_KEY_DELETE, "delete"},
    {KEYS_KEY_LMETA, "lmeta"},
    {KEYS_KEY_RMETA, "rmeta"},
    {KEYS_KEY_APPS, "apps"},
    {KEYS_KEY_POWER, "power"},
    {KEYS_KEY_SLEEP, "sleep"},
    {KEYS_KEY_WAKE, "wake"},
    {KEYS_KEY_WEBSEARCH, "websearch"},
    {KEYS_KEY_FAVOURITES, "favourites"},
    {KEYS_KEY_REFRESH, "refresh"},
    {KEYS_KEY_WEBSTOP, "webstop"},
    {KEYS_KEY_WEBFORWARD, "webforward"},
    {KEYS_KEY_WEBBACK, "webback"},
    {KEYS_KEY_MYCOMPUTER, "mycomputer"},
    {KEYS_KEY_EMAIL, "email"},
    {KEYS_KEY_MEDIASELECT, "mediaselect"}
};

static struct name mods[NUM_MODS] = {
    {KEYS_MOD_SHIFT, "shift"},
    {KEYS_MOD_ALT, "alt"},
    {KEYS_MOD_CTRL, "ctrl"},
    {KEYS_MOD_META, "meta"}
};

static struct name actions[NUM_ACTIONS] = {
    {BIND_ACTION_CLOSE, "close"},
    {BIND_ACTION_FOCUSNEXT, "focusnext"},
    {BIND_ACTION_FOCUSPREVIOUS, "focusprevious"},
    {BIND_ACTION_MARKALL, "markall"},
    {BIND_ACTION_NEXT, "next"},
    {BIND_ACTION_PREVIOUS, "previous"},
    {BIND_ACTION_RUN, "run"},
    {BIND_ACTION_SCROLLDOWN, "scrolldown"},
    {BIND_ACTION_SCROLLUP, "scrollup"},
    {BIND_ACTION_SWAP, "swap"}
};

static struct bind_binding bindings[NUM_BINDINGS];
static unsigned int nbindings;

static unsigned int findname(struct name *names, unsigned int count, char *name, unsigned int length)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (cstring_length(names[i].name) == length && buffer_match(names[i].name, name, length))
            return names[i].value;

    }

    return 0;

}

static unsigned int findmod(char *name, unsigned int length)
{

    unsigned int mod = 0;

    while (length)
    {

        unsigned int part = buffer_findbyte(name, length, '+');

        mod |= findname(mods, NUM_MODS, name, part);
        name += part;
        length -= part;

        if (length)
        {

            name++;
            length--;

        }

    }

    return mod;

}

void bind_add(char *line, unsigned int length)
{

    struct bind_binding *binding = &bindings[nbindings];
    unsigned int k = buffer_findbyte(line, length, ' ');
    unsigned int m = (k < length) ? k + 1 + buffer_findbyte(line + k + 1, length - k - 1, ' ') : length;
    unsigned int a = (m < length) ? m + 1 + buffer_findbyte(line + m + 1, length - m - 1, ' ') : length;

    if (nbindings >= NUM_BINDINGS || a > length)
        return;

    binding->key = findname(keys, NUM_KEYS, line, k);
    binding->mod = findmod(line + k + 1, m - k - 1);
    binding->action = findname(actions, NUM_ACTIONS, line + m + 1, a - m - 1);
    binding->command[0] = '\0';

    if (a < length && length - a - 1 < BIND_COMMANDSIZE)
    {

        buffer_write(binding->command, BIND_COMMANDSIZE, line + a + 1, length - a - 1, 0);
        binding->command[length - a - 1] = '\0';

    }

    if (binding->key && binding->action)
        nbindings++;

}

void bind_reset(void)
{

    nbindings = 0;

}

struct bind_binding *bind_find(unsigned int key, unsigned int mod)
{

    unsigned int i;

    for (i = 0; i < nbindings; i++)
    {

        if (bindings[i].key == key && bindings[i].mod == (mod & (KEYS_MOD_SHIFT | KEYS_MOD_ALT | KEYS_MOD_CTRL | KEYS_MOD_META)))
            return &bindings[i];

    }

    return 0;

}

