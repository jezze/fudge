#ifndef KERNEL_BINARY_H
#define KERNEL_BINARY_H

unsigned int binary_find_symbol(struct vfs_filesystem *filesystem, unsigned int id, char *symbol);
unsigned int binary_get_entry(struct vfs_filesystem *filesystem, unsigned int id);
unsigned int binary_copy_program(struct vfs_filesystem *filesystem, unsigned int id);
unsigned int binary_relocate(struct vfs_filesystem *filesystem, unsigned int id, unsigned int address);

#endif

