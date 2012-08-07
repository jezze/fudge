#include <elf.h>
#include <runtime.h>
#include <binary.h>
#include <modules.h>

unsigned int binary_find_symbol(struct runtime_descriptor *descriptor, char *func)
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    struct elf_section_header *symbolHeader;
    struct elf_symbol symbolTable[400];
    char stringTable[0x1000];

    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, 0, sizeof (struct elf_header), &header);

    if (!elf_validate(&header))
        return 0;

    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, header.shoffset, header.shsize * header.shcount, sectionHeader);

    symbolHeader = elf_get_section(&header, sectionHeader, ELF_SECTION_TYPE_SYMTAB);

    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, symbolHeader->offset, symbolHeader->size, symbolTable);
    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, sectionHeader[symbolHeader->link].offset, sectionHeader[symbolHeader->link].size, stringTable);

    return elf_find_symbol(&header, sectionHeader, symbolHeader, symbolTable, stringTable, func);

}

unsigned int binary_get_entry(struct runtime_descriptor *descriptor)
{

    struct elf_header header;

    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, 0, sizeof (struct elf_header), &header);

    if (!elf_validate(&header))
        return 0;

    return header.entry;

}

unsigned int binary_get_vaddress(struct runtime_descriptor *descriptor)
{

    struct elf_header header;
    struct elf_program_header programHeader;

    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, 0, sizeof (struct elf_header), &header);

    if (!elf_validate(&header))
        return 0;

    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, header.phoffset, sizeof (struct elf_program_header), &programHeader);

    return programHeader.vaddress;

}

static void binary_relocate_section(struct elf_section_header *sectionHeader, struct elf_section_header *relocateHeader, struct elf_section_header *relocateData, struct elf_relocate *relocateTable, struct elf_section_header *symbolHeader, struct elf_symbol *symbolTable, unsigned int address)
{

    unsigned int i;

    for (i = 0; i < relocateHeader->size / relocateHeader->esize; i++)
    {

        struct elf_relocate *relocateEntry = &relocateTable[i];
        unsigned char type = relocateEntry->info & 0x0F;
        unsigned char index = relocateEntry->info >> 8;
        struct elf_symbol *symbolEntry = &symbolTable[index];
        unsigned int *entry = (unsigned int *)(address + relocateData->offset + relocateEntry->offset);
        unsigned int value = *entry;
        unsigned int addend = (symbolEntry->shindex) ? address + sectionHeader[symbolEntry->shindex].offset + symbolEntry->value : 0;

        switch (type)
        {

            case 1:

                *entry = value + addend;

                break;

            case 2:

                *entry = value + addend - (unsigned int)entry;

                break;

        }

    }

}

void binary_relocate(struct runtime_descriptor *descriptor, unsigned int address)
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    unsigned int i;

    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, 0, sizeof (struct elf_header), &header);

    if (!elf_validate(&header))
        return;

    descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, header.shoffset, header.shsize * header.shcount, sectionHeader);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_relocate relocateTable[200];
        struct elf_symbol symbolTable[400];

        if (sectionHeader[i].type != ELF_SECTION_TYPE_REL)
            continue;

        descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, sectionHeader[i].offset, sectionHeader[i].size, relocateTable);
        descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, sectionHeader[sectionHeader[i].link].offset, sectionHeader[sectionHeader[i].link].size, symbolTable);

        binary_relocate_section(sectionHeader, &sectionHeader[i], &sectionHeader[sectionHeader[i].info], relocateTable, &sectionHeader[sectionHeader[i].link], symbolTable, address);

        sectionHeader[sectionHeader[i].info].address += address;

        descriptor->mount->filesystem->write(descriptor->mount->filesystem, descriptor->id, header.shoffset, header.shsize * header.shcount, sectionHeader);

    }

}

