unsigned int ascii_length(char *in);
void ascii_copy(char *out, char *in);
unsigned int ascii_toint(unsigned char c);
unsigned int ascii_rvalue(char *in, unsigned int count, unsigned int base);
unsigned int ascii_wvalue(void *out, unsigned int count, unsigned int value, unsigned int base);
unsigned int ascii_wvaluepadded(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int padding);
unsigned int ascii_dname(char *out, unsigned int ocount, char *in, unsigned int icount, char c);
unsigned int ascii_fname(char *out, unsigned int ocount, char *in, unsigned int icount, char c);
