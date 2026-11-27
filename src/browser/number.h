#define NUMBER_PI                       3.14159265358979323846
#define NUMBER_E                        2.71828182845904523536
#define NUMBER_TEXTSIZE                 32

struct number_parsed
{

    double value;
    unsigned int used;

};

double number_nan(void);
double number_infinity(void);
unsigned int number_isnan(double x);
unsigned int number_isfinite(double x);
struct number_parsed number_parse(char *text, unsigned int count);
unsigned int number_format(double value, char *buffer);
unsigned int number_fixed(double value, unsigned int digits, char *buffer);
unsigned int number_radix(double value, unsigned int radix, char *buffer);
double number_floor(double x);
double number_ceil(double x);
double number_trunc(double x);
double number_round(double x);
double number_mod(double a, double b);
double number_sqrt(double x);
double number_exp(double x);
double number_log(double x);
double number_pow(double a, double b);
double number_sin(double x);
double number_cos(double x);
double number_tan(double x);
double number_atan(double x);
double number_atan2(double y, double x);
double number_asin(double x);
double number_acos(double x);
int number_toint32(double x);