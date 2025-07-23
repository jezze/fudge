#define PANIC(source) panic(source, __FILE__, __LINE__)

unsigned int lookup(char *name);
void panic(unsigned int source, char *file, unsigned int line);
