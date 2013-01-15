unsigned int binary_find_symbol(struct vfs_interface *interface, unsigned int id, unsigned int count, const char *symbol);
unsigned int binary_copy_program(struct vfs_interface *interface, unsigned int id);
unsigned int binary_relocate(struct vfs_interface *interface, unsigned int id, unsigned int address);
