#include <fudge.h>
#include "number.h"
#include "date.h"

#define DAY                             86400000.0
#define LIMIT                           8.64e15

static char *weekdays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static int floordiv(int a, int b)
{

    return (a >= 0) ? a / b : -((-a + b - 1) / b);

}

static int daysfromcivil(int year, int month, int day)
{

    int era;
    int yoe;
    int doy;
    int doe;

    /* days since 1970-01-01 in the proleptic gregorian calendar, with march as the first month */
    year -= month <= 2;
    era = floordiv(year, 400);
    yoe = year - era * 400;
    doy = (153 * ((month > 2) ? month - 3 : month + 9) + 2) / 5 + day - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

    return era * 146097 + doe - 719468;

}

double date_make(double year, double month, double day, double hours, double minutes, double seconds, double milliseconds)
{

    double whole;
    int y;
    int m;
    double time;

    if (!number_isfinite(year) || !number_isfinite(month) || !number_isfinite(day) || !number_isfinite(hours) || !number_isfinite(minutes) || !number_isfinite(seconds) || !number_isfinite(milliseconds))
        return number_nan();

    /* a month past either end of the year carries into the year */
    whole = number_floor(number_trunc(month) / 12.0);
    y = (int)(number_trunc(year) + whole);
    m = (int)(number_trunc(month) - whole * 12.0);
    time = (daysfromcivil(y, m + 1, 1) + number_trunc(day) - 1) * DAY + number_trunc(hours) * 3600000.0 + number_trunc(minutes) * 60000.0 + number_trunc(seconds) * 1000.0 + number_trunc(milliseconds);

    return (time > LIMIT || time < -LIMIT) ? number_nan() : time;

}

struct date_parts date_split(double time)
{

    struct date_parts parts;
    int days = (int)number_floor(time / DAY);
    int rest = (int)(time - days * DAY);
    int z = days + 719468;
    int era = floordiv(z, 146097);
    int doe = z - era * 146097;
    int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    int mp = (5 * doy + 2) / 153;

    parts.day = doy - (153 * mp + 2) / 5 + 1;
    parts.month = (mp < 10) ? mp + 2 : mp - 10;
    parts.year = yoe + era * 400 + (parts.month <= 1);
    parts.weekday = ((days % 7) + 11) % 7;
    parts.hours = rest / 3600000;
    parts.minutes = rest / 60000 % 60;
    parts.seconds = rest / 1000 % 60;
    parts.milliseconds = rest % 1000;

    return parts;

}

struct reader
{

    char *text;
    unsigned int offset;

};

static int readnumber(struct reader *reader, unsigned int digits)
{

    unsigned int start = reader->offset;
    int value = 0;

    while (reader->offset - start < digits && reader->text[reader->offset] >= '0' && reader->text[reader->offset] <= '9')
        value = value * 10 + (reader->text[reader->offset++] - '0');

    return (reader->offset > start) ? value : -1;

}

static unsigned int accept(struct reader *reader, char c)
{

    if (reader->text[reader->offset] != c)
        return 0;

    reader->offset++;

    return 1;

}

double date_parse(char *text)
{

    struct reader reader;
    int year;
    int month = 1;
    int day = 1;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int milliseconds = 0;
    int zone = 0;
    int sign = 1;

    reader.text = text;
    reader.offset = 0;

    while (accept(&reader, ' '));

    /* the iso form, YYYY-MM-DDTHH:MM:SS.sssZ, with every part after the year optional and / allowed between the date's parts */
    if (accept(&reader, '-'))
        sign = -1;
    else
        accept(&reader, '+');

    year = readnumber(&reader, 6);

    if (year < 0)
        return number_nan();

    if (accept(&reader, '-') || accept(&reader, '/'))
        month = readnumber(&reader, 2);

    if (accept(&reader, '-') || accept(&reader, '/'))
        day = readnumber(&reader, 2);

    if (accept(&reader, 'T') || accept(&reader, ' '))
    {

        hours = readnumber(&reader, 2);
        minutes = (accept(&reader, ':')) ? readnumber(&reader, 2) : -1;

        if (accept(&reader, ':'))
            seconds = readnumber(&reader, 2);

        if (accept(&reader, '.'))
        {

            unsigned int start = reader.offset;

            milliseconds = readnumber(&reader, 3);

            if (reader.offset - start == 1)
                milliseconds *= 100;
            else if (reader.offset - start == 2)
                milliseconds *= 10;

            while (readnumber(&reader, 1) >= 0);

        }

    }

    if (accept(&reader, '+') || accept(&reader, '-'))
    {

        int direction = (text[reader.offset - 1] == '-') ? -1 : 1;
        int zonehours = readnumber(&reader, 2);
        int zoneminutes;

        accept(&reader, ':');

        zoneminutes = readnumber(&reader, 2);
        zone = direction * (zonehours * 60 + ((zoneminutes < 0) ? 0 : zoneminutes));

    }

    else
    {

        accept(&reader, 'Z');

    }

    if (text[reader.offset] && text[reader.offset] != ' ')
        return number_nan();

    if (month < 1 || month > 12 || day < 1 || day > 31 || hours < 0 || hours > 24 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59 || milliseconds < 0 || zone > 24 * 60 || zone < -24 * 60)
        return number_nan();

    return date_make(sign * year, month - 1, day, hours, minutes, seconds, milliseconds) - zone * 60000.0;

}

static unsigned int putnumber(char *buffer, unsigned int offset, int value, unsigned int width)
{

    char digits[12];
    unsigned int count = 0;
    unsigned int negative = value < 0;

    if (negative)
        value = -value;

    do
    {

        digits[count++] = '0' + value % 10;
        value /= 10;

    } while (value);

    if (negative)
        buffer[offset++] = '-';

    while (count < width)
        digits[count++] = '0';

    while (count)
        buffer[offset++] = digits[--count];

    return offset;

}

static unsigned int puttext(char *buffer, unsigned int offset, char *text)
{

    unsigned int i;

    for (i = 0; text[i]; i++)
        buffer[offset++] = text[i];

    return offset;

}

unsigned int date_format(double time, unsigned int kind, char *buffer)
{

    struct date_parts parts;
    unsigned int offset = 0;

    if (number_isnan(time))
    {

        offset = puttext(buffer, offset, "Invalid Date");
        buffer[offset] = '\0';

        return offset;

    }

    parts = date_split(time);

    switch (kind)
    {

    case DATE_ISO:
        offset = putnumber(buffer, offset, parts.year, 4);
        buffer[offset++] = '-';
        offset = putnumber(buffer, offset, parts.month + 1, 2);
        buffer[offset++] = '-';
        offset = putnumber(buffer, offset, parts.day, 2);
        buffer[offset++] = 'T';
        offset = putnumber(buffer, offset, parts.hours, 2);
        buffer[offset++] = ':';
        offset = putnumber(buffer, offset, parts.minutes, 2);
        buffer[offset++] = ':';
        offset = putnumber(buffer, offset, parts.seconds, 2);
        buffer[offset++] = '.';
        offset = putnumber(buffer, offset, parts.milliseconds, 3);
        buffer[offset++] = 'Z';

        break;

    case DATE_UTCSTRING:
        offset = puttext(buffer, offset, weekdays[parts.weekday]);
        offset = puttext(buffer, offset, ", ");
        offset = putnumber(buffer, offset, parts.day, 2);
        buffer[offset++] = ' ';
        offset = puttext(buffer, offset, months[parts.month]);
        buffer[offset++] = ' ';
        offset = putnumber(buffer, offset, parts.year, 4);
        buffer[offset++] = ' ';
        offset = putnumber(buffer, offset, parts.hours, 2);
        buffer[offset++] = ':';
        offset = putnumber(buffer, offset, parts.minutes, 2);
        buffer[offset++] = ':';
        offset = putnumber(buffer, offset, parts.seconds, 2);
        offset = puttext(buffer, offset, " GMT");

        break;

    case DATE_LOCALE:
    case DATE_LOCALEDATE:
    case DATE_LOCALETIME:
        /* the way en-US writes it, the time on a twelve hour clock */
        if (kind != DATE_LOCALETIME)
        {

            offset = putnumber(buffer, offset, parts.month + 1, 1);
            buffer[offset++] = '/';
            offset = putnumber(buffer, offset, parts.day, 1);
            buffer[offset++] = '/';
            offset = putnumber(buffer, offset, parts.year, 4);

        }

        if (kind == DATE_LOCALE)
            offset = puttext(buffer, offset, ", ");

        if (kind != DATE_LOCALEDATE)
        {

            offset = putnumber(buffer, offset, (parts.hours % 12) ? parts.hours % 12 : 12, 1);
            buffer[offset++] = ':';
            offset = putnumber(buffer, offset, parts.minutes, 2);
            buffer[offset++] = ':';
            offset = putnumber(buffer, offset, parts.seconds, 2);
            offset = puttext(buffer, offset, (parts.hours < 12) ? " AM" : " PM");

        }

        break;

    default:
        if (kind != DATE_TIMESTRING)
        {

            offset = puttext(buffer, offset, weekdays[parts.weekday]);
            buffer[offset++] = ' ';
            offset = puttext(buffer, offset, months[parts.month]);
            buffer[offset++] = ' ';
            offset = putnumber(buffer, offset, parts.day, 2);
            buffer[offset++] = ' ';
            offset = putnumber(buffer, offset, parts.year, 4);

        }

        if (kind == DATE_STRING)
            buffer[offset++] = ' ';

        if (kind != DATE_DATESTRING)
        {

            offset = putnumber(buffer, offset, parts.hours, 2);
            buffer[offset++] = ':';
            offset = putnumber(buffer, offset, parts.minutes, 2);
            buffer[offset++] = ':';
            offset = putnumber(buffer, offset, parts.seconds, 2);
            offset = puttext(buffer, offset, " GMT+0000 (Coordinated Universal Time)");

        }

        break;

    }

    buffer[offset] = '\0';

    return offset;

}

