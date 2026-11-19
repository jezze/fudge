struct dataentry;

struct dataentry *record_find(unsigned int source, char *set, char *key);
struct dataentry *record_create(unsigned int source, char *set, char *key);
struct dataentry *record_next(struct dataentry *current, unsigned int source, char *set);
char *record_getkey(struct dataentry *record);
char *record_getfield(struct dataentry *record, char *name, unsigned int length);
void record_setfield(struct dataentry *record, char *name, char *value);
void record_remove(unsigned int source, char *set, char *key);
void record_removesource(unsigned int source);
