#include <call.h>
#include <elf.h>
#include <file.h>
#include <string.h>

void write_header(struct elf_header *header)
{

    file_write_string(FILE_STDOUT, "Type: ");
    file_write_dec(FILE_STDOUT, header->type);
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

    file_write_string(FILE_STDOUT, "Program header: ");
    file_write_string(FILE_STDOUT, "{offset: ");
    file_write_dec(FILE_STDOUT, header->programHeaderOffset);
    file_write_string(FILE_STDOUT, "; entry size: ");
    file_write_dec(FILE_STDOUT, header->programHeaderEntrySize);
    file_write_string(FILE_STDOUT, "; count: ");
    file_write_dec(FILE_STDOUT, header->programHeaderCount);
    file_write_string(FILE_STDOUT, "}\n");

    file_write_string(FILE_STDOUT, "Section header: ");
    file_write_string(FILE_STDOUT, "{offset: ");
    file_write_dec(FILE_STDOUT, header->sectionHeaderOffset);
    file_write_string(FILE_STDOUT, "; entry size: ");
    file_write_dec(FILE_STDOUT, header->sectionHeaderEntrySize);
    file_write_string(FILE_STDOUT, "; count: ");
    file_write_dec(FILE_STDOUT, header->sectionHeaderCount);
    file_write_string(FILE_STDOUT, "; string index: ");
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

void write_section_header(struct elf_section_header *header)
{

    file_write_string(FILE_STDOUT, "Name: ");
    file_write_dec(FILE_STDOUT, header->name);
    file_write_string(FILE_STDOUT, "\nType: ");
    file_write_dec(FILE_STDOUT, header->type);
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
    file_write_string(FILE_STDOUT, "\nEntry: ");
    file_write_dec(FILE_STDOUT, header->entrySize);
    file_write_string(FILE_STDOUT, "\n");

}

void main(int argc, char *argv[])
{

    if (argc < 2)
        return;

    char path[256];

    int cwd = file_open("/dev/cwd");
    unsigned int count = file_read(cwd, 256, path);
    file_close(cwd);

    string_concat(path, argv[1]);

    char *content = 0x00360000;

    int file = file_open(path);

    if (file == -1)
        return;

    file_read(file, 0x4000, content);
    file_close(file);

    struct elf_header *header = (struct elf_header *)content;

    if (header->identify[0] != ELF_IDENTITY_MAGIC0)
        return;

    if (argc == 2)
    {

        file_write_string(FILE_STDOUT, "*** ELF header ***\n");
        write_header(header);

        return;

    }

    if (!string_compare(argv[2], "program"))
    {

        if (header->programHeaderOffset)
        {

            struct elf_program_header *pHeader = (struct elf_program_header *)(content + header->programHeaderOffset);

            file_write_string(FILE_STDOUT, "*** ELF program header ***\n");
            write_program_header(pHeader);

        }

        else
        {

            file_write_string(FILE_STDOUT, "No program header\n");

        }

    }

    if (!string_compare(argv[2], "section"))
    {

        unsigned int offset = (argc == 4) ? (argv[3][0] - '0') : 0;

        if (header->sectionHeaderOffset && (offset < header->sectionHeaderCount))
        {

            struct elf_section_header *sHeader = (struct elf_section_header *)(content + header->sectionHeaderOffset + offset * sizeof (struct elf_section_header));

            file_write_string(FILE_STDOUT, "*** ELF section header ***\n");
            write_section_header(sHeader);

        }

    }

}

