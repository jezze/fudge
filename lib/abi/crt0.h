#define PANIC(source) panic(source, __FILE__, __LINE__)

unsigned int lookup(char *key);
unsigned int lookup2(char *name);
void panic(unsigned int source, char *file, unsigned int line);
