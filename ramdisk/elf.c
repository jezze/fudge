#include <call.h>
#include <elf.h>
#include <vfs.h>
#include <write.h>

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

    write_string("ELF header");
    write_string("\nType: ");
    write_string_dec(header.type);
    write_string("\nMachine: ");
    write_string_dec(header.machine);
    write_string("\nVersion: ");
    write_string_dec(header.version);
    write_string("\nEntry: 0x");
    write_string_hex(header.entry);
    write_string("\nProgram Header Offset: ");
    write_string_dec(header.programHeaderOffset);
    write_string("\nSection Header Offset: ");
    write_string_dec(header.sectionHeaderOffset);
    write_string("\nFlags: ");
    write_string_dec(header.flags);
    write_string("\nHeader Size: ");
    write_string_dec(header.headerSize);
    write_string("\nProgram Header Entry Size: ");
    write_string_dec(header.programHeaderEntrySize);
    write_string("\nProgram Header Count: ");
    write_string_dec(header.programHeaderCount);
    write_string("\nSection Header Entry Size: ");
    write_string_dec(header.sectionHeaderEntrySize);
    write_string("\nSection Header Count: ");
    write_string_dec(header.sectionHeaderCount);
    write_string("\nSection Header String Index: ");
    write_string_dec(header.sectionHeaderStringIndex);
    write_string("\n\n");

    struct elf_program_header pHeader;

    vfs_read(node, header.programHeaderOffset, sizeof (struct elf_program_header), &pHeader);

    write_string("ELF program header:");
    write_string("\nType: ");
    write_string_dec(pHeader.type);
    write_string("\nOffset: ");
    write_string_dec(pHeader.offset);
    write_string("\nVirtual address: 0x");
    write_string_hex(pHeader.virtualAddress);
    write_string("\nPhysical address: 0x");
    write_string_hex(pHeader.physicalAddress);
    write_string("\nFile size: ");
    write_string_dec(pHeader.fileSize);
    write_string("\nMemory size: ");
    write_string_dec(pHeader.memorySize);
    write_string("\nFlags: ");
    write_string_dec(pHeader.flags);
    write_string("\nAlign: ");
    write_string_dec(pHeader.align);
    write_string("\n");

}

