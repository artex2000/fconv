#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gtk/gtk.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include "fconv.h"

static FT_Vector last;

static FT_Face fc;
extern PangoFontMap *map;
extern PangoContext *ctx;
extern PangoFont *ft;

static ft2d (const FT_Vector *s, point_t *d)
{
	d->x = (double)s->x;
	d->y = (double)s->y;
}

static int line4 (const FT_Vector *a, const FT_Vector *b, 
	   const FT_Vector *c, const FT_Vector *d)
{
	if (a->x == b->x && b->x == c->x && c->x == d->x)
		return 1;
	if (a->y == b->y && b->y == c->y && c->y == d->y)
		return 1;
	return 0;
}
	
static int line3 (const FT_Vector *a, const FT_Vector *b, 
	   const FT_Vector *c)
{
	double r1, r2;
	double ax, ay, bx, by, cx, cy;
	r1 = 0;
	r2 = 1;

	if (a->x == b->x && b->x == c->x)
		return 1;
	if (a->y == b->y && b->y == c->y)
		return 1;
	if (a->x == b->x && a->y == b->y)
		return 1;
	if (b->x == c->x && b->y == c->y)
		return 1;
	if (a->x != b->x && b->x != c->x) {
		ax = (double)a->x;
		ay = (double)a->y;
		bx = (double)b->x;
		by = (double)b->y;
		cx = (double)c->x;
		cy = (double)c->y;
		r1 = (by - ay) / (bx - ax);
		r2 = (cy - by) / (cx - bx);
	}
	return (r1 == r2) ? 1 : 0;
}

static int move_to (const FT_Vector *to, void *user)
{
	gpart_t gp;

//	printf ("move to %+d:%+d\n", to->x, to->y);

	gp.type = move_to;
	ft2d (&last, &(gp.points[0]));
	ft2d (to, &(gp.points[1]));
	add_part ((glyph_t *)user, &gp);
	last = *to;
	return 0;
}

static int line_to (const FT_Vector *to, void *user)
{
	gpart_t gp;

//	printf ("line to %+d:%+d\n", to->x, to->y);

	gp.type = line_to;
	ft2d (&last, &(gp.points[0]));
	ft2d (to, &(gp.points[1]));
	add_part ((glyph_t *)user, &gp);
	last = *to;
	return 0;
}

static int conic_to (const FT_Vector *c1, const FT_Vector *to, void *user)
{
	gpart_t gp;

//	printf ("%d: conic to %+d:%+d %+d:%+d\n", piece, c1->x, c1->y, to->x, to->y);
	if (line3 (&last, c1, to)) 
		return line_to (to, user);

	gp.type = conic_to;
	ft2d (&last, &(gp.points[0]));
	ft2d (c1, &(gp.points[1]));
	ft2d (to, &(gp.points[2]));
	add_part ((glyph_t *)user, &gp);
	last = *to;
	return 0;
}

static int cubic_to (const FT_Vector *c1, const FT_Vector *c2, 
	      const FT_Vector *to, void *user)
{
	gpart_t gp;

//	printf ("%d: cubic to %+d:%+d %+d:%+d %+d:%+d\n", piece, c1->x, c1->y, 
//			c2->x, c2->y, to->x, to->y);
	if (line4 (&last, c1, c2, to))
		return line_to (to, user);

	gp.type = cubic_to;
	ft2d (&last, &(gp.points[0]));
	ft2d (c1, &(gp.points[1]));
	ft2d (c2, &(gp.points[2]));
	ft2d (to, &(gp.points[3]));
	add_part ((glyph_t *)user, &gp);
	last = *to;
	return 0;
}

void set_font (PangoFontDescription *desc)
{
	ft = pango_font_map_load_font (map, ctx, desc);
	fc = pango_ft2_font_get_face (ft);
	pango_font_description_free (desc);
}

void outline_glyph (char c, glyph_t *g)
{
	int err;
	unsigned int gl_idx;
	FT_Outline ol;
	FT_Outline_Funcs ifs;
	FT_Glyph_Metrics gm;

	ifs.shift = 0;
	ifs.delta = 0;
	ifs.move_to = move_to;
	ifs.line_to = line_to;
	ifs.conic_to = conic_to;
	ifs.cubic_to = cubic_to;

//	printf ("=============decomposing glyph %c\n", c);

	gl_idx = FT_Get_Char_Index (fc, (FT_ULong)c);
	if (!gl_idx) {
		printf ("No glyph for char %c\n", c);
		return;
	}

	err = FT_Load_Glyph (fc, gl_idx, FT_LOAD_NO_SCALE);
	if (err) {
		printf ("Error loading glyph for char %c\n", c);
		return;
	}

	last.x = last.y = 0;

	gm = fc->glyph->metrics;
	g->asc = (double)gm.horiBearingY;
	ol = fc->glyph->outline;
	err = FT_Outline_Decompose (&ol, &ifs, g);
	if (err) {
		printf ("Error decomposing glyph\n");
		return;
	}
//	printf ("================================\n");
}

