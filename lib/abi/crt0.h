#define PANIC(source) panic(source, __FILE__, __LINE__)

void lookup(char *key);
void lookup2(char *key, char *name, unsigned int index, unsigned int inode);
void panic(unsigned int source, char *file, unsigned int line);
