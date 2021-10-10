#define CSTRING_NUMSIZE                 1024

unsigned int cstring_length(char *in);
unsigned int cstring_lengthz(char *in);
void cstring_copy(char *out, char *in);
unsigned int cstring_match(char *in1, char *in2);
unsigned int cstring_isalpha(char c);
unsigned int cstring_isdigit(char c);
unsigned int cstring_toint(char c);
unsigned int cstring_rvalue(char *in, unsigned int count, unsigned int base);
unsigned int cstring_wvalue(char *out, unsigned int count, int value, unsigned int base, unsigned int padding);
