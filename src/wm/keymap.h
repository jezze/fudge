#define KEYMOD_NONE                     0
#define KEYMOD_SHIFT                    1
#define KEYMOD_ALT                      2
#define KEYMAP_NONE                     0
#define KEYMAP_US                       1
#define KEYMAP_SE                       2

struct keycode
{

    unsigned char length;
    unsigned char value[4];

};

struct keyset
{

    struct keycode keycode[4];

};

struct keycode *getkeycode(unsigned int type, unsigned int scancode, unsigned int modifier);
