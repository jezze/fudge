#define KEYMOD_NONE                     0
#define KEYMOD_SHIFT                    1
#define KEYMOD_ALT                      2
#define KEYMOD_CTRL                     4
#define KEYMAP_NONE                     0
#define KEYMAP_US                       1
#define KEYMAP_SE                       2

struct keycode
{

    unsigned char length;
    unsigned char value[4];

};

struct keymap
{

    struct keycode keycode[4];

};

struct keystate
{

    unsigned int extended;
    unsigned int mod;

};

struct keycode *keymap_getkeycode(struct keystate *keystate, unsigned int type, unsigned int scancode);
