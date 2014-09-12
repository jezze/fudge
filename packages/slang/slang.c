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

struct tokenlist
{

    unsigned int size;
    struct token *table;

};

static unsigned int walk_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_walk(index, CALL_DR, count - 1, buffer + 1);

    return call_walk(index, indexw, count, buffer);

}

static void tokenlist_init(struct tokenlist *list, struct token *table)
{

    memory_clear(list, sizeof (struct tokenlist));

    list->table = table;

}

static void tokenlist_add(struct tokenlist *list, unsigned int type, char *str)
{

    list->table[list->size].type = type;
    list->table[list->size].str = str;
    list->size++;

}

static void tokenlist_copy(struct tokenlist *list, struct token *tok)
{

    list->table[list->size].type = tok->type;
    list->table[list->size].str = tok->str;
    list->size++;

}

static struct token *tokenlist_pop(struct tokenlist *list)
{

    if (!list->size)
        return 0;

    list->size--;

    return &list->table[list->size];

}

static unsigned int strtbl_push(char *strtbl, unsigned int offset, char c)
{

    strtbl[offset] = c;

    return offset + 1;

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

static void infixtbl2postfixtbl(struct tokenlist *postfix, struct token *tok, struct tokenlist *stack)
{

    struct token *t;

    if (tok->type == IDENT)
    {

        tokenlist_copy(postfix, tok);

        return;

    }

    if (tok->type == END)
    {

        while ((t = tokenlist_pop(stack)))
            tokenlist_copy(postfix, t);

        tokenlist_copy(postfix, tok);

        return;

    }

    while ((t = tokenlist_pop(stack)))
    {

        if (precedence(tok) > precedence(t))
        {

            tokenlist_copy(stack, t);

            break;

        }

        tokenlist_copy(postfix, t);

    }

    tokenlist_copy(stack, tok);

}

static void translate(struct tokenlist *postfix, struct tokenlist *infix, struct tokenlist *stack)
{

    unsigned int i;

    for (i = 0; i < infix->size; i++)
        infixtbl2postfixtbl(postfix, &infix->table[i], stack);

}

static void tokenize(struct tokenlist *infix, char *strtbl, unsigned int count, char *buffer)
{

    unsigned int i;
    unsigned int ident;
    unsigned int identstart = 0;
    unsigned int identcount = 0;
    unsigned int identcurrent = 0;

    tokenlist_add(infix, PIPE, 0);

    for (i = 0; i < count; i++)
    {

        char c = buffer[i];

        ident = 0;

        switch (c)
        {

        case ' ':
        case '\t':
            break;

        case '<':
            tokenlist_add(infix, IN, 0);

            break;

        case '>':
            tokenlist_add(infix, OUT, 0);

            break;

        case '|':
            tokenlist_add(infix, PIPE, 0);

            break;

        case ';':
        case '\n':
            if (identcount)
            {

                tokenlist_add(infix, IDENT, strtbl + identstart);
                identcurrent = strtbl_push(strtbl, identcurrent, '\0');
                identcount = 0;

            }

            tokenlist_add(infix, END, 0);
            tokenlist_add(infix, PIPE, 0);

            break;

        default:
            ident = 1;
            identcurrent = strtbl_push(strtbl, identcurrent, c);

            break;

        }

        if (ident)
        {

            if (!identcount)
                identstart = identcurrent - 1;

            identcount++;

        }

        else
        {

            if (identcount)
            {

                tokenlist_add(infix, IDENT, strtbl + identstart);
                identcurrent = strtbl_push(strtbl, identcurrent, '\0');
                identcount = 0;

            }

        }

    }

    if (identcount)
    {

        tokenlist_add(infix, IDENT, strtbl + identstart);
        identcurrent = strtbl_push(strtbl, identcurrent, '\0');
        identcount = 0;

    }

    tokenlist_add(infix, END, 0);

}

static void parse(struct tokenlist *postfix, struct tokenlist *stack)
{

    unsigned int i;

    call_walk(CALL_I1, CALL_I0, 0, 0);
    call_walk(CALL_O1, CALL_O0, 0, 0);

    for (i = 0; i < postfix->size; i++)
    {

        struct token *t = &postfix->table[i];
        struct token *a;

        switch (t->type)
        {

        case IDENT:
            tokenlist_copy(stack, t);

            break;

        case IN:
            a = tokenlist_pop(stack);

            if (!a)
                return;

            if (!walk_path(CALL_I1, CALL_DW, ascii_length(a->str), a->str))
                return;

            break;

        case OUT:
            a = tokenlist_pop(stack);

            if (!a)
                return;

            if (!walk_path(CALL_O1, CALL_DW, ascii_length(a->str), a->str))
                return;

            break;

        case PIPE:
            a = tokenlist_pop(stack);

            if (!a)
                return;

            if (!walk_path(CALL_DP, CALL_L0, ascii_length(a->str), a->str))
                return;

            call_spawn(CALL_DP);

            break;

        case END:
            call_walk(CALL_I1, CALL_I0, 0, 0);
            call_walk(CALL_O1, CALL_O0, 0, 0);

            break;

        }

    }

}

void main()
{

    char buffer[4096];
    unsigned int count;
    struct token infixtbl[512];
    struct token postfixtbl[512];
    struct token stacktbl[8];
    struct tokenlist infix;
    struct tokenlist postfix;
    struct tokenlist stack;
    char strtbl[4096];

    tokenlist_init(&infix, infixtbl);
    tokenlist_init(&postfix, postfixtbl);
    tokenlist_init(&stack, stacktbl);

    if (!call_walk(CALL_L0, CALL_DR, 4, "bin/"))
        return;

    call_open(CALL_I0);

    count = call_read(CALL_I0, 0, FUDGE_BSIZE, buffer);

    call_close(CALL_I0);

    tokenize(&infix, strtbl, count, buffer);

    if (stack.size)
        return;

    translate(&postfix, &infix, &stack);

    if (stack.size)
        return;

    parse(&postfix, &stack);

}

