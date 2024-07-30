#define PANIC(source) panic(source, __FILE__, __LINE__)

void panic(unsigned int source, char *file, unsigned int line);
