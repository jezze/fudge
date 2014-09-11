#include <fudge.h>

enum token_type
{

    END,
    IDENT,
    IN,
    OUT,
    PIPE

};

struct token
{

    unsigned int type;
    char *str;

};

static unsigned char input[4096];
static unsigned int ninput;
static struct token infix[512];
static unsigned int ninfix;
static struct token postfix[512];
static unsigned int npostfix;
static struct token stack[8];
static unsigned int nstack;
static char strtbl[4096];
static unsigned int nstrtbl;
static unsigned int ident;
static unsigned int identstart;
static unsigned int identcount;
static unsigned int identcurrent;

static unsigned int walk_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_walk(index, CALL_DR, count - 1, buffer + 1);

    return call_walk(index, indexw, count, buffer);

}

static unsigned int strtbl_push(char c)
{

    strtbl[nstrtbl] = c;
    nstrtbl++;

    return nstrtbl - 1;

}

static void infix_push(unsigned int type, char *str)
{

    infix[ninfix].type = type;
    infix[ninfix].str = str;
    ninfix++;

}

static void postfix_push(struct token *tok)
{

    postfix[npostfix].type = tok->type;
    postfix[npostfix].str = tok->str;
    npostfix++;

}

static void stack_push(struct token *tok)
{

    stack[nstack].type = tok->type;
    stack[nstack].str = tok->str;
    nstack++;

}

static struct token *stack_pop()
{

    if (!nstack)
        return 0;

    nstack--;

    return &stack[nstack];

}

static unsigned int precedence(struct token *tok)
{

    switch (tok->type)
    {

        case PIPE:
            return 1;

        case IN:
        case OUT:
            return 2;

        case END:
            return 3;

    }

    return 0;

}

static void infix2postfix(struct token *tok)
{

    struct token *t;

    if (tok->type == IDENT)
    {

        postfix_push(tok);

        return;

    }

    if (tok->type == END)
    {

        while ((t = stack_pop()))
            postfix_push(t);

        postfix_push(tok);

        return;

    }

    while ((t = stack_pop()))
    {

        if (precedence(tok) > precedence(t))
        {

            stack_push(t);

            break;

        }

        postfix_push(t);

    }

    stack_push(tok);

}

static void translate()
{

    unsigned int i;

    for (i = 0; i < ninfix; i++)
        infix2postfix(&infix[i]);

}

static char next()
{

    char x = input[ninput];

    ninput++;

    return x;

}

static void ident_begin()
{

    identstart = identcurrent;
    identcount = 1;

}

static void ident_end()
{

    infix_push(IDENT, strtbl + identstart);

    identcurrent = strtbl_push('\0');
    identcount = 0;

}

static void tokenize()
{

    char c;

    infix_push(PIPE, 0);

    while ((c = next()))
    {

        ident = 0;

        switch (c)
        {

        case ' ':
        case '\t':
            break;

        case '<':
            infix_push(IN, 0);

            break;

        case '>':
            infix_push(OUT, 0);

            break;

        case '|':
            infix_push(PIPE, 0);

            break;

        case ';':
        case '\n':
            if (identcount)
                ident_end();

            infix_push(END, 0);
            infix_push(PIPE, 0);

            break;

        default:
            ident = 1;
            identcurrent = strtbl_push(c);

            break;

        }

        if (ident)
        {

            if (identcount)
                identcount++;
            else
                ident_begin();

        }

        else
        {

            if (identcount)
                ident_end();

        }

    }

    if (identcount)
        ident_end();

    infix_push(END, 0);

}

static void parse()
{

    unsigned int i;

    call_walk(CALL_I1, CALL_I0, 0, 0);
    call_walk(CALL_O1, CALL_O0, 0, 0);

    for (i = 0; i < npostfix; i++)
    {

        struct token *t = &postfix[i];
        struct token *a;

        switch (t->type)
        {

            case IDENT:
                stack_push(t);

                break;

            case IN:
                a = stack_pop();

                if (!a)
                    return;

                if (!walk_path(CALL_I1, CALL_DW, ascii_length(a->str), a->str))
                    return;

                break;

            case OUT:
                a = stack_pop();

                if (!a)
                    return;

                if (!walk_path(CALL_O1, CALL_DW, ascii_length(a->str), a->str))
                    return;

                break;

            case PIPE:
                a = stack_pop();

                if (!a)
                    return;

                if (!walk_path(CALL_DP, CALL_L0, ascii_length(a->str), a->str))
                    return;

                call_spawn(CALL_DP);

                break;

            case END:
                break;

        }

    }

}

void main()
{

    ninput = 0;
    ninfix = 0;
    npostfix = 0;
    nstack = 0;
    nstrtbl = 0;
    ident = 0;
    identstart = 0;
    identcount = 0;
    identcurrent = 0;

    if (!call_walk(CALL_L0, CALL_DR, 4, "bin/"))
        return;

    call_open(CALL_I0);
    call_read(CALL_I0, 0, FUDGE_BSIZE, input);
    call_close(CALL_I0);

    tokenize();

    if (nstack)
        return;

    translate();

    if (nstack)
        return;

    parse();

}

