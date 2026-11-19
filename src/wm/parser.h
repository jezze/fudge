struct widget;
struct dataentry;

unsigned int parser_bind(struct widget *each, unsigned int first, unsigned int count);
unsigned int parser_substitute(char *out, unsigned int size, char *text, struct dataentry *record);
void parser_parse(unsigned int source, char *in, unsigned int count, void *data);
