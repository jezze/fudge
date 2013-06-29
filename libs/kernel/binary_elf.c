#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"
#include "binary_elf.h"

static struct binary_protocol elf;

static unsigned int match(struct vfs_session *session, unsigned int id)
{

    struct elf_header header;

    if (session->protocol->read(session->backend, id, 0, ELF_HEADER_SIZE, &header) < ELF_HEADER_SIZE)
        return 0;

    return elf_validate(&header);

}

static unsigned int find_symbol(struct vfs_session *session, unsigned int id, unsigned int count, const char *symbol)
{

    struct elf_header header;
    struct elf_section_header sectionTable[16];
    unsigned int i;

    session->protocol->read(session->backend, id, 0, ELF_HEADER_SIZE, &header);

    if (header.shcount > 16)
        return 0;

    session->protocol->read(session->backend, id, header.shoffset, header.shsize * header.shcount, sectionTable);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_symbol symbolTable[512];
        char stringTable[4096];
        unsigned int address;

        if (sectionTable[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        session->protocol->read(session->backend, id, sectionTable[i].offset, sectionTable[i].size, symbolTable);
        session->protocol->read(session->backend, id, sectionTable[sectionTable[i].link].offset, sectionTable[sectionTable[i].link].size, stringTable);

        address = elf_find_symbol(&header, sectionTable, &sectionTable[i], symbolTable, stringTable, count, symbol);

        if (address)
            return address;

    }

    return 0;

}

static unsigned int copy_program(struct vfs_session *session, unsigned int id)
{

    struct elf_header header;
    struct elf_program_header programHeader[8];
    unsigned int i;

    session->protocol->read(session->backend, id, 0, ELF_HEADER_SIZE, &header);

    if (header.phcount > 8)
        return 0;

    session->protocol->read(session->backend, id, header.phoffset, header.phsize * header.phcount, programHeader);

    for (i = 0; i < header.phcount; i++)
        session->protocol->read(session->backend, id, programHeader[i].offset, programHeader[i].fsize, (void *)programHeader[i].vaddress);

    return header.entry;

}

static unsigned int relocate(struct vfs_session *session, unsigned int id, unsigned int address)
{

    struct elf_header header;
    struct elf_section_header sectionTable[16];
    unsigned int i;

    session->protocol->read(session->backend, id, 0, ELF_HEADER_SIZE, &header);

    if (header.shcount > 16)
        return 0;

    session->protocol->read(session->backend, id, header.shoffset, header.shsize * header.shcount, sectionTable);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_relocation relocationTable[256];
        struct elf_symbol symbolTable[512];

        sectionTable[i].address += address;

        if (sectionTable[i].type != ELF_SECTION_TYPE_REL)
            continue;

        session->protocol->read(session->backend, id, sectionTable[i].offset, sectionTable[i].size, relocationTable);
        session->protocol->read(session->backend, id, sectionTable[sectionTable[i].link].offset, sectionTable[sectionTable[i].link].size, symbolTable);
        elf_relocate_section(sectionTable, &sectionTable[i], &sectionTable[sectionTable[i].info], relocationTable, symbolTable, address);

    }

    session->protocol->write(session->backend, id, header.shoffset, header.shsize * header.shcount, sectionTable);

    return 1;

}

struct binary_protocol *binary_elf_setup()
{

    binary_init_protocol(&elf, match, find_symbol, copy_program, relocate);

    return &elf;

}

