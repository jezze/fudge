#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/irq.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/mmu.h>
#include <arch/x86/modules/kbd/kbd.h>
#include <kernel/vfs.h>

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

struct kbd_device kbdDevice;

void kbd_cbuffer_write(struct kbd_device *device, char c)
{

    if ((device->bufferHead + 1) % KBD_BUFFER_SIZE != device->bufferTail)
    {

        device->buffer[device->bufferHead] = c;
        device->bufferHead = (device->bufferHead + 1) % KBD_BUFFER_SIZE;

    }

}

char kbd_cbuffer_read(struct kbd_device *device)
{

    if (device->bufferHead != device->bufferTail)
    {

        char c = device->buffer[device->bufferTail];
        device->bufferTail = (device->bufferTail + 1) % KBD_BUFFER_SIZE;

        return c;

    }

    return 0;

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
            kbd_cbuffer_write(&kbdDevice, kbdMapUpperUS[scancode]);
        else
            kbd_cbuffer_write(&kbdDevice, kbdMapLowerUS[scancode]);

    }

}

static unsigned int kbd_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

/*
    struct mmu_directory *directory = mmu_get_kernel_directory();
    unsigned int virtual = *(unsigned int *)buffer;

    file_write_hex(session_get_out(), virtual);

    virtual = virtual + (virtual % 4096);

    file_write_hex(session_get_out(), virtual);

    unsigned int physical = mmu_get_physical_address(directory, virtual);

    file_write_hex(session_get_out(), physical);

*/

    char c = kbd_cbuffer_read(&kbdDevice);

    if (c)
    {

        memory_copy(buffer, &c, 1);

        return 1;

    }

    return 0;

}

void kbd_init()
{

    kbdDevice.bufferHead = 0;
    kbdDevice.bufferTail = 0;
    kbdDevice.toggleAlt = 0;
    kbdDevice.toggleCtrl = 0;
    kbdDevice.toggleShift = 0;

    irq_register_handler(IRQ_ROUTINE_KBD, kbd_handler);

    struct vfs_node *kbdNode = vfs_add_node("kbd", 0);
    kbdNode->read = kbd_read;

    struct vfs_node *devNode = call_open("/dev");
    file_write(devNode, devNode->length, 1, kbdNode);

}

