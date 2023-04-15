#define PANIC() panic(__FILE__, __LINE__)

void panic(char *file, unsigned int line);
