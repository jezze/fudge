#include <abi.h>
#include <fudge.h>
#include <keymap/keymap.h>

static unsigned int escaped;
static unsigned int modifier;

static struct keycode *scancode2keycode(struct keymap *map, unsigned char scancode)
{

    switch (scancode)
    {

    case 0xE0:
        escaped = 1;

        return 0;

    case 0x2A:
    case 0x36:
        modifier |= KEYMOD_SHIFT;

        return 0;

    case 0x38:
        modifier |= KEYMOD_ALT;

        return 0;

    case 0xAA:
    case 0xB6:
        modifier &= ~KEYMOD_SHIFT;

        return 0;

    case 0xB8:
        modifier &= ~KEYMOD_ALT;

        return 0;

    }

    if (scancode & 0x80)
        return 0;

    return keymap_getkeycode(map, scancode, modifier);

}

void main(void)
{

    struct keymap *map = keymap_load(KEYMAP_SE);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    escaped = 0;
    modifier = 0;

    file_open(FILE_PO);
    file_open(FILE_PI);

    while ((count = file_read(FILE_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            struct keycode *keycode = scancode2keycode(map, buffer[i]);

            if (!keycode)
                continue;

            if (keycode->length)
                file_writeall(FILE_PO, keycode->value, keycode->length);

        }

    }

    file_close(FILE_PI);
    file_close(FILE_PO);

}

