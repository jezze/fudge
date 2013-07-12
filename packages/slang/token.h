enum token_type
{

    TOKEN_TYPE_NULL                     = 0,
    TOKEN_TYPE_NUM                      = 1,
    TOKEN_TYPE_ALPHA                    = 2,
    TOKEN_TYPE_ALPHANUM                 = 3,
    TOKEN_TYPE_SPACE                    = 4,
    TOKEN_TYPE_LT                       = 8,
    TOKEN_TYPE_GT                       = 16,
    TOKEN_TYPE_MINUS                    = 32,
    TOKEN_TYPE_SEMICOLON                = 64,
    TOKEN_TYPE_DOT                      = 128,
    TOKEN_TYPE_SLASH                    = 256,
    TOKEN_TYPE_PIPE                     = 512,
    TOKEN_TYPE_QUOTE                    = 1024,
    TOKEN_TYPE_NEWLINE                  = 2048

};

struct token_string
{

    char *position;
    unsigned int count;

};

struct token_state
{

    char *buffer;
    unsigned int count;
    unsigned int next;

};

enum token_type token_current(struct token_state *state);
unsigned int token_next(struct token_state *state);
unsigned int token_accept(struct token_state *state, enum token_type type);
void token_init_state(struct token_state *state, unsigned int count, char *buffer);
