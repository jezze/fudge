#include <lib/call.h>
#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/stack.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/elf.h>
#include <kernel/kernel.h>
#include <kernel/shell.h>

struct vfs_node shellLocationNode;
struct vfs_node shellStdoutNode;
struct vfs_node *shellVgaNode;
struct vfs_node *shellVgaColorNode;
struct vfs_node *shellVgaCursorNode;
unsigned short shellVgaCursorOffset;

char shellBuffer[SHELL_BUFFER_SIZE];
struct stack shellStack;

char shellLocation[256];

static void shell_scroll()
{

    char buffer[SHELL_CHARACTER_SIZE];

    vfs_read(shellVgaNode, SHELL_CHARACTER_WIDTH, SHELL_CHARACTER_SIZE - SHELL_CHARACTER_WIDTH, buffer);
    memory_set(buffer + SHELL_CHARACTER_SIZE - SHELL_CHARACTER_WIDTH, ' ', SHELL_CHARACTER_WIDTH);
    vfs_write(shellVgaNode, 0, SHELL_CHARACTER_SIZE, buffer);

    shellVgaCursorOffset -= SHELL_CHARACTER_WIDTH;

}

static void shell_putc(char c)
{

    if (c == '\b')
    {

        shellVgaCursorOffset--;

    }

    else if (c == '\t')
    {

        shellVgaCursorOffset = (shellVgaCursorOffset + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        shellVgaCursorOffset -= (shellVgaCursorOffset % SHELL_CHARACTER_WIDTH);

    }

    else if (c == '\n')
    {

        shellVgaCursorOffset += SHELL_CHARACTER_WIDTH - (shellVgaCursorOffset % SHELL_CHARACTER_WIDTH);

    }
    
    else if (c >= ' ')
    {

        vfs_write(shellVgaNode, shellVgaCursorOffset, 1, &c);
        shellVgaCursorOffset++;

    }

    if (shellVgaCursorOffset >= SHELL_CHARACTER_WIDTH * SHELL_CHARACTER_HEIGHT - SHELL_CHARACTER_WIDTH)
        shell_scroll();

    vfs_write(shellVgaCursorNode, 0, 1, &shellVgaCursorOffset);

}

static void shell_vga_clear()
{

    char c = ' ';
    int i;

    for (i = 0; i < SHELL_CHARACTER_SIZE; i++)
        vfs_write(shellVgaNode, i, 1, &c);

}

static void shell_clear()
{

    file_write("fudge:/$ ");
    stack_clear(&shellStack);

}

static void shell_execute_flat(unsigned int *address, int argc, char *argv[])
{

    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))address;

    func(argc, argv);

}

static void shell_execute_elf(struct vfs_node *node, struct elf_header *header, int argc, char *argv[])
{

    struct elf_program_header *pHeader = (struct elf_program_header *)(0x280000 + header->programHeaderOffset);

    vfs_read(node, pHeader->offset, pHeader->memorySize, (void *)pHeader->virtualAddress);

    shell_execute_flat((unsigned int *)pHeader->virtualAddress, argc, argv);

}

static void shell_call(struct vfs_node *node, int argc, char *argv[])
{

    char *buffer = (char *)0x280000;

    vfs_read(node, 0, node->length, buffer);

    if (elf_check((struct elf_header *)buffer))
    {

        shell_execute_elf(node, (struct elf_header *)0x280000, argc, argv);

    }

    else
    {

        file_write("Unrecognized binary format. Continue? (y/n): ");

        char c = file_read_single();

        file_write("\n");

        if (c == 'y')
        {

            shell_execute_flat((unsigned int *)0x280000, argc, argv);

        }

    }

}

static void shell_interpret(char *command)
{

    char *argv[32];
    int argc = string_split(argv, command, ' ');

    if (argc)
    {

        struct vfs_node *initrd = call_open("initrd");
        struct vfs_node *node = vfs_find(initrd, argv[0]);

        if (node)
            shell_call(node, argc, argv);

        else
        {

            file_write(argv[0]);
            file_write(": Command not found\n");

        }

    }

    shell_clear();

}

static void shell_handle_input(char c)
{

    switch (c)
    {

        case '\t':

            break;

        case '\b':

            if (stack_pop(&shellStack))
            {

                file_write_single('\b');
                file_write_single(' ');
                file_write_single('\b');

             }

            break;

        case '\n':

            stack_push(&shellStack, '\0');
            file_write_single(c);
            shell_interpret(shellBuffer);

            break;

        default:

            stack_push(&shellStack, c);
            file_write_single(c);

            break;

    }

}

static void shell_poll()
{

    for (;;)
    {

        char c = file_read_single();

        shell_handle_input(c);

    }

}

static unsigned int shell_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
        shell_putc(((char *)buffer)[j]);

    return count;

}

static unsigned int shell_location_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(shellLocation) - offset;

    string_copy(buffer, shellLocation + offset);

    return count;

}

static unsigned int shell_location_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(shellLocation) - offset;

    string_copy(shellLocation + offset, buffer);

    return count;

}

static void shell_init_vga()
{

    shellVgaNode = call_open("dev/vga_fb");
    shellVgaColorNode = call_open("dev/vga_fb_color");
    shellVgaCursorNode = call_open("dev/vga_fb_cursor");

    unsigned char color = (SHELL_COLOR_BLACK << 4) | (SHELL_COLOR_WHITE & 0x0F);
    vfs_write(shellVgaColorNode, 0, 1, &color);

    shell_vga_clear();

}

void shell_init()
{

    shell_init_vga();

    string_copy(shellLocation, "/");

    memory_set(&shellStdoutNode, 0, sizeof (struct vfs_node));
    string_copy(shellStdoutNode.name, "stdout");
    shellStdoutNode.length = SHELL_CHARACTER_SIZE;
    shellStdoutNode.write = shell_write;

    memory_set(&shellLocationNode, 0, sizeof (struct vfs_node));
    string_copy(shellLocationNode.name, "location");
    shellLocationNode.length = 256;
    shellLocationNode.read = shell_location_read;
    shellLocationNode.write = shell_location_write;

    struct vfs_node *node = call_open("dev");
    vfs_write(node, node->length, 1, &shellStdoutNode);
    vfs_write(node, node->length, 1, &shellLocationNode);

    stack_init(&shellStack, shellBuffer, SHELL_BUFFER_SIZE);

    file_write("Fudge\n");
    file_write("-----\n");
    file_write("Copyright (c) 2009 Jens Nyberg\n");
    file_write("Type 'cat help.txt' to read the help section.\n\n");

    shell_clear();
    shell_poll();

}

