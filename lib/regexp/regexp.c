#include <fudge/ascii.h>
#include "regexp.h"

#define RE_MAX (~(unsigned int)0)

struct re_data
{

    unsigned int (*cont)(char *, char *, struct re_data *, unsigned int);
    struct re_data *data;
    char *pat;
    char *str;
    unsigned int ok;
    unsigned int incarn;

};

static unsigned int re_seq(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o);
static unsigned int re_special(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o);
static unsigned int re_class1(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o);
static unsigned int re_single(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o, unsigned int i);

static unsigned int re_eat1(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o)
{

    return c(p, s + (o ? 1 : 0), d, o);

}

static unsigned int re_seq_atom_tail(char *p, char *s, struct re_data *d, unsigned int o)
{

    if (*p != ')')
        return RE_E_SYNTAX;

    return d->cont(p + 1, s, d->data, o);

}

static unsigned int re_class1_tail(char *p, char *s, struct re_data *d, unsigned int o)
{

    return re_class1(p, d->str, d->cont, d->data, d->ok || o);

}

static unsigned int re_class1(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o)
{

    struct re_data d2;

    d2.cont = c;
    d2.data = d;
    d2.str = s;
    d2.ok = o;

    switch (*p)
    {

    case '\\':
        return re_special(p + 1, s, re_class1_tail, &d2, 1);

    case ']':
        return c(p + 1, s, d, o);

    case '\0':
        return RE_E_SYNTAX;

    case '-': case '[':
        return RE_E_NOTIMPL;

    default:
        if (p[1] != '-')
            return re_eat1(p + 1, s, re_class1_tail, &d2, (*s == *p));

        switch (p[2])
        {

        case '\0':
            return RE_E_SYNTAX;

        case '[': case ']': case '\\': case '-':
            return RE_E_NOTIMPL;

        }

        return re_eat1(p + 3, s, re_class1_tail, &d2, (*s >= p[0]) && (*s <= p[2]));

    }

}

static unsigned int re_class_tail(char *p, char *s, struct re_data *d, unsigned int o)
{

    return re_eat1(p, s, d->cont, d->data, d->ok && o);

}

static unsigned int re_class_neg_tail(char *p, char *s, struct re_data *d, unsigned int o)
{

    return re_eat1(p, s, d->cont, d->data, d->ok && (*s != '\0') && !o);

}

static unsigned int re_class(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o)
{

    struct re_data d2;

    d2.cont = c;
    d2.data = d;
    d2.str = s;
    d2.ok = o;

    if (*p == '^')
        return re_class1(p + 1, s, re_class_neg_tail, &d2, 0);

    return re_class1(p, s, re_class_tail, &d2, 0);

}

static unsigned int re_const_class_tail(char *p, char *s, struct re_data *d, unsigned int o)
{

    return d->cont(d->pat, s, d->data, o);

}

static unsigned int re_const_class(char *p1, char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o)
{

    struct re_data d2;

    d2.cont = c;
    d2.data = d;
    d2.pat = p;

    return re_class(p1, s, re_const_class_tail, &d2, o);

}

static unsigned int re_special(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o)
{

    switch (*p)
    {

    case 's':
        return re_const_class(" \t\n]", p + 1, s, c, d, o);

    case 'S':
        return re_const_class("^ \t\n]", p + 1, s, c, d, o);

    case 'd':
        return re_const_class("0-9]", p + 1, s, c, d, o);

    case 'D':
        return re_const_class("^0-9]", p + 1, s, c, d, o);

    case 'n':
        return re_eat1(p + 1, s, c, d, o && (*s == '\n'));

    case 't':
        return re_eat1(p + 1, s, c, d, o && (*s == '\t'));

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
        return re_eat1(p + 1, s, c, d, o && (*s == *p));

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
        return RE_E_NOTIMPL;

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
        return RE_E_SYNTAX;

    }

}

static unsigned int re_atom(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o)
{

    switch (*p)
    {

    default:
        return re_eat1(p + 1, s, c, d, o && (*s == *p));

    case '.':
        return re_const_class("^\n]", p + 1, s, c, d, o);

    case '\\':
        return re_special(p + 1, s, c, d, o);

    case '[':
        return re_class(p + 1, s, c, d, o);

    case '(':
    {

        struct re_data d2;

        d2.cont = c;
        d2.data = d;

        return re_seq(p + 1, s, re_seq_atom_tail, &d2, o);

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
        return RE_E_SYNTAX;

    }

}

static unsigned int re_single_tail(char *p, char *s, struct re_data *d, unsigned int o)
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
            hi = RE_MAX;

        p++;

        break;

    case '{':
    {

        char *q;
        unsigned int len;

        len = ascii_length(p + 1);
        lo = hi = ascii_rvalue(p + 1, len, 10, 0);
        q = p + len;

        if ((p + 1) == q)
            return RE_E_SYNTAX;

        if (*q == ',')
        {

            len = ascii_length(q);
            hi = ascii_rvalue(q + 1, len, 10, 0);
            q = q + len;

            if (q[-1] == ',')
                hi = RE_MAX;

        }

        if ((*q != '}') || (lo > hi))
            return RE_E_SYNTAX;

        p = q + 1;

        break;

    }

    default:
        return d->cont(p, s, d->data, o);

    }

    if ((lo == 0) && (d->incarn == 1))
    {

        unsigned int r2 = re_single_tail(p, d->str, d, d->ok);

        if (r2 != RE_E_FAIL)
            return r2;

    }

    if ((hi >= 1) && (d->incarn >= lo))
    {

        unsigned int r2 = re_single_tail(p, s, d, o);

        if (r2 != RE_E_FAIL)
            return r2;

    }

    if ((d->incarn > lo) && (s == d->str))
        return RE_E_FAIL;

    if (d->incarn < hi)
        return re_single(d->pat, s, d->cont, d->data, o, d->incarn + 1);

    return RE_E_FAIL;

}

static unsigned int re_single(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o, unsigned int i)
{

    struct re_data d2;

    d2.cont = c;
    d2.data = d;
    d2.pat = p;
    d2.str = s;
    d2.ok = o;
    d2.incarn = i;

    return re_atom(p, s, re_single_tail, &d2, o);

}

static unsigned int re_seq_or_tail(char *p, char *s, struct re_data *d, unsigned int o)
{

    unsigned int r2 = d->cont(p, s, d->data, o);

    if (r2 != RE_E_FAIL)
        return r2;

    return d->cont(p, d->str, d->data, d->ok);

}

static unsigned int re_seq_tail(char *p, char *s, struct re_data *d, unsigned int o)
{

    if (*p == '|')
    {

        struct re_data d3;
        struct re_data d2;

        d3.cont = d->cont;
        d3.data = d->data;
        d3.str = s;
        d3.ok = o;
        d2.cont = re_seq_or_tail;
        d2.data = &d3;
        d2.str = d->str;
        d2.ok = d->ok;

        return re_single(p + 1, d->str, re_seq_tail, &d2, d->ok, 1);

    }

    if ((*p == '\0') || (*p == ')'))
        return d->cont(p, s, d->data, o);

    return re_single(p, s, re_seq_tail, d, o, 1);

}

static unsigned int re_seq(char *p, char *s, unsigned int (*c)(char *, char *, struct re_data *, unsigned int), struct re_data *d, unsigned int o)
{

    struct re_data d2;

    d2.cont = c;
    d2.data = d;
    d2.str = s;
    d2.ok = o;

    return re_seq_tail(p, s, &d2, o);

}

static unsigned int re_match_tail(char *p, char *s, struct re_data *d, unsigned int o)
{

    if (*p != '\0')
        return RE_E_SYNTAX;

    return !(o && (*s == '\0'));

}

unsigned int re_match(char *p, char *s)
{

    return re_seq(p, s, re_match_tail, 0, 1);

}

