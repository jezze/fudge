#include <lib/cbuffer.h>
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

static void shell_execute_elf(struct elf_header *header, int argc, char *argv[])
{

    //unsigned char identify[16];

    arch_puts("ELF header");
    arch_puts("\nType: ");
    arch_puts_dec(header->type);
    arch_puts("\nMachine: ");
    arch_puts_dec(header->machine);
    arch_puts("\nVersion: ");
    arch_puts_dec(header->version);
    arch_puts("\nEntry: 0x");
    arch_puts_hex(header->entry);
    arch_puts("\nProgram Header Offset: ");
    arch_puts_dec(header->programHeaderOffset);
    arch_puts("\nSection Header Offset: ");
    arch_puts_dec(header->sectionHeaderOffset);
    arch_puts("\nFlags: ");
    arch_puts_dec(header->flags);
    arch_puts("\nHeader Size: ");
    arch_puts_dec(header->headerSize);
    arch_puts("\nProgram Header Entry Size: ");
    arch_puts_dec(header->programHeaderEntrySize);
    arch_puts("\nProgram Header Count: ");
    arch_puts_dec(header->programHeaderCount);
    arch_puts("\nSection Header Entry Size: ");
    arch_puts_dec(header->sectionHeaderEntrySize);
    arch_puts("\nSection Header Count: ");
    arch_puts_dec(header->sectionHeaderCount);
    arch_puts("\nSection Header String Index: ");
    arch_puts_dec(header->sectionHeaderStringIndex);
    arch_puts("\n\n");

    struct elf_program_header *pHeader = (struct elf_program_header *)(header->entry + header->programHeaderOffset);

    arch_puts("ELF program header:");
    arch_puts("\nType: ");
    arch_puts_dec(pHeader->type);
    arch_puts("\nOffset: ");
    arch_puts_dec(pHeader->offset);
    arch_puts("\nVirtual address: 0x");
    arch_puts_hex(pHeader->virtualAddress);
    arch_puts("\nPhysical address: 0x");
    arch_puts_hex(pHeader->physicalAddress);
    arch_puts("\nFile size: ");
    arch_puts_dec(pHeader->fileSize);
    arch_puts("\nMemory size: ");
    arch_puts_dec(pHeader->memorySize);
    arch_puts("\nFlags: ");
    arch_puts_dec(pHeader->flags);
    arch_puts("\nAlign: ");
    arch_puts_dec(pHeader->align);
    arch_puts("\n");

}

static void shell_execute_flat(unsigned int *address, int argc, char *argv[])
{

    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))address;

    func(argc, argv);

}

static void shell_call(vfs_node_t *node, int argc, char *argv[])
{

    char *buffer = (char *)0x200000;

    vfs_read(node, 0, 5000, buffer);

    if (buffer[0] == ELF_IDENTITY_MAGIC0)
        shell_execute_elf((struct elf_header *)0x200000, argc, argv);
    else
        shell_execute_flat((unsigned int *)0x200000, argc, argv);

}

static int shell_get_arguments(char *argv[], char *command)
{

    int argc = 0;
    int length = string_length(command);
    char *base = command;

    int i = 0;

    for (i = 0; i < length; i++)
    {

        if (command[i] == ' ')
        {

            argv[argc] = base;
            argc++;

            command[i] = 0;
            base = &command[i + 1];

        }

    }

    argv[argc] = base;
    argc++;

    return argc;

}

static void shell_interpret(char *command)
{

    char *argv[32];
    int argc = shell_get_arguments(argv, command);

    if (!string_compare(argv[0], ""))
    {

    }

    else if (!string_compare(argv[0], "clear"))
    {

        arch_clear();

    }

    else
    {

        vfs_node_t *node = vfs_find(fsRoot, argv[0]);

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

