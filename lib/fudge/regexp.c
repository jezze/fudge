#include "cstring.h"
#include "regexp.h"

#define MAX (~(unsigned int)0)

struct data
{

    unsigned int (*cont)(char *, char *, struct data *, unsigned int);
    struct data *data;
    char *pat;
    char *str;
    unsigned int ok;
    unsigned int incarn;

};

static unsigned int seq(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o);
static unsigned int special(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o);
static unsigned int class1(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o);
static unsigned int single(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o, unsigned int i);

static unsigned int eat1(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o)
{

    return c(p, s + (o ? 1 : 0), d, o);

}

static unsigned int seqatomtail(char *p, char *s, struct data *d, unsigned int o)
{

    if (*p != ')')
        return REGEXP_SYNTAX;

    return d->cont(p + 1, s, d->data, o);

}

static unsigned int class1tail(char *p, char *s, struct data *d, unsigned int o)
{

    return class1(p, d->str, d->cont, d->data, d->ok || o);

}

static unsigned int class1(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o)
{

    struct data d2;

    d2.cont = c;
    d2.data = d;
    d2.str = s;
    d2.ok = o;

    switch (*p)
    {

    case '\\':
        return special(p + 1, s, class1tail, &d2, 1);

    case ']':
        return c(p + 1, s, d, o);

    case '\0':
        return REGEXP_SYNTAX;

    case '-': case '[':
        return REGEXP_NOTIMPL;

    default:
        if (p[1] != '-')
            return eat1(p + 1, s, class1tail, &d2, (*s == *p));

        switch (p[2])
        {

        case '\0':
            return REGEXP_SYNTAX;

        case '[': case ']': case '\\': case '-':
            return REGEXP_NOTIMPL;

        }

        return eat1(p + 3, s, class1tail, &d2, (*s >= p[0]) && (*s <= p[2]));

    }

}

static unsigned int classtail(char *p, char *s, struct data *d, unsigned int o)
{

    return eat1(p, s, d->cont, d->data, d->ok && o);

}

static unsigned int classnegtail(char *p, char *s, struct data *d, unsigned int o)
{

    return eat1(p, s, d->cont, d->data, d->ok && (*s != '\0') && !o);

}

static unsigned int class(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o)
{

    struct data d2;

    d2.cont = c;
    d2.data = d;
    d2.str = s;
    d2.ok = o;

    if (*p == '^')
        return class1(p + 1, s, classnegtail, &d2, 0);

    return class1(p, s, classtail, &d2, 0);

}

static unsigned int constclasstail(char *p, char *s, struct data *d, unsigned int o)
{

    return d->cont(d->pat, s, d->data, o);

}

static unsigned int constclass(char *p1, char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o)
{

    struct data d2;

    d2.cont = c;
    d2.data = d;
    d2.pat = p;

    return class(p1, s, constclasstail, &d2, o);

}

static unsigned int special(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o)
{

    switch (*p)
    {

    case 's':
        return constclass(" \t\n]", p + 1, s, c, d, o);

    case 'S':
        return constclass("^ \t\n]", p + 1, s, c, d, o);

    case 'd':
        return constclass("0-9]", p + 1, s, c, d, o);

    case 'D':
        return constclass("^0-9]", p + 1, s, c, d, o);

    case 'n':
        return eat1(p + 1, s, c, d, o && (*s == '\n'));

    case 't':
        return eat1(p + 1, s, c, d, o && (*s == '\t'));

    case '(':
    case ')':
    case '|':
    case '{':
    case '}':
    case '?':
    case '*':
    case '+':
    case '[':
    case ']':
    case '.':
    case '-':
    case '^':
    case '\\':
    case '$':
        return eat1(p + 1, s, c, d, o && (*s == *p));

    case 'p':
    case 'P':
    case 'r':
    case 'X':
    case 'b':
    case 'c':
    case 'C':
    case 'w':
    case 'W':
    case 'i':
    case 'I':
        return REGEXP_NOTIMPL;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case 'x':
    case 'u':
    case 'U':
    default:
        return REGEXP_SYNTAX;

    }

}

static unsigned int atom(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o)
{

    switch (*p)
    {

    default:
        return eat1(p + 1, s, c, d, o && (*s == *p));

    case '.':
        return constclass("^\n]", p + 1, s, c, d, o);

    case '\\':
        return special(p + 1, s, c, d, o);

    case '[':
        return class(p + 1, s, c, d, o);

    case '(':
    {

        struct data d2;

        d2.cont = c;
        d2.data = d;

        return seq(p + 1, s, seqatomtail, &d2, o);

    }

    case '{':
    case '}':
    case '?':
    case '*':
    case '+':
    case ')':
    case '|':
    case '^':
    case '$':
        return REGEXP_SYNTAX;

    }

}

static unsigned int singletail(char *p, char *s, struct data *d, unsigned int o)
{

    unsigned int lo = 1;
    unsigned int hi = 1;

    switch (*p)
    {

    case '?':
    case '*':
    case '+':
        if (*p != '+')
            lo = 0;

        if (*p != '?')
            hi = MAX;

        p++;

        break;

    case '{':
    {

        char *q;
        unsigned int len;

        len = cstring_length(p + 1);
        lo = hi = cstring_read_value(p + 1, len, 10);
        q = p + len;

        if ((p + 1) == q)
            return REGEXP_SYNTAX;

        if (*q == ',')
        {

            len = cstring_length(q);
            hi = cstring_read_value(q + 1, len, 10);
            q = q + len;

            if (q[-1] == ',')
                hi = MAX;

        }

        if ((*q != '}') || (lo > hi))
            return REGEXP_SYNTAX;

        p = q + 1;

        break;

    }

    default:
        return d->cont(p, s, d->data, o);

    }

    if ((lo == 0) && (d->incarn == 1))
    {

        unsigned int r2 = singletail(p, d->str, d, d->ok);

        if (r2 != REGEXP_FAIL)
            return r2;

    }

    if ((hi >= 1) && (d->incarn >= lo))
    {

        unsigned int r2 = singletail(p, s, d, o);

        if (r2 != REGEXP_FAIL)
            return r2;

    }

    if ((d->incarn > lo) && (s == d->str))
        return REGEXP_FAIL;

    if (d->incarn < hi)
        return single(d->pat, s, d->cont, d->data, o, d->incarn + 1);

    return REGEXP_FAIL;

}

static unsigned int single(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o, unsigned int i)
{

    struct data d2;

    d2.cont = c;
    d2.data = d;
    d2.pat = p;
    d2.str = s;
    d2.ok = o;
    d2.incarn = i;

    return atom(p, s, singletail, &d2, o);

}

static unsigned int seqortail(char *p, char *s, struct data *d, unsigned int o)
{

    unsigned int r2 = d->cont(p, s, d->data, o);

    if (r2 != REGEXP_FAIL)
        return r2;

    return d->cont(p, d->str, d->data, d->ok);

}

static unsigned int seqtail(char *p, char *s, struct data *d, unsigned int o)
{

    if (*p == '|')
    {

        struct data d3;
        struct data d2;

        d3.cont = d->cont;
        d3.data = d->data;
        d3.str = s;
        d3.ok = o;
        d2.cont = seqortail;
        d2.data = &d3;
        d2.str = d->str;
        d2.ok = d->ok;

        return single(p + 1, d->str, seqtail, &d2, d->ok, 1);

    }

    if ((*p == '\0') || (*p == ')'))
        return d->cont(p, s, d->data, o);

    return single(p, s, seqtail, d, o, 1);

}

static unsigned int seq(char *p, char *s, unsigned int (*c)(char *, char *, struct data *, unsigned int), struct data *d, unsigned int o)
{

    struct data d2;

    d2.cont = c;
    d2.data = d;
    d2.str = s;
    d2.ok = o;

    return seqtail(p, s, &d2, o);

}

static unsigned int matchtail(char *p, char *s, struct data *d, unsigned int o)
{

    if (*p != '\0')
        return REGEXP_SYNTAX;

    return !(o && (*s == '\0'));

}

unsigned int regexp_match(char *p, char *s)
{

    return seq(p, s, matchtail, 0, 1);

}

