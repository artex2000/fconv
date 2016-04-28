#include <stdio.h>
#include <cairo.h>

#include "fconv.h"

static cairo_t *cr;

static void cut2image (cut_t *ct)
{
	if (ct->type == move)
		cairo_move_to (cr, ct->end.x, ct->end.y);
	else
		cairo_line_to (cr, ct->end.x, ct->end.y);
}

static void curve2image (curve_t *cv)
{
	if (cv->type == cw_circle)
		cairo_arc (cr, cv->center.x, cv->center.y,
				cv->radius,
				cv->angle1, cv->angle2);
	else
		cairo_arc_negative (cr, cv->center.x, cv->center.y,
				cv->radius,
				cv->angle1, cv->angle2);
}

static void process_glyph (glyph_t *g)
{
	cut_t ct;
	curve_t cv[4];
	gpart_t *gp;
	int i, j;

	for (i = 0, gp = g->col; i < g->cur; i++, gp++) {
		if (gp->type == move_to || gp->type == line_to) {
			gp2cut (gp, &ct);
			cut2image (&ct);
		} else {
			gp2curve (gp, cv);
			for (j = 0; j < 4; j++)
				curve2image (&cv[j]);
		}
	}
}

static void glyph2image (glyph_t **p, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++, p++)
		process_glyph (*p);
}

void generate_image (char *s, cairo_t *ctx, double w, double h)
{
	glyph_t **p;
	int c;

	cr = ctx;
	c = generate_glyph (s);
	if (c == 0)
		return;
	p = get_scaled_image (w, h);
	glyph2image (p, c);
	free_image (p);
}
