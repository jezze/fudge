unsigned int call_debug(char *message);
unsigned int call_despawn(void);
unsigned int call_find(unsigned int count, char *name);
unsigned int call_kill(unsigned int task);
unsigned int call_load(unsigned int address);
unsigned int call_pick(unsigned int index, struct message *message, unsigned int count, void *data);
unsigned int call_place(unsigned int index, unsigned int target, unsigned int event, unsigned int count, void *data);
unsigned int call_spawn(unsigned int address);
unsigned int call_unload(unsigned int address);
