#include <fudge.h>

static struct elf_header *h;
static struct elf_section_headers *sh[32];
static struct elf_symbol *sym[64];

void write_header_type(unsigned int type)
{

    switch (type)
    {

        case ELF_TYPE_NONE:

            file_write_format(FILE_STDOUT, "none");

            break;

        case ELF_TYPE_RELOCATABLE:

            file_write_format(FILE_STDOUT, "relocatable");

            break;

        case ELF_TYPE_EXECUTABLE:

            file_write_format(FILE_STDOUT, "executable");

            break;

        case ELF_TYPE_DYNAMIC:

            file_write_format(FILE_STDOUT, "dynamic");

            break;

        case ELF_TYPE_CORE:

            file_write_format(FILE_STDOUT, "core");

            break;

        case ELF_TYPE_LOPROC:

            file_write_format(FILE_STDOUT, "loproc");

            break;

        case ELF_TYPE_HIPROC:

            file_write_format(FILE_STDOUT, "hiproc");

            break;

        default:

            file_write_format(FILE_STDOUT, "unknown");

            break;

    }

}

void write_header(struct elf_header *header)
{

    file_write_format(FILE_STDOUT, "Type: %d\n", header->type);
    file_write_format(FILE_STDOUT, "Machine: %d\n", header->machine);
    file_write_format(FILE_STDOUT, "Version: %d\n", header->version);
    file_write_format(FILE_STDOUT, "Entry: 0x%x\n", header->entry);
    file_write_format(FILE_STDOUT, "Flags: %d\n", header->flags);
    file_write_format(FILE_STDOUT, "Size: %d\n", header->size);
    file_write_format(FILE_STDOUT, "Program header: {o:%d s:%d c:%d}\n", header->programHeaderOffset, header->programHeaderSize, header->programHeaderCount);
    file_write_format(FILE_STDOUT, "Section header: {o:%d s:%d c:%d si:%d}\n", header->sectionHeaderOffset, header->sectionHeaderSize, header->sectionHeaderCount, header->sectionHeaderStringIndex);

}

void write_program_header(struct elf_program_header *header)
{

    file_write_format(FILE_STDOUT, "Type: %d\n", header->type);
    file_write_format(FILE_STDOUT, "Offset: %d\n", header->offset);
    file_write_format(FILE_STDOUT, "Virtual address: 0x%x\n", header->virtualAddress);
    file_write_format(FILE_STDOUT, "Physical address: 0x%x\n", header->physicalAddress);
    file_write_format(FILE_STDOUT, "File size: %d\n", header->fileSize);
    file_write_format(FILE_STDOUT, "Memory size: %d\n", header->memorySize);
    file_write_format(FILE_STDOUT, "Flags: %d\n", header->flags);
    file_write_format(FILE_STDOUT, "Align: %d\n", header->align);

}

void write_section_header_type(unsigned int type)
{

    switch (type)
    {

        case ELF_SECTION_TYPE_NULL:

            file_write_format(FILE_STDOUT, "null");

            break;

        case ELF_SECTION_TYPE_PROGBITS:

            file_write_format(FILE_STDOUT, "progbits");

            break;

        case ELF_SECTION_TYPE_SYMTAB:

            file_write_format(FILE_STDOUT, "symtab");

            break;

        case ELF_SECTION_TYPE_STRTAB:

            file_write_format(FILE_STDOUT, "strtab");

            break;

        case ELF_SECTION_TYPE_RELA:

            file_write_format(FILE_STDOUT, "rela");

            break;

        case ELF_SECTION_TYPE_HASH:

            file_write_format(FILE_STDOUT, "hash");

            break;

        case ELF_SECTION_TYPE_DYNAMIC:

            file_write_format(FILE_STDOUT, "dynamic");

            break;

        case ELF_SECTION_TYPE_NOTE:

            file_write_format(FILE_STDOUT, "note");

            break;

        case ELF_SECTION_TYPE_NOBITS:

            file_write_format(FILE_STDOUT, "nobits");

            break;

        case ELF_SECTION_TYPE_REL:

            file_write_format(FILE_STDOUT, "rel");

            break;

        case ELF_SECTION_TYPE_SHLIB:

            file_write_format(FILE_STDOUT, "shlib");

            break;

        case ELF_SECTION_TYPE_DYNSYM:

            file_write_format(FILE_STDOUT, "dynsym");

            break;

        case ELF_SECTION_TYPE_LOPROC:

            file_write_format(FILE_STDOUT, "loproc");

            break;

        case ELF_SECTION_TYPE_HIPROC:

            file_write_format(FILE_STDOUT, "hiproc");

            break;

        case ELF_SECTION_TYPE_LOUSER:

            file_write_format(FILE_STDOUT, "louser");

            break;

        case ELF_SECTION_TYPE_HIUSER:

            file_write_format(FILE_STDOUT, "hiuser");

            break;

        default:

            file_write_format(FILE_STDOUT, "unknown");

            break;

    }

}

void write_section_header(struct elf_section_header *header)
{

    file_write_format(FILE_STDOUT, "Name: %d\n", header->name);
    file_write_format(FILE_STDOUT, "Type: ");
    write_section_header_type(header->type);
    file_write_format(FILE_STDOUT, "\n");
    file_write_format(FILE_STDOUT, "Flags: %d\n", header->flags);
    file_write_format(FILE_STDOUT, "Address: 0x%x\n", header->address);
    file_write_format(FILE_STDOUT, "Offset: %d\n", header->offset);
    file_write_format(FILE_STDOUT, "Size: %d\n", header->size);
    file_write_format(FILE_STDOUT, "Link: %d\n", header->link);
    file_write_format(FILE_STDOUT, "Info: %d\n", header->info);
    file_write_format(FILE_STDOUT, "Align: %d\n", header->align);
    file_write_format(FILE_STDOUT, "Entry Size: %d\n", header->entrySize);

}

void write_section_headers(unsigned int address)
{

    struct elf_header *header = (struct elf_header *)address;

    unsigned int i;

    for (i = 0; i < header->sectionHeaderCount; i++)
    {

        struct elf_section_header *sheader = (struct elf_section_header *)(address + header->sectionHeaderOffset + i * header->sectionHeaderSize);

        file_write_format(FILE_STDOUT, "sh%d: {n:%d t:", i, sheader->name);
        write_section_header_type(sheader->type);
        file_write_format(FILE_STDOUT, " f:%d ad:0x%x o:%d s:%d l:%d i:%d al:%d es:%d}\n", sheader->flags, sheader->address, sheader->offset, sheader->size, sheader->link, sheader->info, sheader->align, sheader->entrySize);

    }

}

void write_relocation_table(unsigned int address)
{

    struct elf_header *header = (struct elf_header *)address;

    unsigned int i;

    for (i = 0; i < header->sectionHeaderCount; i++)
    {

        struct elf_section_header *sheader = (struct elf_section_header *)(address + header->sectionHeaderOffset + i * header->sectionHeaderSize);

        if (sheader->type != 9)
            continue;

        file_write_format(FILE_STDOUT, "Section %d: {name: %d; type: %d; addr: %x; off: %d; esi: %d}\n", sheader->name, sheader->type, sheader->flags, sheader->address, sheader->offset, sheader->entrySize);

        struct elf_relocate *rheader = (struct elf_relocate *)(address + sheader->offset);

        file_write_format(FILE_STDOUT, "offset:%d info:%d\n", rheader->offset, rheader->info);

    }

}

void parse_header(unsigned int address)
{

    h = (struct elf_header *)address;

/*
    unsigned int i;

    for (i = 0; i < h->sectionHeaderCount; i++)
        sh[i] = (struct elf_section_header *)(address + h->sectionHeaderOffset + h->sectionHeaderSize * i);
*/

}

void write_symbol_table(unsigned int address)
{

    struct elf_header *header = (struct elf_header *)address;

    unsigned int i;

    for (i = 0; i < header->sectionHeaderCount; i++)
    {

        struct elf_section_header *sheader = (struct elf_section_header *)(address + header->sectionHeaderOffset + i * header->sectionHeaderSize);

        if (sheader->type != 2)
            continue;

        file_write_format(FILE_STDOUT, "Section %d\n", i);

        unsigned int j;

        unsigned int max = sheader->size / sheader->entrySize;

        for (j = 0; j < max; j++)
        {

            struct elf_symbol *symbols = (struct elf_symbol *)(address + sheader->offset + j * sheader->entrySize);

            file_write_format(FILE_STDOUT, "n:%d v:%d s:%d i:%d o:%d x:%d\n", symbols->name, symbols->value, symbols->size, symbols->info, symbols->other, symbols->shndx);

        }

    }

}

void main(int argc, char *argv[])
{

    if (argc < 2)
        return;

    char path[256];

    int cwd = file_open("/cwd");
    unsigned int count = file_read(cwd, 256, path);
    file_close(cwd);

    string_concat(path, argv[1]);

    unsigned int content = 0x00360000;

    int file = file_open(path);

    if (file == -1)
        return;

    file_read(file, 0x4000, (unsigned int *)content);
    file_close(file);

    struct elf_header *header = (struct elf_header *)content;

    if (header->identify[0] != ELF_IDENTITY_MAGIC0)
        return;

    if (argc == 2)
    {

        write_header(header);
        write_section_headers(content);

        return;

    }

    if (!string_compare(argv[2], "program"))
    {

        if (header->programHeaderOffset)
        {

            struct elf_program_header *pHeader = (struct elf_program_header *)(content + header->programHeaderOffset);

            write_program_header(pHeader);

        }

        else
        {

            file_write_format(FILE_STDOUT, "No program header\n");

        }

    }

    if (!string_compare(argv[2], "section"))
    {

        unsigned int offset = 0;

        if (argc == 4)
        {

            if (string_length(argv[3]) == 2)
            {

                offset += (argv[3][0] - '0') * 10;
                offset += (argv[3][1] - '0') * 1;

            }

            else
            {

                offset += (argv[3][0] - '0') * 1;

            }

        }

        if (header->sectionHeaderOffset && (offset < header->sectionHeaderCount))
        {

            struct elf_section_header *sHeader = (struct elf_section_header *)(content + header->sectionHeaderOffset + offset * sizeof (struct elf_section_header));

            write_section_header(sHeader);

        }

        else
        {

            file_write_format(FILE_STDOUT, "No section header\n");

        }

    }

    if (!string_compare(argv[2], "relocation"))
    {

        write_relocation_table(content);

    }

    if (!string_compare(argv[2], "symbol"))
    {

        write_symbol_table(content);

    }

}

