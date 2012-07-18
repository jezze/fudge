#ifndef LIB_CALL_H
#define LIB_CALL_H

unsigned int call_open(unsigned int id, unsigned int count, void *buffer);
void call_close(unsigned int id);
unsigned int call_read(unsigned int id, unsigned int offset, unsigned int count, void *buffer);
unsigned int call_write(unsigned int id, unsigned int offset, unsigned int count, void *buffer);
unsigned int call_execute(unsigned int id);
unsigned int call_exit();
unsigned int call_wait();
unsigned int call_load(unsigned int id);
unsigned int call_unload(unsigned int id);
unsigned int call_attach(unsigned int id, void (*callback)());
unsigned int call_detach(unsigned int id);

#endif

