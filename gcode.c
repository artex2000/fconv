#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "fconv.h"
#include "conf.h"

#define MACHINE_ZERO_X 89.219
#define MACHINE_ZERO_Y 136.831

#define START_POINT_X 84.511
#define START_POINT_Y 123.698

double dpi = 300;	//dots per inch

//machine zero coordinates - should be added
static double z_m_x = MACHINE_ZERO_X;
static double z_m_y = MACHINE_ZERO_Y;

//tool zero coordinates
static double z_t_x = START_POINT_X - MACHINE_ZERO_X;
static double z_t_y = START_POINT_Y - MACHINE_ZERO_Y;

//vertical router movement
static double z_drill = -4.061;
static double z_hang = -2.5;
static double z_current = 0;

//feed rate
static int feed_cut = 100;
static int feed_current = 0;

//control of movement G61, G64
static int mstop;

static FILE *fp;

static double d2i (double d)
{
	double i;
	i = d / dpi;
	return i;
}

static double get_m_y (double d)
{
	double m_y;
	m_y = d2i (d);	//dots to inches
	m_y = -m_y;	//change sign
	m_y += z_m_y + z_t_y;
	return m_y;
}

static double get_m_x (double d)
{
	double m_x;
	m_x = d2i (d);	//dots to inches
	m_x = -m_x;	//change sign
	m_x += z_m_x + z_t_x;
	return m_x;
}

static void router_up (void)
{
	if (z_current < z_hang) {	//router up
		fprintf (fp, "G00 Z%.3f\n", z_hang);
		z_current = z_hang;
	}
}

static void router_down (void)
{
	if (z_current > z_drill) {	//router down
		fprintf (fp, "G01 Z%.3f", z_drill);
		z_current = z_drill;
		if (feed_current < feed_cut) {
			fprintf (fp, " F%d\n", feed_cut);
			feed_current = feed_cut;
		} else {
			fprintf (fp, "\n");
		}
	}
}

static void cut2gcode (cut_t *c) 
{
	double x, y;
	x = get_m_x (c->end.y);
	y = get_m_y (c->end.x);
	if (c->type == move) {
		router_up ();
		fprintf (fp, "G00 X%.3f Y%.3f\n", x, y);
	} else {
		router_down ();
		fprintf (fp, "G01 X%.3f Y%.3f\n", x, y);
	}
}

static void curve2gcode (curve_t *c) 
{
	double x, y, r;
	x = get_m_x (c->end.y);
	y = get_m_y (c->end.x);
	r = d2i (c->radius);
	router_down ();
	if (c->type == cw_circle)
		fprintf (fp, "G02 X%.3f Y%.3f R%.3f\n", x, y, r);
	else
		fprintf (fp, "G03 X%.3f Y%.3f R%.3f\n", x, y, r);
}

static void modal_stop (void)
{
	if (mstop == 0) {
		fprintf (fp, "G61 ");
		mstop = 1;
	}
}

static void modal_smooth (void)
{
	if (mstop == 1) {
		fprintf (fp, "G64 ");
		mstop = 0;
	}
}

static void process_glyph (glyph_t *g)
{
	cut_t ct;
	curve_t cv[4];
	gpart_t *gp, *next;
	int i, j;

	mstop = 0;
	for (i = 0, gp = g->col; i < g->cur; i++, gp++) {
		if (gp->type == move_to || gp->type == line_to) {
			(gp->ex_stop == 0) ? modal_smooth () : modal_stop ();
			gp2cut (gp, &ct);
			cut2gcode (&ct);
		} else {
			gp2curve (gp, cv);
			modal_smooth ();
			for (j = 0; j < 3; j++) //first 3 appx are smooth
				curve2gcode (&cv[j]);
			(gp->ex_stop == 0) ? modal_smooth () : modal_stop ();
			curve2gcode (&cv[j]);
		}
	}
}

static void prologue (void)
{
	fprintf (fp, "M06 M16\n");
	fprintf (fp, "M11 M13\n");
	fprintf (fp, "M93 S18000\n");
	fprintf (fp, "G17 G90 G53\n");
	fprintf (fp, "G04 X4.0\n");
	fprintf (fp, "G40 ");
}

static void epilogue (void)
{
	fprintf (fp, "G00 Z0\n");	//router up
	fprintf (fp, "G91 G28 X0 Y0 Z0\n");
	fprintf (fp, "M92 M95\n");
	fprintf (fp, "M07 M17\n");
	fprintf (fp, "M30\n");
}

static void glyph2gcode (glyph_t **p, int cnt, char *fname)
{
	int i;

	fp = fopen (fname, "w");
	if (fp == NULL) {
		perror ("Error writing to file\n");
		exit (1);
	}
	prologue ();
	for (i = 0; i < cnt; i++, p++)
		process_glyph (*p);
	epilogue ();
	fclose (fp);
}

static void init_gcode_conf (void)
{

    feed_cut = get_feedrate ();
    z_hang = get_hang ();
    z_drill = get_drill ();
    z_t_x = get_xorigin ();
    z_t_y = get_yorigin ();
}

void generate_gcode (char *s, double w_inch)
{
	glyph_t **p;
	int c;
        double xs, ys;
	char fname[26];

	c = generate_glyph (s);
	if (c == 0)
		return;
	snprintf (fname, 26, "%s_gcode.txt", s);

        init_gcode_conf ();
        xs = get_xscale ();
        ys = get_yscale ();

        if (!xs && !ys)
            p = get_scaled_image (w_inch * dpi, 0, 0);
        else
            p = get_scaled_image (xs * dpi, ys * dpi, 0);

	glyph2gcode (p, c, fname);
	free_image (p);
}
