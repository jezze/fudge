#define LOG_CRITICAL                    0
#define LOG_ERROR                       1
#define LOG_WARNING                     2
#define LOG_INFO                        3

void log_notify(unsigned int level, void *buffer, unsigned int count);
