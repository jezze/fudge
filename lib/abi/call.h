unsigned int call_create(unsigned int descriptor, unsigned int pdescriptor, char *name, unsigned int length);
unsigned int call_debug(void);
unsigned int call_destroy(unsigned int descriptor);
unsigned int call_despawn(void);
unsigned int call_kill(unsigned int id);
unsigned int call_link(unsigned int descriptor);
unsigned int call_list(unsigned int descriptor, unsigned int cdescriptor, unsigned int count, struct record *records);
unsigned int call_load(unsigned int descriptor);
unsigned int call_notify(unsigned int descriptor, unsigned int event, unsigned int count, void *data);
unsigned int call_pick(struct message_header *header, void *data);
unsigned int call_place(unsigned int id, struct message_header *header, void *data);
unsigned int call_read(unsigned int descriptor, void *buffer, unsigned int count);
unsigned int call_seek(unsigned int descriptor, unsigned int offset);
unsigned int call_spawn(void);
unsigned int call_unlink(unsigned int descriptor);
unsigned int call_unload(unsigned int descriptor);
unsigned int call_walk(unsigned int descriptor, unsigned int pdescriptor, char *path, unsigned int length);
unsigned int call_write(unsigned int descriptor, void *buffer, unsigned int count);
