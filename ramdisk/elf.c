#include <call.h>
#include <elf.h>
#include <file.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    struct vfs_node *node = call_open(argv[1]);

    if (!node)
        return;

    struct elf_header header;

    vfs_read(node, 0, sizeof (struct elf_header), &header);

    if (header.identify[0] != ELF_IDENTITY_MAGIC0)
        return;

    //unsigned char identify[16];

    file_write("ELF header");
    file_write("\nType: ");
    file_write_dec(header.type);
    file_write("\nMachine: ");
    file_write_dec(header.machine);
    file_write("\nVersion: ");
    file_write_dec(header.version);
    file_write("\nEntry: 0x");
    file_write_hex(header.entry);
    file_write("\nProgram Header Offset: ");
    file_write_dec(header.programHeaderOffset);
    file_write("\nSection Header Offset: ");
    file_write_dec(header.sectionHeaderOffset);
    file_write("\nFlags: ");
    file_write_dec(header.flags);
    file_write("\nHeader Size: ");
    file_write_dec(header.headerSize);
    file_write("\nProgram Header Entry Size: ");
    file_write_dec(header.programHeaderEntrySize);
    file_write("\nProgram Header Count: ");
    file_write_dec(header.programHeaderCount);
    file_write("\nSection Header Entry Size: ");
    file_write_dec(header.sectionHeaderEntrySize);
    file_write("\nSection Header Count: ");
    file_write_dec(header.sectionHeaderCount);
    file_write("\nSection Header String Index: ");
    file_write_dec(header.sectionHeaderStringIndex);
    file_write("\n\n");

    struct elf_program_header pHeader;

    vfs_read(node, header.programHeaderOffset, sizeof (struct elf_program_header), &pHeader);

    file_write("ELF program header:");
    file_write("\nType: ");
    file_write_dec(pHeader.type);
    file_write("\nOffset: ");
    file_write_dec(pHeader.offset);
    file_write("\nVirtual address: 0x");
    file_write_hex(pHeader.virtualAddress);
    file_write("\nPhysical address: 0x");
    file_write_hex(pHeader.physicalAddress);
    file_write("\nFile size: ");
    file_write_dec(pHeader.fileSize);
    file_write("\nMemory size: ");
    file_write_dec(pHeader.memorySize);
    file_write("\nFlags: ");
    file_write_dec(pHeader.flags);
    file_write("\nAlign: ");
    file_write_dec(pHeader.align);
    file_write("\n");

}

