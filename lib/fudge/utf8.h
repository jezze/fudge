unsigned int utf8_read(char *text, unsigned int length, unsigned int *codepoint);
unsigned int utf8_prev(char *text, unsigned int offset);
unsigned int utf8_fit(char *text, unsigned int length, unsigned int limit);
unsigned int utf8_write(char *text, unsigned int count, unsigned int codepoint);
