#define DATE_TEXTSIZE                   64
#define DATE_ISO                        0
#define DATE_STRING                     1
#define DATE_DATESTRING                 2
#define DATE_TIMESTRING                 3
#define DATE_UTCSTRING                  4
#define DATE_LOCALE                     5
#define DATE_LOCALEDATE                 6
#define DATE_LOCALETIME                 7

struct date_parts
{

    int year;
    int month;
    int day;
    int weekday;
    int hours;
    int minutes;
    int seconds;
    int milliseconds;

};

double date_make(double year, double month, double day, double hours, double minutes, double seconds, double milliseconds);
struct date_parts date_split(double time);
double date_parse(char *text);
unsigned int date_format(double time, unsigned int kind, char *buffer);
