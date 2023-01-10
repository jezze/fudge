struct vector2 vector2_create(double x, double y);
struct vector2 vector2_zero(void);
struct vector2 vector2_add(struct vector2 *v, double x, double y);
struct vector2 vector2_add_vector2(struct vector2 *v1, struct vector2 *v2);
struct vector2 vector2_mul(struct vector2 *v, double x, double y);
struct vector2 vector2_mul_vector2(struct vector2 *v1, struct vector2 *v2);
