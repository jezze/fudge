#include <call.h>
#include <elf.h>
#include <file.h>
#include <session.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    struct vfs_node *node = vfs_find(session_get_location(), argv[1]);

    if (!node)
        return;

    struct elf_header header;

    vfs_read(node, 0, sizeof (struct elf_header), &header);

    if (header.identify[0] != ELF_IDENTITY_MAGIC0)
        return;

    //unsigned char identify[16];

    file_write_string(session_get_out(), "ELF header");
    file_write_string(session_get_out(), "\nType: ");
    file_write_dec(session_get_out(), header.type);
    file_write_string(session_get_out(), "\nMachine: ");
    file_write_dec(session_get_out(), header.machine);
    file_write_string(session_get_out(), "\nVersion: ");
    file_write_dec(session_get_out(), header.version);
    file_write_string(session_get_out(), "\nEntry: 0x");
    file_write_hex(session_get_out(), header.entry);
    file_write_string(session_get_out(), "\nProgram Header Offset: ");
    file_write_dec(session_get_out(), header.programHeaderOffset);
    file_write_string(session_get_out(), "\nSection Header Offset: ");
    file_write_dec(session_get_out(), header.sectionHeaderOffset);
    file_write_string(session_get_out(), "\nFlags: ");
    file_write_dec(session_get_out(), header.flags);
    file_write_string(session_get_out(), "\nHeader Size: ");
    file_write_dec(session_get_out(), header.headerSize);
    file_write_string(session_get_out(), "\nProgram Header Entry Size: ");
    file_write_dec(session_get_out(), header.programHeaderEntrySize);
    file_write_string(session_get_out(), "\nProgram Header Count: ");
    file_write_dec(session_get_out(), header.programHeaderCount);
    file_write_string(session_get_out(), "\nSection Header Entry Size: ");
    file_write_dec(session_get_out(), header.sectionHeaderEntrySize);
    file_write_string(session_get_out(), "\nSection Header Count: ");
    file_write_dec(session_get_out(), header.sectionHeaderCount);
    file_write_string(session_get_out(), "\nSection Header String Index: ");
    file_write_dec(session_get_out(), header.sectionHeaderStringIndex);
    file_write_string(session_get_out(), "\n\n");

    struct elf_program_header pHeader;

    vfs_read(node, header.programHeaderOffset, sizeof (struct elf_program_header), &pHeader);

    file_write_string(session_get_out(), "ELF program header:");
    file_write_string(session_get_out(), "\nType: ");
    file_write_dec(session_get_out(), pHeader.type);
    file_write_string(session_get_out(), "\nOffset: ");
    file_write_dec(session_get_out(), pHeader.offset);
    file_write_string(session_get_out(), "\nVirtual address: 0x");
    file_write_hex(session_get_out(), pHeader.virtualAddress);
    file_write_string(session_get_out(), "\nPhysical address: 0x");
    file_write_hex(session_get_out(), pHeader.physicalAddress);
    file_write_string(session_get_out(), "\nFile size: ");
    file_write_dec(session_get_out(), pHeader.fileSize);
    file_write_string(session_get_out(), "\nMemory size: ");
    file_write_dec(session_get_out(), pHeader.memorySize);
    file_write_string(session_get_out(), "\nFlags: ");
    file_write_dec(session_get_out(), pHeader.flags);
    file_write_string(session_get_out(), "\nAlign: ");
    file_write_dec(session_get_out(), pHeader.align);
    file_write_string(session_get_out(), "\n");

}

