#include <lib/call.h>
#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/irq.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/kbd.h>

char kbd_mapLowerUS[128] =
{
       0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/',    0,  '*',    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  '-',    0,    0,    0,  '+',    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

char kbd_mapUpperUS[128] =
{
       0,   27,  '!',  ' ',  '#',  ' ',  '%',  '&',  '/',  '(',  ')',  '=',  '-',  '=', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '[',  ']', '\n',    0,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ';', '\'',  '`',    0, '\\',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  ',',  '.',  '/',    0,  '*',    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  '-',    0,    0,    0,  '+',    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

struct vfs_node kbd_node;
struct kbd_device keyboard;

void kbd_handler(struct isr_registers *registers)
{

    unsigned char scancode = io_inb(KBD_PORT_READ);

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
            cbuffer_write(&keyboard.cbuffer, kbd_mapUpperUS[scancode]);
        else
            cbuffer_write(&keyboard.cbuffer, kbd_mapLowerUS[scancode]);

    }

}

static unsigned int kbd_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    char c = cbuffer_read(&keyboard.cbuffer);

    if (c)
    {

        memory_copy(buffer, &c, 1);
        return 1;

    }

    return 0;

}

void kbd_init()
{

    cbuffer_init(&keyboard.cbuffer, keyboard.buffer, KBD_BUFFER_SIZE);
    keyboard.toggleAlt = 0;
    keyboard.toggleCtrl = 0;
    keyboard.toggleShift = 0;

    memory_set(&kbd_node, 0, sizeof (struct vfs_node));
    string_copy(kbd_node.name, "stdin");
    kbd_node.read = kbd_read;

    struct vfs_node *node = call_vfs_find("dev");
    vfs_write(node, node->length, 1, &kbd_node);

}

