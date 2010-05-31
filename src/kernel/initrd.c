#include <lib/types.h>
#include <lib/mem.h>
#include <lib/string.h>
#include <kernel/heap.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>

initrd_header_t *initrd_header;
initrd_file_header_t *file_headers;
vfs_node_t initrd_root;
vfs_node_t root_nodes[32];

static uint32_t initrd_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint32_t *buffer)
{

    initrd_file_header_t header = file_headers[node->inode];

    if (offset > header.length)
        return 0;

    if (offset + size > header.length)
        size = header.length - offset;

    memcpy(buffer, (uint8_t*) (header.offset + offset), size);

    return size;

}

static vfs_node_t *initrd_walk(vfs_node_t *node, uint32_t index)
{

    if (index < initrd_header->nfiles)
        return &root_nodes[index];
    else
        return 0;

}

static vfs_node_t *initrd_find(vfs_node_t *node, char *name)
{

    uint32_t i;

    for (i = 0; i < initrd_header->nfiles; i++)
    {

        if (!string_compare(name, root_nodes[i].name))
            return &root_nodes[i];

    }

    return 0;

}

vfs_node_t *initrd_init(uint32_t location)
{

    initrd_header = (initrd_header_t *)location;
    file_headers = (initrd_file_header_t *)(location + sizeof (initrd_header_t));

    string_copy(initrd_root.name, "initrd");
    initrd_root.inode = 0;
    initrd_root.length = 0;
    initrd_root.flags = VFS_DIRECTORY;
    initrd_root.open = 0;
    initrd_root.close = 0;
    initrd_root.read = 0;
    initrd_root.write = 0;
    initrd_root.walk = &initrd_walk;
    initrd_root.find = &initrd_find;

    uint32_t i;

    for (i = 0; i < initrd_header->nfiles; i++)
    {

        file_headers[i].offset += location;
        string_copy(root_nodes[i].name, file_headers[i].name);
        root_nodes[i].inode = i;
        root_nodes[i].length = file_headers[i].length;
        root_nodes[i].flags = VFS_FILE;
        root_nodes[i].open = 0;
        root_nodes[i].close = 0;
        root_nodes[i].read = &initrd_read;
        root_nodes[i].write = 0;
        root_nodes[i].walk = 0;
        root_nodes[i].find = 0;

    }

    return &initrd_root;

}

