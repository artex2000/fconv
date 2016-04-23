#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "fconv.h"

PangoFontMap *map;
PangoContext *ctx;
PangoFont *ft;

void set_font (PangoFontDescription *desc){}

/*
void draw_outline (cairo_t *cr, list_item *o)
{
		ct = (Cut *)n->data;
		cv = (Curve *)n->data;

		switch (ct->action) {
			case Move:
				cairo_move_to (cr, ct->end.x, ct->end.y);
				break;
			case Line:
				cairo_line_to (cr, ct->end.x, ct->end.y);
				break;
			case Cw:
				cairo_arc (cr, cv->center.x, cv->center.y,
						cv->radius,
						cv->angle1, cv->angle2);
				break;
			case Ccw:
				cairo_arc_negative (cr, cv->center.x, cv->center.y,
						cv->radius,
						cv->angle1, cv->angle2);
				break;
}
*/

static gboolean
expose_event (GtkWidget *widget, GdkEventExpose *event)
{
	cairo_t *cr;
	double cx, cy, cw, ch;
	int i;

	cr = gdk_cairo_create (widget->window);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_set_line_width (cr, 0.5);
	cx = cy = 1.0;
	cw = (double)widget->allocation.width - 2;
	ch = (double)widget->allocation.height - 2;
	
	cairo_rectangle (cr, cx, cy, cw, ch);
//	cairo_new_sub_path (cr);
	cairo_stroke (cr);
	cairo_destroy (cr);
	return TRUE;
}

static void font_button_clicked (GtkFontButton *button)
{
	const gchar *font;
	PangoFontDescription *desc;

	font = gtk_font_button_get_font_name (button);
	desc = pango_font_description_from_string (font);

	set_font (desc);
}

static void g_button_clicked (GObject *button, GtkWidget *str)
{
	printf ("Current text %s\n", gtk_entry_get_text (GTK_ENTRY (str)));
}

static void d_button_clicked (GObject *button, GtkWidget *str)
{
	printf ("Current text %s\n", gtk_entry_get_text (GTK_ENTRY (str)));
}

int main(int argc, char *argv[]) 
{
	GtkWidget *window;
	GtkWidget *canvas;
	GtkWidget *font_button;
	GtkWidget *g_button;
	GtkWidget *d_button;
	GtkWidget *str;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	PangoFontDescription *desc;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(window), "Font converter");

	hbox = gtk_hbox_new (TRUE, 5);
	label = gtk_label_new ("Text: ");
	font_button = gtk_font_button_new_with_font ("Sans Bold 12");
	d_button = gtk_button_new_with_label ("Draw");
	g_button = gtk_button_new_with_label ("G Code");
	str = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (str), 15);
	gtk_entry_set_text (GTK_ENTRY (str), "AWP");

	gtk_box_pack_start_defaults (GTK_BOX (hbox), label);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), str);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), font_button);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), d_button);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), g_button);

	canvas = gtk_drawing_area_new();
	gtk_widget_set_size_request (canvas, 1200, 600);

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), canvas);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), hbox);

	gtk_container_add (GTK_CONTAINER (window), vbox);

	g_signal_connect(window, "destroy",
		G_CALLBACK(gtk_main_quit), NULL);  

	g_signal_connect(G_OBJECT (canvas), "expose_event",
		G_CALLBACK(expose_event), NULL);  

	g_signal_connect(G_OBJECT (font_button), "font_set",
		G_CALLBACK(font_button_clicked), NULL);  

	g_signal_connect(G_OBJECT (g_button), "clicked",
		G_CALLBACK(g_button_clicked), str);  

	g_signal_connect(G_OBJECT (d_button), "clicked",
		G_CALLBACK(d_button_clicked), str);  

	gtk_widget_show_all(window);
	map = pango_ft2_font_map_new ();
	ctx = pango_font_map_create_context (map);
	desc = pango_font_description_from_string ("Sans Bold 12");
	set_font (desc);

	gtk_main();

	g_object_unref (map);
	g_object_unref (ctx);
	return 0;
}
