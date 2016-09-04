#define DEBUG(t, s)                     debug_write(t, s, __FILE__, __LINE__)
#define DEBUG_CRITICAL                  0
#define DEBUG_ERROR                     1
#define DEBUG_WARNING                   2
#define DEBUG_INFO                      3

struct debug_log
{

    struct resource resource;
    void (*write)(unsigned int level, char *string, char *file, unsigned int line);

};

void debug_write(unsigned int level, char *string, char *file, unsigned int line);
void debug_initlog(struct debug_log *log, void (*write)(unsigned int level, char *string, char *file, unsigned int line));
