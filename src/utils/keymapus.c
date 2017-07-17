#include <abi.h>
#include <fudge.h>
#include <keymap/keymap.h>

static unsigned int modifier;

static struct keycode *scancode2keycode(struct keymap *map, unsigned char scancode)
{

    modifier = keymap_modkey(scancode, modifier);

    if (scancode == 0xE0)
        return 0;

    if (scancode & 0x80)
        return 0;

    return keymap_getkeycode(map, scancode, modifier);

}

void main(void)
{

    struct keymap *map = keymap_load(KEYMAP_US);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

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

