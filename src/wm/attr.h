#define ATTR_BLIT                               1
#define ATTR_COLOR                              2
#define ATTR_COLUMNS                            3
#define ATTR_CONTENT                            4
#define ATTR_HALIGN                             5
#define ATTR_FIT                                6
#define ATTR_FORM                               7
#define ATTR_ID                                 8
#define ATTR_IN                                 9
#define ATTR_LABEL                              10
#define ATTR_MIMETYPE                           11
#define ATTR_MODE                               12
#define ATTR_PADDING                            13
#define ATTR_SOURCE                             14
#define ATTR_TITLE                              15
#define ATTR_VALIGN                             16
#define ATTR_WEIGHT                             17
#define ATTR_WRAP                               18
#define ATTR_FIT_NORMAL                         0
#define ATTR_FIT_STRETCHED                      1
#define ATTR_FORM_FLOAT                         0
#define ATTR_FORM_HORIZONTAL                    1
#define ATTR_FORM_MAXIMIZE                      2
#define ATTR_FORM_VERTICAL                      3
#define ATTR_MIMETYPE_FUDGEMOUSE                1
#define ATTR_MIMETYPE_PCX                       2
#define ATTR_MODE_NORMAL                        0
#define ATTR_MODE_READONLY                      1
#define ATTR_BLIT_NORMAL                        0
#define ATTR_BLIT_INVERTED                      1
#define ATTR_HALIGN_LEFT                        0
#define ATTR_HALIGN_CENTER                      1
#define ATTR_HALIGN_RIGHT                       2
#define ATTR_VALIGN_TOP                         0
#define ATTR_VALIGN_MIDDLE                      1
#define ATTR_VALIGN_BOTTOM                      2
#define ATTR_WEIGHT_NORMAL                      0
#define ATTR_WEIGHT_BOLD                        1
#define ATTR_WRAP_NONE                          0
#define ATTR_WRAP_CHAR                          1
#define ATTR_WRAP_WORD                          2

unsigned int attr_isvalue(unsigned int attribute);
unsigned int attr_isstring(unsigned int attribute);
unsigned int attr_isenum(unsigned int attribute);
unsigned int attr_update(unsigned int attribute, char *value, unsigned int current);
