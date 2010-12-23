#include <lib/elf.h>
#include <modules/elf/elf.h>

void elf_execute(struct elf_header *header, int argc, char *argv[])
{

}

unsigned int elf_check(unsigned int address)
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

    return 1;

}

