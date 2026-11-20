#define HTML_NODES                      2048
#define HTML_ATTRIBUTES                 4096
#define HTML_TEXTSIZE                   0x18000
#define HTML_DEPTH                      40
#define HTML_NODE_DOCUMENT              0
#define HTML_NODE_ELEMENT               1
#define HTML_NODE_TEXT                  2
#define HTML_TAG_OTHER                  0
#define HTML_TAG_A                      1
#define HTML_TAG_BODY                   2
#define HTML_TAG_BR                     3
#define HTML_TAG_HEAD                   4
#define HTML_TAG_HTML                   5
#define HTML_TAG_IMG                    6
#define HTML_TAG_INPUT                  7
#define HTML_TAG_LI                     8
#define HTML_TAG_LINK                   9
#define HTML_TAG_OL                     10
#define HTML_TAG_P                      11
#define HTML_TAG_PRE                    12
#define HTML_TAG_SCRIPT                 13
#define HTML_TAG_SELECT                 14
#define HTML_TAG_STYLE                  15
#define HTML_TAG_TABLE                  16
#define HTML_TAG_TD                     17
#define HTML_TAG_TEXTAREA               18
#define HTML_TAG_TH                     19
#define HTML_TAG_TITLE                  20
#define HTML_TAG_TR                     21
#define HTML_TAG_UL                     22
#define HTML_TAG_FONT                   23
#define HTML_TAG_OPTION                 24
#define HTML_TAG_DT                     25
#define HTML_TAG_DD                     26
#define HTML_TAG_DL                     27
#define HTML_TAG_TBODY                  28
#define HTML_TAG_THEAD                  29
#define HTML_TAG_TFOOT                  30
#define HTML_TAG_BUTTON                 31
#define HTML_TAG_HR                     32
#define HTML_TAG_CAPTION                33

struct html_node
{

    unsigned int type;
    unsigned int tag;
    unsigned int name;
    unsigned int parent;
    unsigned int first;
    unsigned int last;
    unsigned int prev;
    unsigned int next;
    unsigned int attribute;
    unsigned int attributes;
    unsigned int text;
    unsigned int length;

};

struct html_attribute
{

    unsigned int name;
    unsigned int value;

};

struct html_document
{

    struct html_node nodes[HTML_NODES];
    struct html_attribute attributes[HTML_ATTRIBUTES];
    char text[HTML_TEXTSIZE];
    unsigned int nodecount;
    unsigned int attributecount;
    unsigned int textcount;
    unsigned int html;
    unsigned int head;
    unsigned int body;
    unsigned int full;

};

char *html_getstring(struct html_document *document, unsigned int offset);
char *html_getattribute(struct html_document *document, unsigned int node, char *name);
unsigned int html_gettitle(struct html_document *document);
unsigned int html_setattribute(struct html_document *document, unsigned int node, char *name, char *value);
void html_removeattribute(struct html_document *document, unsigned int node, char *name);
unsigned int html_next(struct html_document *document, unsigned int node, unsigned int root);
void html_parse(struct html_document *document, char *source, unsigned int count);
void html_parsetext(struct html_document *document, char *source, unsigned int count);
