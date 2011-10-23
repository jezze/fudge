#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/io/io.h>
#include <modules/kbd/kbd.h>

static char kbdMapUS[256] =
{
       0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '+', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,   27,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '=', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"',  '~',    0,  '|',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  '<',  '>',  '?',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

static struct kbd_device kbdDevice;

static unsigned int kbd_buffer_getc(struct kbd_buffer *self, char *buffer)
{

    if (self->head != self->tail)
    {

        buffer[0] = self->buffer[self->tail];
        self->tail = ((self->tail + 1) % self->size);

        return 1;

    }

    return 0;

}

static unsigned int kbd_buffer_putc(struct kbd_buffer *self, char *buffer)
{

    if ((self->head + 1) != self->tail)
    {

        self->buffer[self->head] = buffer[0];
        self->head = ((self->head + 1) % self->size);

        return 1;

    }

    return 0;

}

static void kbd_handler()
{

    unsigned char scancode = io_inb(KBD_PORT_READ);

    if (kbdDevice.escaped)
    {

        //scancode += 256;
        kbdDevice.escaped = 0;

    }

    if (scancode == 0xE0)
        kbdDevice.escaped = 1;

    if (scancode == 0x38)
        kbdDevice.toggleAlt = 1;

    if (scancode == 0xB8)
        kbdDevice.toggleAlt = 0;

    if (scancode == 0x1D)
        kbdDevice.toggleCtrl = 1;

    if (scancode == 0x9D)
        kbdDevice.toggleCtrl = 0;

    if (scancode == 0x2A)
        kbdDevice.toggleShift = 1;

    if (scancode == 0xAA)
        kbdDevice.toggleShift = 0;

    if (scancode & 0x80)
    {

        // Break codes

    }

    else
    {

        if (kbdDevice.toggleShift)
            scancode += 128;

        kbdDevice.buffer.putc(&kbdDevice.buffer, &kbdMapUS[scancode]);

    }

}

void kbd_init()
{

    string_copy(kbdDevice.base.name, "kbd");
    kbdDevice.base.module.type = MODULES_TYPE_DEVICE;
    kbdDevice.base.type = MODULES_DEVICE_TYPE_KEYBOARD;
    kbdDevice.buffer.size = 256;
    kbdDevice.buffer.head = 0;
    kbdDevice.buffer.tail = 0;
    kbdDevice.buffer.getc = kbd_buffer_getc;
    kbdDevice.buffer.putc = kbd_buffer_putc;
    kbdDevice.escaped = 0;
    kbdDevice.toggleAlt = 0;
    kbdDevice.toggleCtrl = 0;
    kbdDevice.toggleShift = 0;

    kernel_register_irq(0x01, kbd_handler);

    modules_register_device(&kbdDevice.base);

}

