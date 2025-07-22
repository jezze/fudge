#define OPTION_MAX                      32

struct option
{

    unsigned int keyhash;
    char value[64];

};

int option_getdecimal(char *key);
char *option_getstring(char *key);
unsigned int option_setdecimal(char *key, int value);
unsigned int option_setstring(char *key, char *value);
void option_add(char *key, char *value);
void option_init(struct option *option, char *key, char *value);
