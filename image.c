#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fconv.h"

static char current[20];
static glyph_t **image;
static int symbols = 0;

static void fill_glyph (char *s, int cnt)
{
	int i;

	if (symbols != 0)
		free_image (image);

	symbols = cnt;

	if (strcmp (current, s))
		strcpy (current, s);

	image = malloc (symbols * sizeof (glyph_t *));
	if (image == NULL) {
		printf ("Not enough memory to process glyph\n");
		exit (1);
	}

	for (i = 0; i < symbols; i++) {
		image[i] = new_glyph ();
		outline_glyph (current[i], image[i]);
	}
}

static void normalize_glyph (void)
{
	int i;
	rect_t c;
	double h;

	for (i = 0; i < symbols; i++) {
		flip_hor_glyph (image[i]);
		get_cbox (image[i], &c);
		offset_glyph (image[i], -c.lt.x, -c.lt.y);
		image[i]->adv = c.rb.x - c.lt.x;
		h = c.rb.y - c.lt.y;
		image[i]->dsc = h - image[i]->asc;
	}
}

static void get_unscaled_size (double *w, double *h)
{
	double asc, dsc, adv;
	int i;

	asc = dsc = adv = 0;
	for (i = 0; i < symbols; i++) {
		adv += image[i]->adv;
		asc = (image[i]->asc > asc) ? image[i]->asc : asc;
		dsc = (image[i]->dsc > dsc) ? image[i]->dsc : dsc;
	}
	*w = adv;
	*h = asc + dsc;
}

static void fit_image (glyph_t **tmp, double w, double h, double border)
{
	double iw, ih;
	double sx, sy, s;
	int i;
	double asc, adv;

	get_unscaled_size (&iw, &ih);

	w -= 2 * border;
	h -= 2 * border;

	sx = (w == 0) ? 1 : w / iw;
	sy = (h == 0) ? 1 : h / ih;
	s = (w == 0) ? sy : (h == 0) ? sx : (sx < sy) ? sx : sy;

	asc = 0;
	adv = border;
	for (i = 0; i < symbols; i++) {
		tmp[i] = duplicate_glyph (image[i]);
		scale_glyph (tmp[i], s);
		asc = (tmp[i]->asc > asc) ? tmp[i]->asc : asc;
	}
	for (i = 0; i < symbols; i++) {
		offset_glyph (tmp[i], adv, asc - tmp[i]->asc + border);
		adv += tmp[i]->adv;
	}
}

int generate_glyph (char *s)
{
	int i;

	i = strlen (s);
	if (i > 19) {
		printf ("String is too long \n");
		return 0;
	}
	if (strcmp (s, current)) {
		fill_glyph (s, i);
		normalize_glyph ();
	}
	return i;
}

glyph_t ** get_scaled_image (double w, double h, double border)
{
	glyph_t **tmp;

	tmp = malloc (symbols * sizeof (glyph_t *));
	if (tmp == NULL) {
		printf ("Not enough memory to process glyph\n");
		exit (1);
	}

	fit_image (tmp, w, h, border);
	return tmp;
}

void free_image (glyph_t **g)
{
	int i;
	void *p = g;
	for (i = 0; i < symbols; i++, g++)
		free_glyph (*g);
	free (p);
}

