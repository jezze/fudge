enum keymod
{

    KEYMOD_NONE                         = 0,
    KEYMOD_SHIFT                        = 1,
    KEYMOD_ALT                          = 2

};

struct keycode
{

    unsigned char length;
    unsigned char value[4];

};

struct keyset
{

    struct keycode keycode[4];

};
