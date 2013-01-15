#define ERROR_REGISTER_SLOTS            8

void error_assert(unsigned int condition, char *message, char *file, unsigned int line);
void error_panic(char *message, char *file, unsigned int line);
void error_register(unsigned int index, unsigned int value);
