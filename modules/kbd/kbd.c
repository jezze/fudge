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

static unsigned int kbd_device_getc(struct kbd_device *device, char *buffer)
{

    char c = 0;

    if (device->bufferHead != device->bufferTail)
    {

        c = device->buffer[device->bufferTail];
        device->bufferTail = (device->bufferTail + 1) % KBD_BUFFER_SIZE;

    }

    if (c)
    {

        buffer[0] = c;

        return 1;

    }

    return 0;

}

static unsigned int kbd_device_putc(struct kbd_device *device, char *buffer)
{

    if ((device->bufferHead + 1) % KBD_BUFFER_SIZE != device->bufferTail)
    {

        device->buffer[device->bufferHead] = buffer[0];
        device->bufferHead = (device->bufferHead + 1) % KBD_BUFFER_SIZE;

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
            
        kbd_device_putc(&kbdDevice, &kbdMapUS[scancode]);

    }

}

void kbd_init()
{

    string_copy(kbdDevice.base.name, "kbd");
    kbdDevice.base.module.type = MODULES_TYPE_DEVICE;
    kbdDevice.base.type = MODULES_DEVICE_TYPE_KEYBOARD;
    kbdDevice.getc = kbd_device_getc;
    kbdDevice.putc = kbd_device_putc;
    kbdDevice.bufferHead = 0;
    kbdDevice.bufferTail = 0;
    kbdDevice.escaped = 0;
    kbdDevice.toggleAlt = 0;
    kbdDevice.toggleCtrl = 0;
    kbdDevice.toggleShift = 0;

    kernel_register_irq(0x01, kbd_handler);

    modules_register_device(&kbdDevice.base);

}

