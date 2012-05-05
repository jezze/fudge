#ifndef LIB_CALL_H
#define LIB_CALL_H

unsigned int call_open(char *path);
void call_close(unsigned int fd);
unsigned int call_read(unsigned int fd, unsigned int offset, unsigned int count, void *buffer);
unsigned int call_write(unsigned int fd, unsigned int offset, unsigned int count, void *buffer);
unsigned int call_execute(unsigned int fd, unsigned int argc, char **argv);
unsigned int call_load(unsigned int fd);
unsigned int call_unload(unsigned int fd);
unsigned int call_exit();
unsigned int call_wait();
unsigned int call_attach(int index, void (*routine)());
unsigned int call_detach(int index);
void call_halt();
void call_reboot();

#endif

