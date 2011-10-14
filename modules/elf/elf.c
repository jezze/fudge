#include <lib/elf.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/elf/elf.h>

static struct elf_module elfModule;

struct elf_header *elf_get_header(void *address)
{

    struct elf_header *header = (struct elf_header *)address;

    if (header->identify[0] != ELF_IDENTITY_MAGIC0)
        return 0;

    if (header->identify[1] != ELF_IDENTITY_MAGIC1)
        return 0;

    if (header->identify[2] != ELF_IDENTITY_MAGIC2)
        return 0;

    if (header->identify[3] != ELF_IDENTITY_MAGIC3)
        return 0;

    return header;

}

struct elf_program_header *elf_get_program_header(struct elf_header *header)
{

    void *address = header;
    address += header->programHeaderOffset;

    return (struct elf_program_header *)address;

}

void elf_init()
{

}

