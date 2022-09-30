struct option
{

    char *key;
    char value[64];

};

int option_getdecimal(struct option *options, char *key);
char *option_getstring(struct option *options, char *key);
void option_set(struct option *options, char *key, char *value);
void option_add(struct option *options, char *key, char *value);
void option_init(struct option *option, char *key, char *value);
