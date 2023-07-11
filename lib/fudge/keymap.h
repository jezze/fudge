#define KEYMOD_NONE                     0
#define KEYMOD_SHIFT                    1
#define KEYMOD_ALT                      2
#define KEYMOD_CTRL                     4
#define KEYMAP_NONE                     0
#define KEYMAP_US                       1
#define KEYMAP_SE                       2
#define KEYMAP_LAYOUT_NONE              0
#define KEYMAP_LAYOUT_QWERTY_US         1
#define KEY_ESC                         0x1001
#define KEY_1                           0x1002
#define KEY_2                           0x1003
#define KEY_3                           0x1004
#define KEY_4                           0x1005
#define KEY_5                           0x1006
#define KEY_6                           0x1007
#define KEY_7                           0x1008
#define KEY_8                           0x1009
#define KEY_9                           0x100A
#define KEY_0                           0x100B
#define KEY_MINUS                       0x100C
#define KEY_EQUAL                       0x100D
#define KEY_BACKSPACE                   0x100E
#define KEY_TAB                         0x100F
#define KEY_Q                           0x1010
#define KEY_W                           0x1011
#define KEY_E                           0x1012
#define KEY_R                           0x1013
#define KEY_T                           0x1014
#define KEY_Y                           0x1015
#define KEY_U                           0x1016
#define KEY_I                           0x1017
#define KEY_O                           0x1018
#define KEY_P                           0x1019
#define KEY_LBRACKET                    0x101A
#define KEY_RBRACKET                    0x101B
#define KEY_ENTER                       0x101C
#define KEY_LCONTROL                    0x101D
#define KEY_A                           0x101E
#define KEY_S                           0x101F
#define KEY_D                           0x1020
#define KEY_F                           0x1021
#define KEY_G                           0x1022
#define KEY_H                           0x1023
#define KEY_J                           0x1024
#define KEY_K                           0x1025
#define KEY_L                           0x1026
#define KEY_SEMICOLON                   0x1027
#define KEY_SINGLEQUOTE                 0x1028
#define KEY_BACKTICK                    0x1029
#define KEY_LSHIFT                      0x102A
#define KEY_BACKSLASH                   0x102B
#define KEY_Z                           0x102C
#define KEY_X                           0x102D
#define KEY_C                           0x102E
#define KEY_V                           0x102F
#define KEY_B                           0x1030
#define KEY_N                           0x1031
#define KEY_M                           0x1032
#define KEY_COMMA                       0x1033
#define KEY_PERIOD                      0x1034
#define KEY_SLASH                       0x1035
#define KEY_RSHIFT                      0x1036
#define KEY_KEYPAD_STAR                 0x1037
#define KEY_LALT                        0x1038
#define KEY_SPACE                       0x1039
#define KEY_CAPSLOCK                    0x103A
#define KEY_F1                          0x103B
#define KEY_F2                          0x103C
#define KEY_F3                          0x103D
#define KEY_F4                          0x103E
#define KEY_F5                          0x103F
#define KEY_F6                          0x1040
#define KEY_F7                          0x1041
#define KEY_F8                          0x1042
#define KEY_F9                          0x1043
#define KEY_F10                         0x1044
#define KEY_NUMLOCK                     0x1045
#define KEY_SCROLLLOCK                  0x1046
#define KEY_KEYPAD_7                    0x1047
#define KEY_KEYPAD_8                    0x1048
#define KEY_KEYPAD_9                    0x1049
#define KEY_KEYPAD_MINUS                0x104A
#define KEY_KEYPAD_4                    0x104B
#define KEY_KEYPAD_5                    0x104C
#define KEY_KEYPAD_6                    0x104D
#define KEY_KEYPAD_PLUS                 0x104E
#define KEY_KEYPAD_1                    0x104F
#define KEY_KEYPAD_2                    0x1050
#define KEY_KEYPAD_3                    0x1051
#define KEY_KEYPAD_0                    0x1052
#define KEY_KEYPAD_PERIOD               0x1053
#define KEY_F11                         0x1057
#define KEY_F12                         0x1058

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

    unsigned int layout;
    unsigned int keymap;
    unsigned int extended;
    unsigned int mod;
    unsigned int id;
    struct keycode keycode;

};

unsigned int keymap_getkeycode(struct keystate *keystate, unsigned int scancode);
void keymap_init(struct keystate *keystate, unsigned int layout, unsigned int keymap);
