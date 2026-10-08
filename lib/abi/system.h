unsigned int system_run(unsigned int target, char *fmt, ...);
unsigned int system_runv(unsigned int target, char *fmt, void **args);
unsigned int system_feed(void *input, unsigned int inputcount, void *output, unsigned int outputcount, char *fmt, ...);
unsigned int system_feedv(void *input, unsigned int inputcount, void *output, unsigned int outputcount, char *fmt, void **args);
unsigned int system_unixtime(char *service);
unsigned int system_resolve(char *domain, char *address, unsigned int size);
