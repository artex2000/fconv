#include <string.h>
#include <math.h>
#include "fconv.h"

static double cm[3][3];

static void clear_cm (void)
{
	memset (cm, 0, 9 * sizeof (double));
	cm[2][2] = 1;
}

static void offset_cm (double x, double y)
{
	clear_cm ();
	cm[0][0] = 1;
	cm[0][2] = x;
	cm[1][1] = 1;
	cm[1][2] = y;
}

static void scale_cm (double s)
{
	clear_cm ();
	cm[0][0] = s;
	cm[1][1] = s;
}

static void rotate_cm (double a)
{
	clear_cm ();
	cm[0][0] = cos (a);
	cm[0][1] = sin (a);
	cm[1][0] = -sin (a);
	cm[1][1] = cos (a);
}

static void flip_h_cm (void)
{
	clear_cm ();
	cm[0][0] = 1;
	cm[1][1] = -1;
}

static void flip_v_cm (void)
{
	clear_cm ();
	cm[0][0] = -1;
	cm[1][1] = 1;
}

static void tr_point (point_t *o)
{
	double x, y;
	x = o->x * cm[0][0] + o->y * cm[0][1] + cm[0][2];
	y = o->x * cm[1][0] + o->y * cm[1][1] + cm[1][2];
	o->x = x;
	o->y = y;
}

static void tr_glyph (glyph_t *g)
{
	int i, j;
	gpart_t *gp = g->col;
	
	for (j = 0, gp = g->col; j < g->cur; j++, gp++) {
		switch (gp->type) {
			case move_to:
				c = 2;
				break;
			case line_to:
				c = 2;
				break;
			case conic_to:
				c = 3;
				break;
			case cubic_to:
				c = 4;
				break;
			default:
				break;
		}
		for (i = 0; i < c; i++) 
			tr_point (&gp->points[i]);
	}
}

void offset_glyph (glyph_t *outline, double x, double y)
{
	offset_cm (x, y);
	tr_glyph (outline);
}

void scale_glyph (glyph_t *outline, double s)
{
	scale_cm (s);
	tr_glyph (outline);
	outline->asc *= s;
}

void rotate_glyph (glyph_t *outline, double a)
{
	rotate_cm (a);
	tr_glyph (outline);
}

void flip_hor_glyph (glyph_t *outline)
{
	flip_h_cm ();
	tr_glyph (outline);
}

void flip_ver_glyph (glyph_t *outline)
{
	flip_v_cm ();
	tr_glyph (outline);
}

void add_part (glyph_t *g, gpart_t *gp)
{
	if (g->cur == g->max) {
		g->max += GLYPH_PART_INCREMENT;
		g->col = realloc (g->col, g->max * sizeof (gpart_t));
		if (g->col == NULL) {
			printf ("Not enough memory to add glyph\n");
			exit (1);
		}
	}
	memcpy(&g->col[g->cur++], gp, sizeof (gpart_t));
}

glyph_t *new_glyph (void)
{
	glyph_t *g;

	g = malloc (sizeof (*g));
	if (g == NULL) {
		printf ("Not enough memory to add glyph\n");
		exit (1);
	}
	g->cur = 0;
	g->asc = 0;
	g->max = GLYPH_PART_INCREMENT;
	g->col = malloc (g->max * sizeof (gpart_t));
	if (g->col == NULL) {
		printf ("Not enough memory to add glyph\n");
		exit (1);
	}
	return g;
}

void free_glyph (glyph_t *g)
{
	free (g->col);
	free (g);
}

glyph_t *duplicate_glyph (glyph_t *g)
{
	glyph_t *d;

	d = malloc (sizeof (*d));
	if (d == NULL) {
		printf ("Not enough memory to duplicate glyph\n");
		exit (1);
	}
	d->cur = g->cur;
	d->asc = g->asc;
	d->max = g->cur;
	d->col = malloc (d->max * sizeof (gpart_t));
	if (d->col == NULL) {
		printf ("Not enough memory to duplicate glyph\n");
		exit (1);
	}
	memcpy (d->col, g->col, d->max * sizeof (gpart_t));
	return d;
}
