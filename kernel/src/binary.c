#include <elf.h>
#include <modules.h>
#include <binary.h>

unsigned int binary_find_symbol(struct modules_filesystem *filesystem, unsigned int id, char *func)
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    struct elf_section_header *symbolHeader;
    struct elf_symbol symbolTable[400];
    char stringTable[0x1000];

    filesystem->read(filesystem, id, 0, sizeof (struct elf_header), &header);

    if (!elf_validate(&header))
        return 0;

    filesystem->read(filesystem, id, header.shoffset, header.shsize * header.shcount, sectionHeader);

    symbolHeader = elf_get_section(&header, sectionHeader, ELF_SECTION_TYPE_SYMTAB);

    filesystem->read(filesystem, id, symbolHeader->offset, symbolHeader->size, symbolTable);
    filesystem->read(filesystem, id, sectionHeader[symbolHeader->link].offset, sectionHeader[symbolHeader->link].size, stringTable);

    return elf_find_symbol(&header, sectionHeader, symbolHeader, symbolTable, stringTable, func);

}

unsigned int binary_get_entry(struct modules_filesystem *filesystem, unsigned int id)
{

    struct elf_header header;

    filesystem->read(filesystem, id, 0, sizeof (struct elf_header), &header);

    if (!elf_validate(&header))
        return 0;

    return header.entry;

}

unsigned int binary_copy_program(struct modules_filesystem *filesystem, unsigned int id)
{

    struct elf_header header;
    struct elf_program_header programHeader[8];
    unsigned int i;

    filesystem->read(filesystem, id, 0, sizeof (struct elf_header), &header);

    if (!elf_validate(&header))
        return 0;

    filesystem->read(filesystem, id, header.phoffset, header.phsize * header.phcount, programHeader);

    for (i = 0; i < header.phcount; i++)
        filesystem->read(filesystem, id, programHeader[i].offset, programHeader[i].fsize, (void *)programHeader[i].vaddress);

    return 1;

}

static void binary_relocate_section(struct elf_section_header *sectionHeader, struct elf_section_header *relocateHeader, struct elf_section_header *relocateData, struct elf_relocate *relocateTable, struct elf_symbol *symbolTable, unsigned int address)
{

    unsigned int i;

    for (i = 0; i < relocateHeader->size / relocateHeader->esize; i++)
    {

        unsigned char type = relocateTable[i].info & 0x0F;
        unsigned char index = relocateTable[i].info >> 8;
        unsigned int offset = address + relocateData->offset + relocateTable[i].offset;
        unsigned int addend = (symbolTable[index].shindex) ? address + sectionHeader[symbolTable[index].shindex].offset + symbolTable[index].value : 0;
        unsigned int *entry = (unsigned int *)(offset);

        switch (type)
        {

            case 1:

                *entry += addend;

                break;

            case 2:

                *entry += addend - offset;

                break;

        }

    }

}

unsigned int binary_relocate(struct modules_filesystem *filesystem, unsigned int id, unsigned int address)
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    unsigned int i;

    filesystem->read(filesystem, id, 0, sizeof (struct elf_header), &header);

    if (!elf_validate(&header))
        return 0;

    filesystem->read(filesystem, id, header.shoffset, header.shsize * header.shcount, sectionHeader);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_relocate relocateTable[200];
        struct elf_symbol symbolTable[400];

        if (sectionHeader[i].type != ELF_SECTION_TYPE_REL)
            continue;

        filesystem->read(filesystem, id, sectionHeader[i].offset, sectionHeader[i].size, relocateTable);
        filesystem->read(filesystem, id, sectionHeader[sectionHeader[i].link].offset, sectionHeader[sectionHeader[i].link].size, symbolTable);

        binary_relocate_section(sectionHeader, &sectionHeader[i], &sectionHeader[sectionHeader[i].info], relocateTable, symbolTable, address);

        sectionHeader[sectionHeader[i].info].address += address;

        filesystem->write(filesystem, id, header.shoffset, header.shsize * header.shcount, sectionHeader);

    }

    return 1;

}

