#ifndef KERNEL_BINARY_H
#define KERNEL_BINARY_H

unsigned int binary_find_symbol(struct modules_filesystem *filesystem, unsigned int id, char *symbol);
unsigned int binary_get_entry(struct modules_filesystem *filesystem, unsigned int id);
unsigned int binary_copy_program(struct modules_filesystem *filesystem, unsigned int id);
unsigned int binary_relocate(struct modules_filesystem *filesystem, unsigned int id, unsigned int address);

#endif

