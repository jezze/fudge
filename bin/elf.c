#include <call.h>
#include <elf.h>
#include <file.h>
#include <string.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    int cwd = file_open("/dev/cwd");

    char path[256];

    unsigned int count = file_read(cwd, 256, path);
    string_concat(path, argv[1]);

    int file = file_open(path);

    if (file == -1)
    {

        file_close(cwd);

        return;

    }

    struct elf_header header;

    file_read(file, sizeof (struct elf_header), &header);

    if (header.identify[0] != ELF_IDENTITY_MAGIC0)
        return;

    //unsigned char identify[16];

    file_write_string(FILE_STDOUT, "ELF header");
    file_write_string(FILE_STDOUT, "\nType: ");
    file_write_dec(FILE_STDOUT, header.type);
    file_write_string(FILE_STDOUT, "\nMachine: ");
    file_write_dec(FILE_STDOUT, header.machine);
    file_write_string(FILE_STDOUT, "\nVersion: ");
    file_write_dec(FILE_STDOUT, header.version);
    file_write_string(FILE_STDOUT, "\nEntry: 0x");
    file_write_hex(FILE_STDOUT, header.entry);
    file_write_string(FILE_STDOUT, "\nProgram Header Offset: ");
    file_write_dec(FILE_STDOUT, header.programHeaderOffset);
    file_write_string(FILE_STDOUT, "\nSection Header Offset: ");
    file_write_dec(FILE_STDOUT, header.sectionHeaderOffset);
    file_write_string(FILE_STDOUT, "\nFlags: ");
    file_write_dec(FILE_STDOUT, header.flags);
    file_write_string(FILE_STDOUT, "\nHeader Size: ");
    file_write_dec(FILE_STDOUT, header.headerSize);
    file_write_string(FILE_STDOUT, "\nProgram Header Entry Size: ");
    file_write_dec(FILE_STDOUT, header.programHeaderEntrySize);
    file_write_string(FILE_STDOUT, "\nProgram Header Count: ");
    file_write_dec(FILE_STDOUT, header.programHeaderCount);
    file_write_string(FILE_STDOUT, "\nSection Header Entry Size: ");
    file_write_dec(FILE_STDOUT, header.sectionHeaderEntrySize);
    file_write_string(FILE_STDOUT, "\nSection Header Count: ");
    file_write_dec(FILE_STDOUT, header.sectionHeaderCount);
    file_write_string(FILE_STDOUT, "\nSection Header String Index: ");
    file_write_dec(FILE_STDOUT, header.sectionHeaderStringIndex);
    file_write_string(FILE_STDOUT, "\n\n");

    struct elf_program_header pHeader;

    file_read(file, sizeof (struct elf_program_header), &pHeader);

    file_write_string(FILE_STDOUT, "ELF program header:");
    file_write_string(FILE_STDOUT, "\nType: ");
    file_write_dec(FILE_STDOUT, pHeader.type);
    file_write_string(FILE_STDOUT, "\nOffset: ");
    file_write_dec(FILE_STDOUT, pHeader.offset);
    file_write_string(FILE_STDOUT, "\nVirtual address: 0x");
    file_write_hex(FILE_STDOUT, pHeader.virtualAddress);
    file_write_string(FILE_STDOUT, "\nPhysical address: 0x");
    file_write_hex(FILE_STDOUT, pHeader.physicalAddress);
    file_write_string(FILE_STDOUT, "\nFile size: ");
    file_write_dec(FILE_STDOUT, pHeader.fileSize);
    file_write_string(FILE_STDOUT, "\nMemory size: ");
    file_write_dec(FILE_STDOUT, pHeader.memorySize);
    file_write_string(FILE_STDOUT, "\nFlags: ");
    file_write_dec(FILE_STDOUT, pHeader.flags);
    file_write_string(FILE_STDOUT, "\nAlign: ");
    file_write_dec(FILE_STDOUT, pHeader.align);
    file_write_string(FILE_STDOUT, "\n");

    file_close(file);
    file_close(cwd);

}

