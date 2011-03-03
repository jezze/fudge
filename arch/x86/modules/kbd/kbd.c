#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <arch/x86/kernel/irq.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/mmu.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/kbd/kbd.h>

char kbdMapLowerUS[128] =
{
       0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '+', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

char kbdMapUpperUS[128] =
{
       0,   27,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '=', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"',  '~',    0,  '|',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  '<',  '>',  '?',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

struct modules_kbd_device kbdDevice;

static unsigned int kbd_device_read(char *buffer)
{

    char c = 0;

    if (kbdDevice.bufferHead != kbdDevice.bufferTail)
    {

        c = kbdDevice.buffer[kbdDevice.bufferTail];
        kbdDevice.bufferTail = (kbdDevice.bufferTail + 1) % KBD_BUFFER_SIZE;

    }

    if (c)
    {

        memory_copy(buffer, &c, 1);

        return 1;

    }

    return 0;

}

static unsigned int kbd_device_write(char *buffer)
{

    if ((kbdDevice.bufferHead + 1) % KBD_BUFFER_SIZE != kbdDevice.bufferTail)
    {

        kbdDevice.buffer[kbdDevice.bufferHead] = buffer[0];
        kbdDevice.bufferHead = (kbdDevice.bufferHead + 1) % KBD_BUFFER_SIZE;

        return 1;

    }

    return 0;

}

static unsigned int kbd_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return kbdDevice.read(buffer);

}

void kbd_handler(struct isr_registers *registers)
{

    unsigned char scancode = io_inb(KBD_PORT_READ);

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

        // Make codes
        if (kbdDevice.toggleShift)
            kbdDevice.write(&kbdMapUpperUS[scancode]);
        else
            kbdDevice.write(&kbdMapLowerUS[scancode]);

    }

}

void kbd_init()
{

    kbdDevice.bufferHead = 0;
    kbdDevice.bufferTail = 0;
    kbdDevice.toggleAlt = 0;
    kbdDevice.toggleCtrl = 0;
    kbdDevice.toggleShift = 0;
    kbdDevice.read = kbd_device_read;
    kbdDevice.write = kbd_device_write;

    irq_register_handler(IRQ_ROUTINE_KBD, kbd_handler);

    modules_register_module(&kbdDevice.base);

    struct vfs_node *kbdNode = vfs_add_node("kbd", 0);
    kbdNode->read = kbd_node_read;

    struct vfs_node *devNode = call_open("/dev");
    file_write(devNode, devNode->length, 1, kbdNode);

}

