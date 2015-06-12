#define KEYMOD_NONE                     0
#define KEYMOD_SHIFT                    1
#define KEYMOD_ALT                      2

struct keycode
{

    unsigned char length;
    unsigned char value[4];

};

struct keyset
{

    struct keycode keycode[4];

};
