#include <system.h>
#include <regs.h>
#include <mem.h>
#include <irq.h>
#include <cbuffer.h>
#include <kbd.h>

char kbdMapLowerUS[128] =
{
    0, 27,
    '1', '2', '3', '4', '5', '6', '7', '8',    '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0, 0, 0,
    0,
    0,
    0,
};

char kbdMapUpperUS[128] =
{
    0, 27,
    '!', ' ', '#', ' ', '%', '&', '/', '(',    ')', '=', '-', '=', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
    0,
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0, 0, 0,
    0,
    0,
    0,
};

kbd_device_t keyboard;

void kbd_buffer_clear(kbd_device_t *device)
{

    device->bufferSize = 0;

}

void kbd_buffer_write(kbd_device_t *device, char c)
{

    if (device->bufferSize < KBD_BUFFER_SIZE)
    {

        device->buffer[device->bufferSize] = c;
        device->bufferSize++;

    }

}

char kbd_buffer_read(kbd_device_t *device)
{

    if (device->bufferSize)
    {

        device->bufferSize--;
        return device->buffer[device->bufferSize];

    }

    return 0;

}

void kbd_handler(registers_t *r)
{

    uint8_t scancode;

    scancode = inb(0x60);

    if (scancode == 0x2A)
        keyboard.toggleShift = 1;

    if (scancode == 0xAA)
        keyboard.toggleShift = 0;

    if (scancode & 0x80)
    {

        // Break codes

    }

    else
    {

        // Make codes
        if (keyboard.toggleShift)
            kbd_buffer_write(&keyboard, kbdMapUpperUS[scancode]);
        else
            kbd_buffer_write(&keyboard, kbdMapLowerUS[scancode]);

    }

}

void kbd_init()
{

    keyboard.bufferSize = 0;
    keyboard.toggleAlt = 0;
    keyboard.toggleCtrl = 0;
    keyboard.toggleShift = 0;

    irq_register_handler(1, kbd_handler);

}

