#include <fudge.h>

static struct elf_header *h;
static struct elf_section_headers *sh[32];
static struct elf_symbol *sym[64];

void write_header_type(unsigned int type)
{

    switch (type)
    {

        case ELF_TYPE_NONE:

            file_write_string(FILE_STDOUT, "none");

            break;

        case ELF_TYPE_RELOCATABLE:

            file_write_string(FILE_STDOUT, "relocatable");

            break;

        case ELF_TYPE_EXECUTABLE:

            file_write_string(FILE_STDOUT, "executable");

            break;

        case ELF_TYPE_DYNAMIC:

            file_write_string(FILE_STDOUT, "dynamic");

            break;

        case ELF_TYPE_CORE:

            file_write_string(FILE_STDOUT, "core");

            break;

        case ELF_TYPE_LOPROC:

            file_write_string(FILE_STDOUT, "loproc");

            break;

        case ELF_TYPE_HIPROC:

            file_write_string(FILE_STDOUT, "hiproc");

            break;

        default:

            file_write_string(FILE_STDOUT, "unknown");

            break;

    }

}

void write_header(struct elf_header *header)
{

    file_write_string(FILE_STDOUT, "Type: ");
    write_header_type(header->type);
    file_write_string(FILE_STDOUT, "\nMachine: ");
    file_write_dec(FILE_STDOUT, header->machine);
    file_write_string(FILE_STDOUT, "\nVersion: ");
    file_write_dec(FILE_STDOUT, header->version);
    file_write_string(FILE_STDOUT, "\nEntry: 0x");
    file_write_hex(FILE_STDOUT, header->entry);
    file_write_string(FILE_STDOUT, "\nFlags: ");
    file_write_dec(FILE_STDOUT, header->flags);
    file_write_string(FILE_STDOUT, "\nSize: ");
    file_write_dec(FILE_STDOUT, header->size);
    file_write_string(FILE_STDOUT, "\n");

    file_write_string(FILE_STDOUT, "Program header: {");
    file_write_string(FILE_STDOUT, "o:");
    file_write_dec(FILE_STDOUT, header->programHeaderOffset);
    file_write_string(FILE_STDOUT, " s:");
    file_write_dec(FILE_STDOUT, header->programHeaderSize);
    file_write_string(FILE_STDOUT, " c:");
    file_write_dec(FILE_STDOUT, header->programHeaderCount);
    file_write_string(FILE_STDOUT, "}\n");

    file_write_string(FILE_STDOUT, "Section header: {");
    file_write_string(FILE_STDOUT, "o:");
    file_write_dec(FILE_STDOUT, header->sectionHeaderOffset);
    file_write_string(FILE_STDOUT, " s:");
    file_write_dec(FILE_STDOUT, header->sectionHeaderSize);
    file_write_string(FILE_STDOUT, " c:");
    file_write_dec(FILE_STDOUT, header->sectionHeaderCount);
    file_write_string(FILE_STDOUT, " si:");
    file_write_dec(FILE_STDOUT, header->sectionHeaderStringIndex);
    file_write_string(FILE_STDOUT, "}\n");

}

void write_program_header(struct elf_program_header *header)
{

    file_write_string(FILE_STDOUT, "Type: ");
    file_write_dec(FILE_STDOUT, header->type);
    file_write_string(FILE_STDOUT, "\nOffset: ");
    file_write_dec(FILE_STDOUT, header->offset);
    file_write_string(FILE_STDOUT, "\nVirtual address: 0x");
    file_write_hex(FILE_STDOUT, header->virtualAddress);
    file_write_string(FILE_STDOUT, "\nPhysical address: 0x");
    file_write_hex(FILE_STDOUT, header->physicalAddress);
    file_write_string(FILE_STDOUT, "\nFile size: ");
    file_write_dec(FILE_STDOUT, header->fileSize);
    file_write_string(FILE_STDOUT, "\nMemory size: ");
    file_write_dec(FILE_STDOUT, header->memorySize);
    file_write_string(FILE_STDOUT, "\nFlags: ");
    file_write_dec(FILE_STDOUT, header->flags);
    file_write_string(FILE_STDOUT, "\nAlign: ");
    file_write_dec(FILE_STDOUT, header->align);
    file_write_string(FILE_STDOUT, "\n");

}

void write_section_header_type(unsigned int type)
{

    switch (type)
    {

        case ELF_SECTION_TYPE_NULL:

            file_write_string(FILE_STDOUT, "null");

            break;

        case ELF_SECTION_TYPE_PROGBITS:

            file_write_string(FILE_STDOUT, "progbits");

            break;

        case ELF_SECTION_TYPE_SYMTAB:

            file_write_string(FILE_STDOUT, "symtab");

            break;

        case ELF_SECTION_TYPE_STRTAB:

            file_write_string(FILE_STDOUT, "strtab");

            break;

        case ELF_SECTION_TYPE_RELA:

            file_write_string(FILE_STDOUT, "rela");

            break;

        case ELF_SECTION_TYPE_HASH:

            file_write_string(FILE_STDOUT, "hash");

            break;

        case ELF_SECTION_TYPE_DYNAMIC:

            file_write_string(FILE_STDOUT, "dynamic");

            break;

        case ELF_SECTION_TYPE_NOTE:

            file_write_string(FILE_STDOUT, "note");

            break;

        case ELF_SECTION_TYPE_NOBITS:

            file_write_string(FILE_STDOUT, "nobits");

            break;

        case ELF_SECTION_TYPE_REL:

            file_write_string(FILE_STDOUT, "rel");

            break;

        case ELF_SECTION_TYPE_SHLIB:

            file_write_string(FILE_STDOUT, "shlib");

            break;

        case ELF_SECTION_TYPE_DYNSYM:

            file_write_string(FILE_STDOUT, "dynsym");

            break;

        case ELF_SECTION_TYPE_LOPROC:

            file_write_string(FILE_STDOUT, "loproc");

            break;

        case ELF_SECTION_TYPE_HIPROC:

            file_write_string(FILE_STDOUT, "hiproc");

            break;

        case ELF_SECTION_TYPE_LOUSER:

            file_write_string(FILE_STDOUT, "louser");

            break;

        case ELF_SECTION_TYPE_HIUSER:

            file_write_string(FILE_STDOUT, "hiuser");

            break;

        default:

            file_write_string(FILE_STDOUT, "unknown");

            break;

    }

}

void write_section_header(struct elf_section_header *header)
{

    file_write_string(FILE_STDOUT, "Name: ");
    file_write_dec(FILE_STDOUT, header->name);
    file_write_string(FILE_STDOUT, "\nType: ");
    write_section_header_type(header->type);
    file_write_string(FILE_STDOUT, "\nFlags: ");
    file_write_dec(FILE_STDOUT, header->flags);
    file_write_string(FILE_STDOUT, "\nAddress: ");
    file_write_hex(FILE_STDOUT, header->address);
    file_write_string(FILE_STDOUT, "\nOffset: ");
    file_write_dec(FILE_STDOUT, header->offset);
    file_write_string(FILE_STDOUT, "\nSize: ");
    file_write_dec(FILE_STDOUT, header->size);
    file_write_string(FILE_STDOUT, "\nLink: ");
    file_write_dec(FILE_STDOUT, header->link);
    file_write_string(FILE_STDOUT, "\nInfo: ");
    file_write_dec(FILE_STDOUT, header->info);
    file_write_string(FILE_STDOUT, "\nAlign: ");
    file_write_dec(FILE_STDOUT, header->align);
    file_write_string(FILE_STDOUT, "\nEntry Size: ");
    file_write_dec(FILE_STDOUT, header->entrySize);
    file_write_string(FILE_STDOUT, "\n");

}

void write_section_headers(unsigned int address)
{

    struct elf_header *header = (struct elf_header *)address;

    unsigned int i;

    for (i = 0; i < header->sectionHeaderCount; i++)
    {

        struct elf_section_header *sheader = (struct elf_section_header *)(address + header->sectionHeaderOffset + i * header->sectionHeaderSize);

        file_write_string(FILE_STDOUT, "sh");
        file_write_dec(FILE_STDOUT, i);
        file_write_string(FILE_STDOUT, ": {");
        file_write_string(FILE_STDOUT, "n:");
        file_write_dec(FILE_STDOUT, sheader->name);
        file_write_string(FILE_STDOUT, " t:");
        write_section_header_type(sheader->type);
        file_write_string(FILE_STDOUT, " f:");
        file_write_dec(FILE_STDOUT, sheader->flags);
        file_write_string(FILE_STDOUT, " ad: 0x");
        file_write_hex(FILE_STDOUT, sheader->address);
        file_write_string(FILE_STDOUT, " o:");
        file_write_dec(FILE_STDOUT, sheader->offset);
        file_write_string(FILE_STDOUT, " s:");
        file_write_dec(FILE_STDOUT, sheader->size);
        file_write_string(FILE_STDOUT, " l:");
        file_write_dec(FILE_STDOUT, sheader->link);
        file_write_string(FILE_STDOUT, " i:");
        file_write_dec(FILE_STDOUT, sheader->info);
        file_write_string(FILE_STDOUT, " al:");
        file_write_dec(FILE_STDOUT, sheader->align);
        file_write_string(FILE_STDOUT, " es:");
        file_write_dec(FILE_STDOUT, sheader->entrySize);
        file_write_string(FILE_STDOUT, "}\n");

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

        file_write_string(FILE_STDOUT, "Section ");
        file_write_dec(FILE_STDOUT, i);
        file_write_string(FILE_STDOUT, ": {");
        file_write_string(FILE_STDOUT, "name: ");
        file_write_dec(FILE_STDOUT, sheader->name);
        file_write_string(FILE_STDOUT, "; type: ");
        file_write_dec(FILE_STDOUT, sheader->type);
        file_write_string(FILE_STDOUT, "; flags: ");
        file_write_dec(FILE_STDOUT, sheader->flags);
        file_write_string(FILE_STDOUT, "; addr: ");
        file_write_hex(FILE_STDOUT, sheader->address);
        file_write_string(FILE_STDOUT, "; off: ");
        file_write_dec(FILE_STDOUT, sheader->offset);
        file_write_string(FILE_STDOUT, "; esi: ");
        file_write_dec(FILE_STDOUT, sheader->entrySize);
        file_write_string(FILE_STDOUT, "}\n");

        struct elf_relocate *rheader = (struct elf_relocate *)(address + sheader->offset);

        file_write_string(FILE_STDOUT, "offset:");
        file_write_dec(FILE_STDOUT, rheader->offset);
        file_write_string(FILE_STDOUT, " info:");
        file_write_dec(FILE_STDOUT, rheader->info);
        file_write_string(FILE_STDOUT, " \n");

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

        file_write_string(FILE_STDOUT, "Section ");
        file_write_dec(FILE_STDOUT, i);
        file_write_string(FILE_STDOUT, "\n");

        unsigned int j;

        unsigned int max = sheader->size / sheader->entrySize;

        for (j = 0; j < max; j++)
        {

            struct elf_symbol *symbols = (struct elf_symbol *)(address + sheader->offset + j * sheader->entrySize);

            file_write_string(FILE_STDOUT, "n:");
            file_write_dec(FILE_STDOUT, symbols->name);
            file_write_string(FILE_STDOUT, " v:");
            file_write_dec(FILE_STDOUT, symbols->value);
            file_write_string(FILE_STDOUT, " s:");
            file_write_dec(FILE_STDOUT, symbols->size);
            file_write_string(FILE_STDOUT, " i:");
            file_write_dec(FILE_STDOUT, symbols->info);
            file_write_string(FILE_STDOUT, " o:");
            file_write_dec(FILE_STDOUT, symbols->other);
            file_write_string(FILE_STDOUT, " x:");
            file_write_dec(FILE_STDOUT, symbols->shndx);
            file_write_string(FILE_STDOUT, " \n");

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

            file_write_string(FILE_STDOUT, "No program header\n");

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

            file_write_string(FILE_STDOUT, "No section header\n");

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

