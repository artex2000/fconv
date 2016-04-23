#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fconv.h"

static point_t * bezier2 (point_t *pt)
{
	double t;
	int i;
	point_t a, b, c, *p;

	p = malloc (sizeof (*p) * MAX_POINTS);
	if (p == NULL) {
		printf ("Not enough memory to create bezier curve\n");
		exit (1);
	}

	a = pt[0];
	b = pt[1];
	c = pt[2];

	for (i = 0; i < MAX_POINTS; i++) {
		t = ((double)i)/(double)(MAX_POINTS - 1);
		p[i].x = (1 - t) * (1 - t) * a->x +
			2 * (1 - t) * t * b->x +
			t * t * c->x;
		p[i].y = (1 - t) * (1 - t) * a->y +
			2 * (1 - t) * t * b->y +
			t * t * c->y;
	}
	return p;
}

static point_t * bezier3 (point_t *pt)
{
	double t;
	int i;
	point_t *p;

	p = malloc (sizeof (*p) * MAX_POINTS);
	if (p == NULL) {
		printf ("Not enough memory to create bezier curve\n");
		exit (1);
	}

	a = pt[0];
	b = pt[1];
	c = pt[2];
	d = pt[3];

	for (i = 0; i < MAX_POINTS; i++) {
		t = ((double)i)/(double)(MAX_POINTS - 1);
		p[i].x = (1 - t) * (1 - t) * (1 - t) * a->x +
			3 * (1 - t) * (1 - t) * t * b->x +
			3 * (1 - t) * t * t * c->x + 
			t * t * t * d->x;
		p[i].y = (1 - t) * (1 - t) * (1 - t) * a->y +
			3 * (1 - t) * (1 - t) * t * b->y +
			3 * (1 - t) * t * t * c->y + 
			t * t * t * d->y;
	}
	return p;
}


static void get_curve (int start, int size, point_t *p, curve_t *c)
{
	point_t x0, x1, x2;
	double a, b, c, d,
	       e, f, g, h,
	       i, j, k, l;
	double m11, m12, m13, m14;
	double r, xc, yc, a1, a2;

	x0.x = p[start].x;
	x0.y = p[start].y;
	x2.x = p[start + size - 1].x;
	x2.y = p[start + size - 1].y;
	x1.x = p[start + size / 2 - 1].x;
	x1.y = p[start + size / 2 - 1].y;

	c->start = x0;
	c->end = x2;

	a = x0.x * x0.x + x0.y * x0.y;
	b = x0.x;
	c = x0.y;
	d = 1;

	e = x1.x * x1.x + x1.y * x1.y;
	f = x1.x;
	g = x1.y;
	h = 1;

	i = x2.x * x2.x + x2.y * x2.y;
	j = x2.x;
	k = x2.y;
	l = 1;

	m11 = b * g * l + f * k * d + c * h * j -
		j * g * d - k * h * b - f * c * l;
	m12 = a * g * l + e * k * d + c * h * i -
		i * g * d - e * c * l - k * h * a;
	m13 = a * f * l + b * h * i + e * j * d -
		i * f * d - e * b * l - j * h * a;
	m14 = a * f * k + b * g * i + e * j * c -
		i * f * c - e * b * k - j * g * a;

	xc = 0.5 * m12 / m11;
	yc = -0.5 * m13 / m11;
	r = sqrt (xc * xc + yc * yc + m14 / m11);
	
	a1 = atan2 (x0.y - yc, x0.x - xc);
	if (a1 < 0)
		a1 += 2 * M_PI;
	a2 = atan2 (x2.y - yc, x2.x - xc);
	if (a2 < 0)
		a2 += 2 * M_PI;
	
	if (a2 < a1) {
		if ((a1 - a2) >= M_PI) {
			a1 -= 2 * M_PI;
			c->type = cw_circle;
		} else {
			c->type = ccw_circle;
		}
	} else {
		if ((a2 - a1) >= M_PI) {
			a2 -= 2 * M_PI;
			c->type = ccw_circle;
		} else {
			c->type = cw_circle;
		}
	}

	c->center.x = xc;
	c->center.y = yc;
	c->radius = r;
	c->angle1 = a1;
	c->angle2 = a2;
}

void gp2cut (gpart_t *gp, cut_t *c)
{
	c->start = gp->points[0];
	c->end = gp->points[1];
	c->type = (gp->type == move_to) ? move : line;
}

void gp2curve (gpart_t *gp, curve_t *c)
{
	point *bz;

	bz = (gp->type == conic_to) ? 
		bezier2 (gp->points) : bezier3 (gp->points);

	get_curve (0, 65, bz, c++);
	get_curve (64, 65, bz, c++);
	get_curve (128, 65, bz, c++);
	get_curve (192, 65, bz, c);
}

