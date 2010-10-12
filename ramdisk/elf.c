#include <call.h>
#include <elf.h>
#include <stdout.h>
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

    stdout_write("ELF header");
    stdout_write("\nType: ");
    stdout_write_dec(header.type);
    stdout_write("\nMachine: ");
    stdout_write_dec(header.machine);
    stdout_write("\nVersion: ");
    stdout_write_dec(header.version);
    stdout_write("\nEntry: 0x");
    stdout_write_hex(header.entry);
    stdout_write("\nProgram Header Offset: ");
    stdout_write_dec(header.programHeaderOffset);
    stdout_write("\nSection Header Offset: ");
    stdout_write_dec(header.sectionHeaderOffset);
    stdout_write("\nFlags: ");
    stdout_write_dec(header.flags);
    stdout_write("\nHeader Size: ");
    stdout_write_dec(header.headerSize);
    stdout_write("\nProgram Header Entry Size: ");
    stdout_write_dec(header.programHeaderEntrySize);
    stdout_write("\nProgram Header Count: ");
    stdout_write_dec(header.programHeaderCount);
    stdout_write("\nSection Header Entry Size: ");
    stdout_write_dec(header.sectionHeaderEntrySize);
    stdout_write("\nSection Header Count: ");
    stdout_write_dec(header.sectionHeaderCount);
    stdout_write("\nSection Header String Index: ");
    stdout_write_dec(header.sectionHeaderStringIndex);
    stdout_write("\n\n");

    struct elf_program_header pHeader;

    vfs_read(node, header.programHeaderOffset, sizeof (struct elf_program_header), &pHeader);

    stdout_write("ELF program header:");
    stdout_write("\nType: ");
    stdout_write_dec(pHeader.type);
    stdout_write("\nOffset: ");
    stdout_write_dec(pHeader.offset);
    stdout_write("\nVirtual address: 0x");
    stdout_write_hex(pHeader.virtualAddress);
    stdout_write("\nPhysical address: 0x");
    stdout_write_hex(pHeader.physicalAddress);
    stdout_write("\nFile size: ");
    stdout_write_dec(pHeader.fileSize);
    stdout_write("\nMemory size: ");
    stdout_write_dec(pHeader.memorySize);
    stdout_write("\nFlags: ");
    stdout_write_dec(pHeader.flags);
    stdout_write("\nAlign: ");
    stdout_write_dec(pHeader.align);
    stdout_write("\n");

}

