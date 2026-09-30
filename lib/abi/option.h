#define OPTION_MAX                      32
#define OPTION_VALUESIZE                256

struct option
{

    unsigned int keyhash;
    char value[OPTION_VALUESIZE];

};

int option_getdecimal(char *key);
char *option_getstring(char *key);
unsigned int option_setstring(char *key, char *value);
void option_add(char *key, char *value);
