#include "buffer.h"
#include "keymap.h"

static unsigned int qwerty_us[128] = {
    0,
    KEY_ESC,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LBRACKET,
    KEY_RBRACKET,
    KEY_ENTER,
    KEY_LCONTROL,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_SINGLEQUOTE,
    KEY_BACKTICK,
    KEY_LSHIFT,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_RSHIFT,
    KEY_KEYPAD_STAR,
    KEY_LALT,
    KEY_SPACE,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_NUMLOCK,
    KEY_SCROLLLOCK,
    KEY_KEYPAD_7,
    KEY_KEYPAD_8,
    KEY_KEYPAD_9,
    KEY_KEYPAD_MINUS,
    KEY_KEYPAD_4,
    KEY_KEYPAD_5,
    KEY_KEYPAD_6,
    KEY_KEYPAD_PLUS,
    KEY_KEYPAD_1,
    KEY_KEYPAD_2,
    KEY_KEYPAD_3,
    KEY_KEYPAD_0,
    KEY_KEYPAD_PERIOD,
    0,
    0,
    0,
    KEY_F11,
    KEY_F12,
    0
};

static unsigned int qwerty_us_extended[128];

static struct keymap map_us[256] = {
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x31, 0x00, 0x00, 0x00}}, {1, {0x21, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x32, 0x00, 0x00, 0x00}}, {1, {0x40, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x33, 0x00, 0x00, 0x00}}, {1, {0x23, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x34, 0x00, 0x00, 0x00}}, {1, {0x24, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x35, 0x00, 0x00, 0x00}}, {1, {0x25, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x36, 0x00, 0x00, 0x00}}, {1, {0x5E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x37, 0x00, 0x00, 0x00}}, {1, {0x26, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x38, 0x00, 0x00, 0x00}}, {1, {0x2A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x39, 0x00, 0x00, 0x00}}, {1, {0x28, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x30, 0x00, 0x00, 0x00}}, {1, {0x29, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2D, 0x00, 0x00, 0x00}}, {1, {0x5F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x3D, 0x00, 0x00, 0x00}}, {1, {0x2B, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x08, 0x00, 0x00, 0x00}}, {1, {0x08, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x09, 0x00, 0x00, 0x00}}, {1, {0x09, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x71, 0x00, 0x00, 0x00}}, {1, {0x51, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x77, 0x00, 0x00, 0x00}}, {1, {0x57, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x65, 0x00, 0x00, 0x00}}, {1, {0x45, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x72, 0x00, 0x00, 0x00}}, {1, {0x52, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x74, 0x00, 0x00, 0x00}}, {1, {0x54, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x79, 0x00, 0x00, 0x00}}, {1, {0x59, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x75, 0x00, 0x00, 0x00}}, {1, {0x55, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x69, 0x00, 0x00, 0x00}}, {1, {0x49, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6F, 0x00, 0x00, 0x00}}, {1, {0x4F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x70, 0x00, 0x00, 0x00}}, {1, {0x50, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5B, 0x00, 0x00, 0x00}}, {1, {0x7B, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5D, 0x00, 0x00, 0x00}}, {1, {0x7D, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x0A, 0x00, 0x00, 0x00}}, {1, {0x0A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x61, 0x00, 0x00, 0x00}}, {1, {0x41, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x73, 0x00, 0x00, 0x00}}, {1, {0x53, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x64, 0x00, 0x00, 0x00}}, {1, {0x44, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x66, 0x00, 0x00, 0x00}}, {1, {0x46, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x67, 0x00, 0x00, 0x00}}, {1, {0x47, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x68, 0x00, 0x00, 0x00}}, {1, {0x48, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6A, 0x00, 0x00, 0x00}}, {1, {0x4A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6B, 0x00, 0x00, 0x00}}, {1, {0x4B, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6C, 0x00, 0x00, 0x00}}, {1, {0x4C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x3B, 0x00, 0x00, 0x00}}, {1, {0x3A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x27, 0x00, 0x00, 0x00}}, {1, {0x22, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x60, 0x00, 0x00, 0x00}}, {1, {0x7E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5C, 0x00, 0x00, 0x00}}, {1, {0x7C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x7A, 0x00, 0x00, 0x00}}, {1, {0x5A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x78, 0x00, 0x00, 0x00}}, {1, {0x58, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x63, 0x00, 0x00, 0x00}}, {1, {0x43, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x76, 0x00, 0x00, 0x00}}, {1, {0x56, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x62, 0x00, 0x00, 0x00}}, {1, {0x42, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6E, 0x00, 0x00, 0x00}}, {1, {0x4E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6D, 0x00, 0x00, 0x00}}, {1, {0x4D, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2C, 0x00, 0x00, 0x00}}, {1, {0x3C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2E, 0x00, 0x00, 0x00}}, {1, {0x3E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2F, 0x00, 0x00, 0x00}}, {1, {0x3F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x20, 0x00, 0x00, 0x00}}, {1, {0x20, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
};

static struct keymap map_se[256] = {
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x31, 0x00, 0x00, 0x00}}, {1, {0x21, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x32, 0x00, 0x00, 0x00}}, {1, {0x40, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x33, 0x00, 0x00, 0x00}}, {1, {0x23, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x34, 0x00, 0x00, 0x00}}, {1, {0x24, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x35, 0x00, 0x00, 0x00}}, {1, {0x25, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x36, 0x00, 0x00, 0x00}}, {1, {0x5E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x37, 0x00, 0x00, 0x00}}, {1, {0x26, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x38, 0x00, 0x00, 0x00}}, {1, {0x2A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x39, 0x00, 0x00, 0x00}}, {1, {0x28, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x30, 0x00, 0x00, 0x00}}, {1, {0x29, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2D, 0x00, 0x00, 0x00}}, {1, {0x5F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x3D, 0x00, 0x00, 0x00}}, {1, {0x2B, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x08, 0x00, 0x00, 0x00}}, {1, {0x08, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x09, 0x00, 0x00, 0x00}}, {1, {0x09, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x71, 0x00, 0x00, 0x00}}, {1, {0x51, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x77, 0x00, 0x00, 0x00}}, {1, {0x57, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x65, 0x00, 0x00, 0x00}}, {1, {0x45, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x72, 0x00, 0x00, 0x00}}, {1, {0x52, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x74, 0x00, 0x00, 0x00}}, {1, {0x54, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x79, 0x00, 0x00, 0x00}}, {1, {0x59, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x75, 0x00, 0x00, 0x00}}, {1, {0x55, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x69, 0x00, 0x00, 0x00}}, {1, {0x49, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6F, 0x00, 0x00, 0x00}}, {1, {0x4F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x70, 0x00, 0x00, 0x00}}, {1, {0x50, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5B, 0x00, 0x00, 0x00}}, {1, {0x7B, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5D, 0x00, 0x00, 0x00}}, {1, {0x7D, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x0A, 0x00, 0x00, 0x00}}, {1, {0x0A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x61, 0x00, 0x00, 0x00}}, {1, {0x41, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x73, 0x00, 0x00, 0x00}}, {1, {0x53, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x64, 0x00, 0x00, 0x00}}, {1, {0x44, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x66, 0x00, 0x00, 0x00}}, {1, {0x46, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x67, 0x00, 0x00, 0x00}}, {1, {0x47, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x68, 0x00, 0x00, 0x00}}, {1, {0x48, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6A, 0x00, 0x00, 0x00}}, {1, {0x4A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6B, 0x00, 0x00, 0x00}}, {1, {0x4B, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6C, 0x00, 0x00, 0x00}}, {1, {0x4C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x3B, 0x00, 0x00, 0x00}}, {1, {0x3A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x27, 0x00, 0x00, 0x00}}, {1, {0x22, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x60, 0x00, 0x00, 0x00}}, {1, {0x7E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5C, 0x00, 0x00, 0x00}}, {1, {0x7C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x7A, 0x00, 0x00, 0x00}}, {1, {0x5A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x78, 0x00, 0x00, 0x00}}, {1, {0x58, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x63, 0x00, 0x00, 0x00}}, {1, {0x43, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x76, 0x00, 0x00, 0x00}}, {1, {0x56, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x62, 0x00, 0x00, 0x00}}, {1, {0x42, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6E, 0x00, 0x00, 0x00}}, {1, {0x4E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6D, 0x00, 0x00, 0x00}}, {1, {0x4D, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2C, 0x00, 0x00, 0x00}}, {1, {0x3C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2E, 0x00, 0x00, 0x00}}, {1, {0x3E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2F, 0x00, 0x00, 0x00}}, {1, {0x3F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x20, 0x00, 0x00, 0x00}}, {1, {0x20, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
};

static unsigned int *getlayout(unsigned int type, unsigned int extended)
{

    switch (type)
    {

    case KEYMAP_LAYOUT_QWERTY_US:
        return (extended) ? qwerty_us_extended : qwerty_us;

    }

    return 0;

}

static struct keymap *getkeymap(unsigned int type)
{

    switch (type)
    {

    case KEYMAP_US:
        return map_us;

    case KEYMAP_SE:
        return map_se;

    }

    return 0;

}

static void update(struct keystate *keystate, unsigned int type, unsigned int offset, unsigned int release)
{

    unsigned int *layout = getlayout(type, keystate->extended);

    keystate->id = (layout) ? layout[offset] : 0;

    if (release)
    {

        switch (keystate->id)
        {

        case KEY_LCONTROL:
            keystate->mod &= ~KEYMOD_CTRL;

            break;

        case KEY_LSHIFT:
        case KEY_RSHIFT:
            keystate->mod &= ~KEYMOD_SHIFT;

            break;

        case KEY_LALT:
            keystate->mod &= ~KEYMOD_ALT;

            break;

        }

    }

    else
    {

        switch (keystate->id)
        {

        case KEY_LCONTROL:
            keystate->mod |= KEYMOD_CTRL;

            break;

        case KEY_LSHIFT:
        case KEY_RSHIFT:
            keystate->mod |= KEYMOD_SHIFT;

            break;

        case KEY_LALT:
            keystate->mod |= KEYMOD_ALT;

            break;

        }

    }

}

unsigned int keymap_getkeycode(struct keystate *keystate, unsigned int scancode)
{

    if (scancode == 0xE0)
    {

        keystate->extended = !keystate->extended;
        keystate->id = 0;
        keystate->keycode.length = 0;

    }

    else
    {

        unsigned int offset = scancode & 0x7F;
        unsigned int release = scancode & 0x80;

        update(keystate, keystate->layout, offset, release);

        if (offset)
        {

            struct keymap *keymap = getkeymap(keystate->layout);

            if (keymap)
            {

                struct keycode *code = &keymap[offset].keycode[(keystate->mod & KEYMOD_SHIFT) ? 1 : 0];

                buffer_copy(&keystate->keycode, code, sizeof (struct keycode));

            }

        }

    }

    return keystate->id;

}

void keymap_init(struct keystate *keystate, unsigned int layout, unsigned int keymap)
{

    keystate->layout = layout;
    keystate->keymap = keymap;
    keystate->extended = 0;
    keystate->mod = 0;
    keystate->id = 0;
    keystate->keycode.length = 0;

}

