#include <abi.h>
#include <fudge.h>

#define TOKENEND                        1
#define TOKENIDENT                      2
#define TOKENIN                         3
#define TOKENOUT                        4
#define TOKENPIPE                       5

static unsigned int walk_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_walk(index, CALL_PR, count - 1, buffer + 1);

    return call_walk(index, indexw, count, buffer);

}

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

    case TOKENEND:
        return 0;

    case TOKENPIPE:
        return 1;

    case TOKENIN:
    case TOKENOUT:
        return 2;

    }

    return 0;

}

static void tokenize(struct tokenlist *infix, struct buffer *stringtable, unsigned int count, char *buffer)
{

    unsigned int i;
    unsigned int ident;
    unsigned int identquote = 0;
    unsigned int identcount = 0;

    for (i = 0; i < count; i++)
    {

        char c = buffer[i];

        ident = 0;

        switch (c)
        {

        case '<':
            tokenlist_add(infix, TOKENIN, 0);

            break;

        case '>':
            tokenlist_add(infix, TOKENOUT, 0);

            break;

        case '|':
            tokenlist_add(infix, TOKENPIPE, 0);

            break;

        case ';':
        case '\n':
            tokenlist_add(infix, TOKENEND, 0);

            break;

        default:
            ident = 1;

            break;

        }

        if (c == '"')
            identquote = !identquote;

        if (!identquote && (c == ' ' || c == '\t'))
            continue;

        if (ident)
        {

            if (!identcount)
                tokenlist_add(infix, TOKENIDENT, (char *)stringtable->memory + stringtable->head);

            buffer_wcfifo(stringtable, 1, &c);

            identcount++;

        }

        else
        {

            if (identcount)
            {

                c = '\0';

                buffer_wcfifo(stringtable, 1, &c);

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

        if (token->type == TOKENIDENT)
        {

            tokenlist_push(postfix, token);

            continue;

        }

        if (token->type == TOKENEND || token->type == TOKENPIPE)
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

    for (i = 0; i < postfix->head; i++)
    {

        struct token *token = &postfix->table[i];
        struct token *t;

        switch (token->type)
        {

        case TOKENIDENT:
            tokenlist_push(stack, token);

            break;

        case TOKENIN:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (ascii_length(t->str) >= 2 && t->str[0] == '"')
            {

                if (!call_walk(CALL_L1, CALL_PR, 12, "system/pipe/"))
                    return;

                call_walk(CALL_L2, CALL_L1, 1, "0");
                call_walk(CALL_C0, CALL_L1, 1, "1");
                call_write(CALL_L2, 0, ascii_length(t->str) - 2, t->str + 1);

            }

            else
            {

                if (!walk_path(CALL_C0, CALL_PW, ascii_length(t->str), t->str))
                    return;

            }

            break;

        case TOKENOUT:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_CO, CALL_PW, ascii_length(t->str), t->str))
                return;

            break;

        case TOKENPIPE:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_CP, CALL_L0, ascii_length(t->str), t->str))
                return;

            if (!call_walk(CALL_L1, CALL_PR, 12, "system/pipe/"))
                return;

            call_walk(CALL_C0, CALL_L1, 1, "1");
            call_spawn();
            call_walk(CALL_C0, CALL_P0, 0, 0);
            call_walk(CALL_CO, CALL_L1, 1, "0");

            break;

        case TOKENEND:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_CP, CALL_L0, ascii_length(t->str), t->str))
                return;

            call_spawn();
            call_walk(CALL_C0, CALL_P0, 0, 0);
            call_walk(CALL_CO, CALL_PO, 0, 0);

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

    if (!call_walk(CALL_L0, CALL_PR, 4, "bin/"))
        return;

    call_open(CALL_P0);

    for (roff = 0; (count = call_read(CALL_P0, roff, FUDGE_BSIZE, buffer)); roff += count)
        tokenize(&infix, &stringtable, count, buffer);

    call_close(CALL_P0);

    if (stack.head)
        return;

    translate(&postfix, &infix, &stack);

    if (stack.head)
        return;

    parse(&postfix, &stack);

}

