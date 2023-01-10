struct matrix3x3 matrix3x3_create(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2);
struct matrix3x3 matrix3x3_zero(void);
struct matrix3x3 matrix3x3_identity(void);
struct matrix3x3 matrix3x3_getrotationx(double s, double c);
struct matrix3x3 matrix3x3_getrotationy(double s, double c);
struct matrix3x3 matrix3x3_getrotationz(double s, double c);
