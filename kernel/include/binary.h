#ifndef KERNEL_BINARY_H
#define KERNEL_BINARY_H

unsigned int binary_find_symbol(struct runtime_descriptor *descriptor, char *func);
unsigned int binary_get_entry(struct runtime_descriptor *descriptor);
unsigned int binary_get_vaddress(struct runtime_descriptor *descriptor);
void binary_relocate(struct runtime_descriptor *descriptor, unsigned int address);

#endif

