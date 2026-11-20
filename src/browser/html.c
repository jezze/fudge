#include <fudge.h>
#include "html.h"

#define NAMESIZE                        32
#define TAGS                            34
#define ENTITIES                        48

struct parser
{

    struct html_document *document;
    char *source;
    unsigned int count;
    unsigned int offset;
    unsigned int stack[HTML_DEPTH];
    unsigned int depth;

};

struct entity
{

    char *name;
    unsigned int codepoint;

};

struct reference
{

    unsigned int length;
    unsigned int codepoint;

};

static char *tags[TAGS] = {"", "a", "body", "br", "head", "html", "img", "input", "li", "link", "ol", "p", "pre", "script", "select", "style", "table", "td", "textarea", "th", "title", "tr", "ul", "font", "option", "dt", "dd", "dl", "tbody", "thead", "tfoot", "button", "hr", "caption"};
static struct entity entities[ENTITIES] = {
    {"amp", 38}, {"lt", 60}, {"gt", 62}, {"quot", 34}, {"apos", 39}, {"nbsp", 160}, {"copy", 169}, {"reg", 174},
    {"trade", 0x2122}, {"mdash", 0x2014}, {"ndash", 0x2013}, {"hellip", 0x2026}, {"laquo", 171}, {"raquo", 187}, {"lsquo", 0x2018}, {"rsquo", 0x2019},
    {"ldquo", 0x201C}, {"rdquo", 0x201D}, {"bull", 0x2022}, {"middot", 183}, {"euro", 0x20AC}, {"deg", 176}, {"times", 215}, {"divide", 247},
    {"plusmn", 177}, {"sect", 167}, {"para", 182}, {"auml", 228}, {"ouml", 246}, {"uuml", 252}, {"Auml", 196}, {"Ouml", 214},
    {"Uuml", 220}, {"aring", 229}, {"Aring", 197}, {"eacute", 233}, {"egrave", 232}, {"szlig", 223}, {"frac12", 189}, {"pound", 163},
    {"yen", 165}, {"cent", 162}, {"iexcl", 161}, {"iquest", 191}, {"larr", 0x2190}, {"rarr", 0x2192}, {"shy", 173}, {"ensp", 0x2002}
};

static unsigned int inlist(char *list, char *name)
{

    unsigned int length = cstring_length(name);
    unsigned int i;

    for (i = 0; list[i]; i++)
    {

        if (list[i] == '|' && buffer_match(list + i + 1, name, length) && list[i + 1 + length] == '|')
            return 1;

    }

    return 0;

}

static unsigned int isnamechar(char c)
{

    return cstring_isalpha(c) || cstring_isdigit(c) || c == '-' || c == ':' || c == '_';

}

static unsigned int isspace(char c)
{

    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f';

}

static char lower(char c)
{

    return (c >= 'A' && c <= 'Z') ? c + 32 : c;

}

static unsigned int matchlower(char *text, unsigned int count, char *name)
{

    unsigned int length = cstring_length(name);
    unsigned int i;

    if (count < length)
        return 0;

    for (i = 0; i < length; i++)
    {

        if (lower(text[i]) != name[i])
            return 0;

    }

    return 1;

}

static unsigned int addbytes(struct html_document *document, char *text, unsigned int count)
{

    if (document->textcount + count + 1 > HTML_TEXTSIZE)
    {

        document->full = 1;

        return 0;

    }

    buffer_copy(document->text + document->textcount, text, count);

    document->textcount += count;

    return 1;

}

static unsigned int endstring(struct html_document *document)
{

    char zero = 0;

    return addbytes(document, &zero, 1);

}

static unsigned int addname(struct html_document *document, char *text, unsigned int count)
{

    unsigned int offset = document->textcount;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c = lower(text[i]);

        if (!addbytes(document, &c, 1))
            return 0;

    }

    return (endstring(document)) ? offset : 0;

}

static struct reference reference(unsigned int length, unsigned int codepoint)
{

    struct reference result;

    result.length = length;
    result.codepoint = codepoint;

    return result;

}

static struct reference readentity(char *text, unsigned int count)
{

    unsigned int i;

    if (count > 2 && text[1] == '#')
    {

        unsigned int base = (text[2] == 'x' || text[2] == 'X') ? 16 : 10;
        unsigned int start = (base == 16) ? 3 : 2;
        unsigned int value = 0;

        for (i = start; i < count && i < 12; i++)
        {

            char c = lower(text[i]);

            if (c >= '0' && c <= '9')
                value = value * base + (c - '0');
            else if (base == 16 && c >= 'a' && c <= 'f')
                value = value * base + (c - 'a' + 10);
            else
                break;

        }

        if (i == start)
            return reference(0, 0);

        return reference((i < count && text[i] == ';') ? i + 1 : i, (value && value < 0x110000) ? value : 0xFFFD);

    }

    for (i = 0; i < ENTITIES; i++)
    {

        unsigned int length = cstring_length(entities[i].name);

        if (length + 1 < count && buffer_match(text + 1, entities[i].name, length) && !isnamechar(text[1 + length]))
            return reference((text[1 + length] == ';') ? length + 2 : length + 1, entities[i].codepoint);

    }

    return reference(0, 0);

}

static void adddecoded(struct html_document *document, char *text, unsigned int count)
{

    unsigned int i = 0;

    while (i < count)
    {

        struct reference entity = (text[i] == '&') ? readentity(text + i, count - i) : reference(0, 0);

        if (entity.length)
        {

            char encoded[4];

            if (entity.codepoint != 173)
                addbytes(document, encoded, utf8_write(encoded, 4, entity.codepoint));

            i += entity.length;

        }

        else
        {

            if (text[i] != '\r')
                addbytes(document, text + i, 1);

            i++;

        }

    }

}

static unsigned int newnode(struct html_document *document, unsigned int type)
{

    struct html_node *node;

    if (document->nodecount >= HTML_NODES)
    {

        document->full = 1;

        return 0;

    }

    node = &document->nodes[document->nodecount];

    buffer_clear(node, sizeof (struct html_node));

    node->type = type;

    return document->nodecount++;

}

static void appendchild(struct html_document *document, unsigned int parent, unsigned int child)
{

    struct html_node *pnode = &document->nodes[parent];
    struct html_node *cnode = &document->nodes[child];

    cnode->parent = parent;
    cnode->prev = pnode->last;

    if (pnode->last)
        document->nodes[pnode->last].next = child;
    else
        pnode->first = child;

    pnode->last = child;

}

static unsigned int findtag(char *name)
{

    unsigned int i;

    for (i = 1; i < TAGS; i++)
    {

        if (cstring_match(tags[i], name))
            return i;

    }

    return HTML_TAG_OTHER;

}

static unsigned int newelement(struct html_document *document, char *name)
{

    unsigned int node = newnode(document, HTML_NODE_ELEMENT);

    if (node)
    {

        document->nodes[node].name = addname(document, name, cstring_length(name));
        document->nodes[node].tag = findtag(name);

    }

    return node;

}

static unsigned int current(struct parser *parser)
{

    return (parser->depth) ? parser->stack[parser->depth - 1] : 0;

}

static void push(struct parser *parser, unsigned int node)
{

    if (parser->depth < HTML_DEPTH)
        parser->stack[parser->depth++] = node;

}

static char *stackname(struct parser *parser, unsigned int index)
{

    return html_getstring(parser->document, parser->document->nodes[parser->stack[index]].name);

}

static unsigned int findopen(struct parser *parser, char *names, char *stops)
{

    unsigned int i;

    for (i = parser->depth; i > 0; i--)
    {

        char *name = stackname(parser, i - 1);

        if (inlist(names, name))
            return i;

        if (inlist(stops, name) || parser->stack[i - 1] == parser->document->body)
            return 0;

    }

    return 0;

}

static void closeopen(struct parser *parser, char *names, char *stops)
{

    unsigned int index = findopen(parser, names, stops);

    if (index)
        parser->depth = index - 1;

}

static unsigned int ensurehtml(struct parser *parser)
{

    struct html_document *document = parser->document;

    if (document->html)
        return 0;

    document->html = newelement(document, "html");

    if (document->html)
    {

        appendchild(document, 0, document->html);
        push(parser, document->html);

    }

    return document->html;

}

static void ensurehead(struct parser *parser)
{

    struct html_document *document = parser->document;

    ensurehtml(parser);

    if (!document->head && document->html)
    {

        document->head = newelement(document, "head");

        if (document->head)
            appendchild(document, document->html, document->head);

    }

}

static unsigned int ensurebody(struct parser *parser)
{

    struct html_document *document = parser->document;

    ensurehtml(parser);

    if (document->body || !document->html)
        return 0;

    parser->depth = 1;
    document->body = newelement(document, "body");

    if (document->body)
    {

        appendchild(document, document->html, document->body);
        push(parser, document->body);

    }

    return document->body;

}

static void addtext(struct parser *parser, unsigned int parent, char *text, unsigned int count, unsigned int decode)
{

    struct html_document *document = parser->document;
    struct html_node *last = &document->nodes[document->nodes[parent].last];
    unsigned int offset = document->textcount;

    if (decode)
        adddecoded(document, text, count);
    else
        addbytes(document, text, count);

    if (document->textcount == offset)
        return;

    if (document->nodes[parent].last && last->type == HTML_NODE_TEXT && last->text + last->length == offset)
    {

        last->length += document->textcount - offset;

    }

    else
    {

        unsigned int node = newnode(document, HTML_NODE_TEXT);

        if (node)
        {

            document->nodes[node].text = offset;
            document->nodes[node].length = document->textcount - offset;

            appendchild(document, parent, node);

        }

    }

}

static unsigned int isblank(char *text, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!isspace(text[i]))
            return 0;

    }

    return 1;

}

static void inserttext(struct parser *parser, char *text, unsigned int count)
{

    if (!parser->document->body)
    {

        if (isblank(text, count))
            return;

        ensurebody(parser);

    }

    if (current(parser))
        addtext(parser, current(parser), text, count, 1);

}

static unsigned int readname(struct parser *parser, char name[NAMESIZE])
{

    unsigned int length = 0;

    while (parser->offset < parser->count && isnamechar(parser->source[parser->offset]))
    {

        if (length < NAMESIZE - 1)
            name[length++] = lower(parser->source[parser->offset]);

        parser->offset++;

    }

    name[length] = '\0';

    return length;

}

static void skipspace(struct parser *parser)
{

    while (parser->offset < parser->count && isspace(parser->source[parser->offset]))
        parser->offset++;

}

static void addattribute(struct parser *parser, unsigned int node, char *name, unsigned int namelength, char *value, unsigned int valuelength)
{

    struct html_document *document = parser->document;
    struct html_attribute *attribute;

    if (!node)
        return;

    if (document->attributecount >= HTML_ATTRIBUTES || (document->nodes[node].attributes && document->nodes[node].attribute + document->nodes[node].attributes != document->attributecount))
    {

        document->full = 1;

        return;

    }

    if (!document->nodes[node].attributes)
        document->nodes[node].attribute = document->attributecount;

    attribute = &document->attributes[document->attributecount];
    attribute->name = addname(document, name, namelength);
    attribute->value = document->textcount;

    adddecoded(document, value, valuelength);

    if (attribute->name && endstring(document))
    {

        document->attributecount++;
        document->nodes[node].attributes++;

    }

}

static unsigned int readattributes(struct parser *parser, unsigned int node)
{

    char *source = parser->source;
    unsigned int selfclosing = 0;

    while (parser->offset < parser->count)
    {

        unsigned int namestart;
        unsigned int nameend;
        unsigned int valuestart;
        unsigned int valueend;

        skipspace(parser);

        if (parser->offset >= parser->count)
            break;

        if (source[parser->offset] == '>')
        {

            parser->offset++;

            break;

        }

        if (source[parser->offset] == '/')
        {

            selfclosing = 1;
            parser->offset++;

            continue;

        }

        namestart = parser->offset;

        while (parser->offset < parser->count && !isspace(source[parser->offset]) && source[parser->offset] != '=' && source[parser->offset] != '>' && (source[parser->offset] != '/' || parser->offset == namestart))
            parser->offset++;

        nameend = parser->offset;
        selfclosing = 0;

        skipspace(parser);

        valuestart = valueend = parser->offset;

        if (parser->offset < parser->count && source[parser->offset] == '=')
        {

            parser->offset++;

            skipspace(parser);

            if (parser->offset < parser->count && (source[parser->offset] == '"' || source[parser->offset] == '\''))
            {

                char quote = source[parser->offset++];

                valuestart = parser->offset;

                while (parser->offset < parser->count && source[parser->offset] != quote)
                    parser->offset++;

                valueend = parser->offset;

                if (parser->offset < parser->count)
                    parser->offset++;

            }

            else
            {

                valuestart = parser->offset;

                while (parser->offset < parser->count && !isspace(source[parser->offset]) && source[parser->offset] != '>')
                    parser->offset++;

                valueend = parser->offset;

            }

        }

        if (nameend > namestart)
            addattribute(parser, node, source + namestart, nameend - namestart, source + valuestart, valueend - valuestart);

    }

    return selfclosing;

}

static void readraw(struct parser *parser, unsigned int node, char *name, unsigned int decode)
{

    unsigned int start = parser->offset;
    unsigned int length = cstring_length(name);

    while (parser->offset < parser->count)
    {

        if (parser->source[parser->offset] == '<' && parser->offset + 1 < parser->count && parser->source[parser->offset + 1] == '/' && matchlower(parser->source + parser->offset + 2, parser->count - parser->offset - 2, name) && (parser->offset + 2 + length >= parser->count || !isnamechar(parser->source[parser->offset + 2 + length])))
            break;

        parser->offset++;

    }

    if (node && parser->offset > start)
        addtext(parser, node, parser->source + start, parser->offset - start, decode);

    while (parser->offset < parser->count && parser->source[parser->offset] != '>')
        parser->offset++;

    if (parser->offset < parser->count)
        parser->offset++;

}

static void closeimplied(struct parser *parser, char *name)
{

    if (inlist("|address|article|aside|blockquote|center|details|dialog|div|dl|fieldset|figcaption|figure|footer|form|h1|h2|h3|h4|h5|h6|header|hgroup|hr|main|menu|nav|ol|p|pre|section|table|ul|li|dd|dt|", name))
        closeopen(parser, "|p|", "|button|td|th|");

    if (cstring_match(name, "li"))
        closeopen(parser, "|li|", "|ul|ol|menu|");

    if (inlist("|dt|dd|", name))
        closeopen(parser, "|dt|dd|", "|dl|");

    if (cstring_match(name, "tr"))
        closeopen(parser, "|tr|", "|table|");

    if (inlist("|td|th|", name))
        closeopen(parser, "|td|th|", "|tr|table|");

    if (inlist("|tbody|thead|tfoot|", name))
        closeopen(parser, "|tbody|thead|tfoot|", "|table|");

    if (inlist("|option|optgroup|", name))
        closeopen(parser, "|option|", "|select|");

    if (inlist("|h1|h2|h3|h4|h5|h6|", name) && parser->depth && inlist("|h1|h2|h3|h4|h5|h6|", stackname(parser, parser->depth - 1)))
        parser->depth--;

    if (cstring_match(name, "a"))
        closeopen(parser, "|a|", "");

}

static void readstarttag(struct parser *parser)
{

    struct html_document *document = parser->document;
    char name[NAMESIZE];
    unsigned int node = 0;
    unsigned int parent;
    unsigned int selfclosing;

    parser->offset++;

    readname(parser, name);

    if (cstring_match(name, "html"))
    {

        if (!document->html)
            node = ensurehtml(parser);

        readattributes(parser, node);

        return;

    }

    if (cstring_match(name, "head"))
    {

        if (!document->body)
            ensurehead(parser);

        readattributes(parser, 0);

        return;

    }

    if (cstring_match(name, "body"))
    {

        if (!document->body)
            node = ensurebody(parser);

        readattributes(parser, node);

        return;

    }

    if (inlist("|title|style|script|meta|link|base|", name) && !document->body)
    {

        ensurehead(parser);

        parent = document->head;

    }

    else
    {

        ensurebody(parser);
        closeimplied(parser, name);

        parent = current(parser);

    }

    if (parent)
    {

        node = newelement(document, name);

        if (node)
            appendchild(document, parent, node);

    }

    selfclosing = readattributes(parser, node);

    if (inlist("|script|style|", name))
        readraw(parser, node, name, 0);
    else if (inlist("|title|textarea|", name))
        readraw(parser, node, name, 1);
    else if (node && !selfclosing && !inlist("|area|base|br|col|embed|hr|img|input|link|meta|param|source|track|wbr|", name))
        push(parser, node);

    if (inlist("|pre|listing|", name) && parser->offset < parser->count && parser->source[parser->offset] == '\n')
        parser->offset++;

}

static void readendtag(struct parser *parser)
{

    char name[NAMESIZE + 2];

    parser->offset += 2;

    name[0] = '|';

    readname(parser, name + 1);

    while (parser->offset < parser->count && parser->source[parser->offset] != '>')
        parser->offset++;

    if (parser->offset < parser->count)
        parser->offset++;

    if (cstring_match(name + 1, "br"))
    {

        ensurebody(parser);

        if (current(parser))
        {

            unsigned int node = newelement(parser->document, "br");

            if (node)
                appendchild(parser->document, current(parser), node);

        }

        return;

    }

    if (!inlist("|html|body|head|", name + 1))
    {

        unsigned int length = cstring_length(name);

        name[length] = '|';
        name[length + 1] = '\0';

        closeopen(parser, name, "");

    }

}

static void skipto(struct parser *parser, char *end)
{

    unsigned int length = cstring_length(end);

    while (parser->offset < parser->count && !(parser->offset + length <= parser->count && buffer_match(parser->source + parser->offset, end, length)))
        parser->offset++;

    parser->offset = (parser->offset + length <= parser->count) ? parser->offset + length : parser->count;

}

static void reset(struct html_document *document)
{

    document->nodecount = 0;
    document->attributecount = 0;
    document->textcount = 1;
    document->html = 0;
    document->head = 0;
    document->body = 0;
    document->full = 0;
    document->text[0] = '\0';

    newnode(document, HTML_NODE_DOCUMENT);

}

char *html_getstring(struct html_document *document, unsigned int offset)
{

    return document->text + offset;

}

char *html_getattribute(struct html_document *document, unsigned int node, char *name)
{

    struct html_node *element = &document->nodes[node];
    unsigned int i;

    for (i = 0; i < element->attributes; i++)
    {

        struct html_attribute *attribute = &document->attributes[element->attribute + i];

        if (cstring_match(html_getstring(document, attribute->name), name))
            return html_getstring(document, attribute->value);

    }

    return 0;

}

static unsigned int addvalue(struct html_document *document, char *value)
{

    unsigned int offset = document->textcount;

    if (!addbytes(document, value, cstring_length(value)) || !endstring(document))
        return 0;

    return offset;

}

static struct html_attribute *findattribute(struct html_document *document, unsigned int node, char *name)
{

    struct html_node *element = &document->nodes[node];
    unsigned int i;

    for (i = 0; i < element->attributes; i++)
    {

        struct html_attribute *attribute = &document->attributes[element->attribute + i];

        if (cstring_match(html_getstring(document, attribute->name), name))
            return attribute;

    }

    return 0;

}

unsigned int html_setattribute(struct html_document *document, unsigned int node, char *name, char *value)
{

    struct html_node *element = &document->nodes[node];
    struct html_attribute *attribute = findattribute(document, node, name);
    unsigned int offset;

    if (attribute)
    {

        offset = addvalue(document, value);

        if (offset)
            attribute->value = offset;

        return offset != 0;

    }

    if (document->attributecount + element->attributes + 1 > HTML_ATTRIBUTES)
        return 0;

    if (element->attributes && element->attribute + element->attributes != document->attributecount)
    {

        buffer_copy(&document->attributes[document->attributecount], &document->attributes[element->attribute], sizeof (struct html_attribute) * element->attributes);

        element->attribute = document->attributecount;
        document->attributecount += element->attributes;

    }

    if (!element->attributes)
        element->attribute = document->attributecount;

    attribute = &document->attributes[document->attributecount];
    attribute->name = addname(document, name, cstring_length(name));
    attribute->value = addvalue(document, value);

    if (!attribute->name || !attribute->value)
        return 0;

    document->attributecount++;
    element->attributes++;

    return 1;

}

void html_removeattribute(struct html_document *document, unsigned int node, char *name)
{

    struct html_node *element = &document->nodes[node];
    unsigned int found = 0;
    unsigned int i;

    for (i = 0; i < element->attributes; i++)
    {

        struct html_attribute *attribute = &document->attributes[element->attribute + i];

        if (found)
        {

            document->attributes[element->attribute + i - 1].name = attribute->name;
            document->attributes[element->attribute + i - 1].value = attribute->value;

        }

        else if (cstring_match(html_getstring(document, attribute->name), name))
        {

            found = 1;

        }

    }

    if (found)
        element->attributes--;

}

unsigned int html_gettitle(struct html_document *document)
{

    unsigned int node;

    for (node = 1; node < document->nodecount; node++)
    {

        if (document->nodes[node].tag == HTML_TAG_TITLE)
            return document->nodes[node].first;

    }

    return 0;

}

unsigned int html_next(struct html_document *document, unsigned int node, unsigned int root)
{

    if (document->nodes[node].first)
        return document->nodes[node].first;

    while (node && node != root)
    {

        if (document->nodes[node].next)
            return document->nodes[node].next;

        node = document->nodes[node].parent;

    }

    return 0;

}

void html_parse(struct html_document *document, char *source, unsigned int count)
{

    struct parser parser;

    reset(document);

    parser.document = document;
    parser.source = source;
    parser.count = count;
    parser.offset = 0;
    parser.depth = 0;

    while (parser.offset < count)
    {

        char *text = source + parser.offset;
        unsigned int remaining = count - parser.offset;
        unsigned int length;

        if (remaining > 3 && buffer_match(text, "<!--", 4))
        {

            parser.offset += 4;

            skipto(&parser, "-->");

            continue;

        }

        if (remaining > 1 && text[0] == '<' && (text[1] == '!' || text[1] == '?'))
        {

            skipto(&parser, ">");

            continue;

        }

        if (remaining > 1 && text[0] == '<' && cstring_isalpha(text[1]))
        {

            readstarttag(&parser);

            continue;

        }

        if (remaining > 2 && text[0] == '<' && text[1] == '/' && cstring_isalpha(text[2]))
        {

            readendtag(&parser);

            continue;

        }

        for (length = 1; length < remaining && text[length] != '<'; length++);

        inserttext(&parser, text, length);

        parser.offset += length;

    }

    ensurebody(&parser);

}

void html_parsetext(struct html_document *document, char *source, unsigned int count)
{

    struct parser parser;
    unsigned int pre;

    reset(document);

    parser.document = document;
    parser.depth = 0;

    ensurebody(&parser);

    pre = newelement(document, "pre");

    if (pre && document->body)
    {

        appendchild(document, document->body, pre);
        addtext(&parser, pre, source, count, 0);

    }

}
