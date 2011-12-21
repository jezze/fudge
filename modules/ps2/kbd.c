#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/ps2/ps2.h>

static char mapUS[256] =
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

static struct kbd_device device;

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

    if ((self->head + 1) % self->size != self->tail)
    {

        self->buffer[self->head] = buffer[0];
        self->head = ((self->head + 1) % self->size);

        return 1;

    }

    return 0;

}

void kbd_device_init(struct kbd_device *device)
{

    modules_device_init(&device->base, KBD_DEVICE_TYPE);
    device->buffer.size = 256;
    device->buffer.head = 0;
    device->buffer.tail = 0;
    device->buffer.getc = kbd_buffer_getc;
    device->buffer.putc = kbd_buffer_putc;
    device->escaped = 0;
    device->toggleAlt = 0;
    device->toggleCtrl = 0;
    device->toggleShift = 0;


}

static void handle_irq()
{

    struct kbd_device *kbd = &device;

    unsigned char scancode = io_inb(KBD_PORT_READ);

    if (kbd->escaped)
    {

        //scancode += 256;
        kbd->escaped = 0;

    }

    if (scancode == 0xE0)
        kbd->escaped = 1;

    if (scancode == 0x38)
        kbd->toggleAlt = 1;

    if (scancode == 0xB8)
        kbd->toggleAlt = 0;

    if (scancode == 0x1D)
        kbd->toggleCtrl = 1;

    if (scancode == 0x9D)
        kbd->toggleCtrl = 0;

    if (scancode == 0x2A)
        kbd->toggleShift = 1;

    if (scancode == 0xAA)
        kbd->toggleShift = 0;

    if (scancode & 0x80)
    {

        // Break codes

    }

    else
    {

        if (kbd->toggleShift)
            scancode += 128;

        kbd->buffer.putc(&kbd->buffer, &mapUS[scancode]);

    }

    event_handle(EVENT_IRQ_KBD);

}

void init()
{

    kbd_device_init(&device);

    kernel_register_irq(0x01, handle_irq);

    modules_register_device(&device.base);

}

void destroy()
{

    kernel_unregister_irq(0x01);

    modules_unregister_device(&device.base);

}

