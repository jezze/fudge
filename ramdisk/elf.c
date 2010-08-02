#include <call.h>
#include <elf.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    struct vfs_node *node = call_vfs_find(argv[1]);

    if (!node)
        return;

    struct elf_header header;

    vfs_read(node, 0, sizeof (struct elf_header), &header);

    if (header.identify[0] != ELF_IDENTITY_MAGIC0)
        return;

    //unsigned char identify[16];

    call_puts("ELF header");
    call_puts("\nType: ");
    call_puts_dec(header.type);
    call_puts("\nMachine: ");
    call_puts_dec(header.machine);
    call_puts("\nVersion: ");
    call_puts_dec(header.version);
    call_puts("\nEntry: 0x");
    call_puts_hex(header.entry);
    call_puts("\nProgram Header Offset: ");
    call_puts_dec(header.programHeaderOffset);
    call_puts("\nSection Header Offset: ");
    call_puts_dec(header.sectionHeaderOffset);
    call_puts("\nFlags: ");
    call_puts_dec(header.flags);
    call_puts("\nHeader Size: ");
    call_puts_dec(header.headerSize);
    call_puts("\nProgram Header Entry Size: ");
    call_puts_dec(header.programHeaderEntrySize);
    call_puts("\nProgram Header Count: ");
    call_puts_dec(header.programHeaderCount);
    call_puts("\nSection Header Entry Size: ");
    call_puts_dec(header.sectionHeaderEntrySize);
    call_puts("\nSection Header Count: ");
    call_puts_dec(header.sectionHeaderCount);
    call_puts("\nSection Header String Index: ");
    call_puts_dec(header.sectionHeaderStringIndex);
    call_puts("\n\n");

    struct elf_program_header pHeader;

    vfs_read(node, header.programHeaderOffset, sizeof (struct elf_program_header), &pHeader);

    call_puts("ELF program header:");
    call_puts("\nType: ");
    call_puts_dec(pHeader.type);
    call_puts("\nOffset: ");
    call_puts_dec(pHeader.offset);
    call_puts("\nVirtual address: 0x");
    call_puts_hex(pHeader.virtualAddress);
    call_puts("\nPhysical address: 0x");
    call_puts_hex(pHeader.physicalAddress);
    call_puts("\nFile size: ");
    call_puts_dec(pHeader.fileSize);
    call_puts("\nMemory size: ");
    call_puts_dec(pHeader.memorySize);
    call_puts("\nFlags: ");
    call_puts_dec(pHeader.flags);
    call_puts("\nAlign: ");
    call_puts_dec(pHeader.align);
    call_puts("\n");

}

