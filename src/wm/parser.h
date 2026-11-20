struct widget;
struct dataentry;

unsigned int parser_bindall(void);
unsigned int parser_substitute(char *out, unsigned int size, char *text, struct dataentry *record);
void parser_parse(unsigned int source, char *in, unsigned int count, void *data);
