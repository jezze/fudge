#define ATTR_COLOR                      1
#define ATTR_COLUMNS                    2
#define ATTR_CONTENT                    3
#define ATTR_CURSOR                     4
#define ATTR_FLOW                       5
#define ATTR_HALIGN                     6
#define ATTR_ID                         7
#define ATTR_IN                         8
#define ATTR_LABEL                      9
#define ATTR_MIMETYPE                   10
#define ATTR_MODE                       11
#define ATTR_ONCLICK                    12
#define ATTR_OVERFLOW                   13
#define ATTR_PADDING                    14
#define ATTR_SOURCE                     15
#define ATTR_SPAN                       16
#define ATTR_TITLE                      17
#define ATTR_VALIGN                     18
#define ATTR_WEIGHT                     19
#define ATTR_WRAP                       20
#define ATTR_FLOW_DEFAULT               0
#define ATTR_FLOW_HORIZONTAL            1
#define ATTR_FLOW_HORIZONTALSTRETCH     2
#define ATTR_FLOW_VERTICAL              3
#define ATTR_FLOW_VERTICALSTRETCH       4
#define ATTR_HALIGN_LEFT                0
#define ATTR_HALIGN_CENTER              1
#define ATTR_HALIGN_RIGHT               2
#define ATTR_MIMETYPE_NONE              0
#define ATTR_MIMETYPE_FUDGEMOUSE        1
#define ATTR_MIMETYPE_PCX               2
#define ATTR_MODE_NORMAL                0
#define ATTR_MODE_READONLY              1
#define ATTR_OVERFLOW_NONE              0
#define ATTR_OVERFLOW_SCROLL            1
#define ATTR_OVERFLOW_HSCROLL           2
#define ATTR_OVERFLOW_VSCROLL           3
#define ATTR_VALIGN_TOP                 0
#define ATTR_VALIGN_MIDDLE              1
#define ATTR_VALIGN_BOTTOM              2
#define ATTR_WEIGHT_NORMAL              0
#define ATTR_WEIGHT_BOLD                1
#define ATTR_WRAP_NONE                  0
#define ATTR_WRAP_CHAR                  1
#define ATTR_WRAP_WORD                  2

unsigned int attr_isvalue(unsigned int attribute);
unsigned int attr_isstring(unsigned int attribute);
unsigned int attr_isenum(unsigned int attribute);
unsigned int attr_update(unsigned int attribute, char *value, unsigned int current);
