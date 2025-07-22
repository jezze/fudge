#define PANIC(source) panic(source, __FILE__, __LINE__)

unsigned int lookup(unsigned int type, char *name);
void panic(unsigned int source, char *file, unsigned int line);
