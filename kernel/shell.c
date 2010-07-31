#include <lib/elf.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/kernel.h>
#include <kernel/shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
struct stack shellStack;

static void shell_clear()
{

    arch_puts("fudge:/$ ");
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

    vfs_read(node, 0, 0x20000, buffer);

    if (buffer[0] == ELF_IDENTITY_MAGIC0)
    {

        shell_execute_elf(node, (struct elf_header *)0x280000, argc, argv);

    }

    else
    {

        arch_puts("Unrecognized binary format. Continue? (y/n): ");

        char c = arch_getc();

        arch_puts("\n");

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

    if (!argc)
    {

    }

    else if (!string_compare(argv[0], "clear"))
    {

        arch_clear();

    }

    else
    {

        struct vfs_node *node = vfs_find(kernel_get_vfs_root(), argv[0]);

        if (node)
            shell_call(node, argc, argv);

        else
        {

            arch_puts(argv[0]);
            arch_puts(": Command not found\n");

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

                arch_putc('\b');
                arch_putc(' ');
                arch_putc('\b');

             }

            break;

        case '\n':

            stack_push(&shellStack, '\0');
            arch_putc(c);
            shell_interpret(shellBuffer);

            break;

        default:

            stack_push(&shellStack, c);
            arch_putc(c);

            break;

    }

}

static void shell_poll()
{

    for (;;)
    {

        char c = arch_getc();

        shell_handle_input(c);

    }

}

void shell_init()
{

    stack_init(&shellStack, shellBuffer, SHELL_BUFFER_SIZE);

    arch_puts("Fudge\n");
    arch_puts("-----\n");
    arch_puts("Copyright (c) 2009 Jens Nyberg\n");
    arch_puts("Type 'cat help.txt' to read the help section.\n\n");

    shell_clear();
    shell_poll();

}

