#include "buffer.h"
#include "elf.h"

unsigned int elf_validate(struct elf_header *header)
{

    unsigned char id[] = {ELF_IDENTITY_MAGIC0, ELF_IDENTITY_MAGIC1, ELF_IDENTITY_MAGIC2, ELF_IDENTITY_MAGIC3};

    return buffer_match(header->identify, id, 4);

}

