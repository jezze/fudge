unsigned int ascii_length(char *in);
unsigned int ascii_lengthz(char *in);
void ascii_copy(char *out, char *in);
unsigned int ascii_match(char *in1, char *in2);
unsigned int ascii_toint(unsigned char c);
unsigned int ascii_rvalue(char *in, unsigned int count, unsigned int base);
unsigned int ascii_wvalue(char *out, unsigned int count, int value, unsigned int base, unsigned int padding);
