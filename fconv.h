#ifndef __FONT_CONV_H__
#define __FONT_CONV_H__

typedef struct {
	double x;
	double y;
} point_t;

typedef struct {
	point_t lt;
	point_t rb;
} rect_t;

typedef enum {
	move_to,
	line_to,
	conic_to,
	cubic_to
} outline_t;

typedef enum {
	move,
	line,
	cw_circle,
	ccw_circle
} stroke_t;

typedef struct {
	point_t points[4];
	outline_t type;
	int ex_stop;
} gpart_t;

typedef struct {
	gpart_t *col;
	double asc;
	double dsc;
	double adv;
	int cur;
	int max;
} glyph_t;

typedef struct {
	point_t start;
	point_t end;
	stroke_t type;
} cut_t;

typedef struct {
	point_t start;
	point_t end;
	stroke_t type;
	point_t center;
	double radius;
	double angle1;
	double angle2;
} curve_t;



void gp2cut (gpart_t *g, cut_t *c);
void gp2curve (gpart_t *g, curve_t *c);

glyph_t *new_glyph (void);
void free_glyph (glyph_t *g);
glyph_t *duplicate_glyph (glyph_t *g);
void add_part (glyph_t *g, gpart_t *gp);
void offset_glyph (glyph_t *g, double x, double y);
void scale_glyph (glyph_t *g, double sf);
void rotate_glyph (glyph_t *g, double angle);
void flip_hor_glyph (glyph_t *g);
void flip_ver_glyph (glyph_t *g);
void get_cbox (glyph_t *g, rect_t *c);
void set_ex_stop (glyph_t *g);

int generate_glyph (char *s);
void generate_gcode (char *s, double w_inch);
glyph_t ** get_scaled_image (double w, double h, double border);
void free_image (glyph_t **g);


#endif
