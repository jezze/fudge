#include <fudge.h>

static unsigned int walk_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_walk(index, CALL_DR, count - 1, buffer + 1);

    return call_walk(index, indexw, count, buffer);

}

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

    unsigned int head;
    unsigned int size;
    struct token *table;

};

static void tokenlist_init(struct tokenlist *list, unsigned int size, struct token *table)
{

    memory_clear(list, sizeof (struct tokenlist));

    list->size = size;
    list->table = table;

}

static void tokenlist_push(struct tokenlist *list, struct token *token)
{

    list->table[list->head].type = token->type;
    list->table[list->head].str = token->str;

    if (list->head < list->size)
        list->head++;

}

static struct token *tokenlist_pop(struct tokenlist *list)
{

    if (!list->head)
        return 0;

    list->head--;

    return &list->table[list->head];

}

static void tokenlist_add(struct tokenlist *list, unsigned int type, char *str)
{

    struct token token;

    token.type = type;
    token.str = str;

    tokenlist_push(list, &token);

}

static unsigned int precedence(struct token *token)
{

    switch (token->type)
    {

    case END:
        return 0;

    case PIPE:
        return 1;

    case IN:
    case OUT:
        return 2;

    }

    return 0;

}

static void tokenize(struct tokenlist *infix, struct buffer *stringtable, unsigned int count, char *buffer)
{

    unsigned int i;
    unsigned int ident;
    unsigned int identcount = 0;

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
            tokenlist_add(infix, END, 0);

            break;

        default:
            ident = 1;

            if (!identcount)
                tokenlist_add(infix, IDENT, (char *)stringtable->memory + stringtable->head);

            break;

        }

        if (ident)
        {

            buffer_pushlifo(stringtable, 1, &c);

            identcount++;

        }

        else
        {

            if (identcount)
            {

                c = '\0';

                buffer_pushlifo(stringtable, 1, &c);

                identcount = 0;

            }

        }

    }

}

static void translate(struct tokenlist *postfix, struct tokenlist *infix, struct tokenlist *stack)
{

    unsigned int i;

    for (i = 0; i < infix->head; i++)
    {

        struct token *token = &infix->table[i];
        struct token *t;

        if (token->type == IDENT)
        {

            tokenlist_push(postfix, token);

            continue;

        }

        if (token->type == END || token->type == PIPE)
        {

            while ((t = tokenlist_pop(stack)))
                tokenlist_push(postfix, t);

            tokenlist_push(postfix, token);

            continue;

        }

        while ((t = tokenlist_pop(stack)))
        {

            if (precedence(token) > precedence(t))
            {

                tokenlist_push(stack, t);

                break;

            }

            tokenlist_push(postfix, t);

        }

        tokenlist_push(stack, token);

    }

}

static void parse(struct tokenlist *postfix, struct tokenlist *stack)
{

    unsigned int i;

    call_walk(CALL_I1, CALL_I0, 0, 0);
    call_walk(CALL_O1, CALL_O0, 0, 0);

    for (i = 0; i < postfix->head; i++)
    {

        struct token *token = &postfix->table[i];
        struct token *t;

        switch (token->type)
        {

        case IDENT:
            tokenlist_push(stack, token);

            break;

        case IN:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_I1, CALL_DW, ascii_length(t->str), t->str))
                return;

            break;

        case OUT:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_O1, CALL_DW, ascii_length(t->str), t->str))
                return;

            break;

        case PIPE:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_P0, CALL_L0, ascii_length(t->str), t->str))
                return;

            if (!walk_path(CALL_L1, CALL_DW, 13, "/system/pipe/"))
                return;

            call_open(CALL_L1);
            call_walk(CALL_O1, CALL_L1, 1, "0");
            call_spawn(CALL_P0);
            call_walk(CALL_I1, CALL_L1, 1, "1");
            call_walk(CALL_O1, CALL_O0, 0, 0);
            call_close(CALL_L1);

            break;

        case END:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_P0, CALL_L0, ascii_length(t->str), t->str))
                return;

            call_spawn(CALL_P0);
            call_walk(CALL_I1, CALL_I0, 0, 0);
            call_walk(CALL_O1, CALL_O0, 0, 0);

            break;

        }

    }

}

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count, roff;
    char stringdata[32768];
    struct buffer stringtable;
    struct token infixdata[1024];
    struct token postfixdata[1024];
    struct token stackdata[8];
    struct tokenlist infix;
    struct tokenlist postfix;
    struct tokenlist stack;

    buffer_init(&stringtable, 1, 32768, stringdata);
    tokenlist_init(&infix, 1024, infixdata);
    tokenlist_init(&postfix, 1024, postfixdata);
    tokenlist_init(&stack, 8, stackdata);

    if (!call_walk(CALL_L0, CALL_DR, 4, "bin/"))
        return;

    call_open(CALL_I0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += count)
        tokenize(&infix, &stringtable, count, buffer);

    call_close(CALL_I0);

    if (stack.head)
        return;

    translate(&postfix, &infix, &stack);

    if (stack.head)
        return;

    parse(&postfix, &stack);

}

