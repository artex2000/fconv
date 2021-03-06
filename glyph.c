#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fconv.h"

#define GLYPH_PART_INCREMENT 20

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
	int i, j, c;
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
	if (x == 0 && y == 0)
		return;
	offset_cm (x, y);
	tr_glyph (outline);
}

void scale_glyph (glyph_t *outline, double s)
{
	if (s == 1)
		return;
	scale_cm (s);
	tr_glyph (outline);
	outline->adv *= s;
	outline->asc *= s;
	outline->dsc *= s;
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
	d->dsc = g->dsc;
	d->adv = g->adv;
	d->max = g->cur;
	d->col = malloc (d->max * sizeof (gpart_t));
	if (d->col == NULL) {
		printf ("Not enough memory to duplicate glyph\n");
		exit (1);
	}
	memcpy (d->col, g->col, d->max * sizeof (gpart_t));
	return d;
}

void get_cbox (glyph_t *g, rect_t *c)
{
	gpart_t *gp;
	int i, j, l;
	double xmin, ymin, xmax, ymax;

	gp = g->col;
	if (gp->type != move_to) {
		printf ("glyph is not started with move_to\n");
		exit (1);
	}
	xmin = xmax = gp->points[1].x;
	ymin = ymax = gp->points[1].y;
	gp++;
	for (i = 1; i < g->cur; i++, gp++) {
		if (gp->type == move_to)
			continue;

		l = (gp->type == line_to) ? 2 :
		    (gp->type == conic_to) ? 3 : 4;
		for (j = 1; j < l; j++) {
			if (xmin > gp->points[j].x)
				xmin = gp->points[j].x;
			else if (xmax < gp->points[j].x)
				xmax = gp->points[j].x;
			if (ymin > gp->points[j].y)
				ymin = gp->points[j].y;
			else if (ymax < gp->points[j].y)
				ymax = gp->points[j].y;
		}
	}
	c->lt.x = xmin;
	c->lt.y = ymin;
	c->rb.x = xmax;
	c->rb.y = ymax;
}

static int get_angle (gpart_t *a, gpart_t *b)
{
	point_t l, m, r;
	double a_dot_b;
	double mod_a, mod_b;
	double ax, bx, ay, by;
	double angle, a_cos;
 
	m = b->points[0];
	r = b->points[1];
	l = (a->type == line_to) ? a->points[0] :
		(a->type == conic_to) ? a->points[1] : a->points[2];

	ax = l.x - m.x;
	ay = l.y - m.y;
	bx = r.x - m.x;
	by = r.y - m.y;

	a_dot_b = ax * bx + ay * by;
	mod_a = sqrt (ax * ax + ay * ay);
	mod_b = sqrt (bx * bx + by * by);
	a_cos = a_dot_b / (mod_a * mod_b);
	if (a_cos < -1 && a_cos > -1.01)
		a_cos  = -1;
	else if (a_cos > 1 && a_cos < 1.01)
		a_cos = 1;

	angle = acos (a_cos);
	return (angle < (M_PI / 2)) ? 1 : 0;
}

void set_ex_stop (glyph_t *g)
{
	gpart_t *gp, *next;
	int i;

	for (i = 0, gp = g->col; i < g->cur - 1; i++, gp++) {
		if (gp->type == move_to) {
			gp->ex_stop = 0;
			continue;
		}
		next = gp + 1;
		if (next->type == move_to) {
			gp->ex_stop = 0;
			continue;
		}
		if (gp->type == line_to && next->type == line_to) {
			gp->ex_stop = 1;
			continue;
		}
		gp->ex_stop = get_angle (gp, next);
	}
	gp->ex_stop = 0;
}
