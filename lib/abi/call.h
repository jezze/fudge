unsigned int call_announce(unsigned int ichannel);
unsigned int call_create(unsigned int idescriptor, unsigned int pdescriptor, char *name, unsigned int length);
unsigned int call_debug(void);
unsigned int call_destroy(unsigned int idescriptor);
unsigned int call_despawn(void);
unsigned int call_kill(unsigned int task);
unsigned int call_link(unsigned int idescriptor, unsigned int source);
unsigned int call_list(unsigned int idescriptor, unsigned int cdescriptor, unsigned int count, struct record *records);
unsigned int call_load(unsigned int idescriptor);
unsigned int call_notify(unsigned int idescriptor, unsigned int event, unsigned int count, void *data);
unsigned int call_pick(struct message *message, void *data);
unsigned int call_place(unsigned int ichannel, unsigned int event, unsigned int count, void *data);
unsigned int call_read(unsigned int idescriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int call_read_all(unsigned int idescriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int call_spawn(unsigned int pdescriptor, unsigned int wdescriptor);
unsigned int call_spawn_absolute(unsigned int pdescriptor, unsigned int wdescriptor, char *path);
unsigned int call_stat(unsigned int idescriptor, struct record *record);
unsigned int call_unlink(unsigned int idescriptor);
unsigned int call_unload(unsigned int idescriptor);
unsigned int call_walk(unsigned int idescriptor, unsigned int pdescriptor, char *path, unsigned int length);
unsigned int call_walk_duplicate(unsigned int idescriptor, unsigned int pdescriptor);
unsigned int call_walk_relative(unsigned int idescriptor, unsigned int pdescriptor, char *path);
unsigned int call_walk_absolute(unsigned int idescriptor, char *path);
unsigned int call_write(unsigned int idescriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int call_write_all(unsigned int idescriptor, void *buffer, unsigned int count, unsigned int offset);
