struct vector3 vector3_create(double x, double y, double z);
struct vector3 vector3_zero(void);
struct vector3 vector3_rotatex(struct vector3 *v, double theta);
struct vector3 vector3_rotatey(struct vector3 *v, double theta);
struct vector3 vector3_rotatez(struct vector3 *v, double theta);
struct vector3 vector3_add(struct vector3 *v, double x, double y, double z);
struct vector3 vector3_add_vector3(struct vector3 *v1, struct vector3 *v2);
struct vector3 vector3_mul_matrix3x3(struct vector3 *v, struct matrix3x3 *m);
