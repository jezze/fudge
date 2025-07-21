#define OPTION_MAX                      32

struct option
{

    unsigned int keyhash;
    char value[64];

};

unsigned int option_isvalid(char *key);
struct option *option_get(unsigned int i);
int option_getdecimal(char *key);
char *option_getstring(char *key);
unsigned int option_setdecimal(char *key, int value);
unsigned int option_set(char *key, char *value);
void option_add(char *key, char *value);
void option_init(struct option *option, char *key, char *value);
