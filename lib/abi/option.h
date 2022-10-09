struct option
{

    char *key;
    char value[64];

};

int option_getdecimal(char *key);
char *option_getstring(char *key);
unsigned int option_set(char *key, char *value);
void option_add(char *key, char *value);
void option_init(struct option *option, char *key, char *value);
