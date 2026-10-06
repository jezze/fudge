unsigned int system_run(unsigned int target, char *command);
unsigned int system_feed(char *command, void *input, unsigned int inputcount, void *output, unsigned int outputcount);
unsigned int system_resolve(char *domain, char *address, unsigned int size);
