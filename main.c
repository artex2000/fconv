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

void set_font (PangoFontDescription *desc);

static gboolean
expose_event (GtkWidget *widget, GdkEventExpose *event, GtkWidget *str)
{
	cairo_t *cr;
	double cw, ch;
	int i;
	char *s;

	s = (char *) gtk_entry_get_text (GTK_ENTRY (str));
	cr = gdk_cairo_create (widget->window);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_set_line_width (cr, 0.5);
	cw = (double)widget->allocation.width;
	ch = (double)widget->allocation.height;
	generate_image (s, cr, cw, ch);	
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
	char *s;
	double sz;
	GtkWidget *spin;

	s = (char *) gtk_entry_get_text (GTK_ENTRY (str));
	spin = (GtkWidget *)g_object_get_data (button, "spin");
	sz = gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin));
	generate_gcode (s, sz);
}

static void d_button_clicked (GObject *button, GtkWidget *canvas)
{
	gtk_widget_queue_draw (canvas);
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
	GtkWidget *label1, *label2;
	GtkWidget *spin;
	PangoFontDescription *desc;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(window), "Font converter");

	hbox = gtk_hbox_new (FALSE, 10);
	label1 = gtk_label_new ("Text: ");
	font_button = gtk_font_button_new_with_font ("Gunplay 12");
	d_button = gtk_button_new_with_label ("Draw");
	g_button = gtk_button_new_with_label ("G Code");
	str = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (str), 15);
	gtk_entry_set_text (GTK_ENTRY (str), "NOPARKING");
	label2 = gtk_label_new ("Width (inch): ");
	spin = gtk_spin_button_new_with_range (3, 120, 1);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON (spin), 2);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), (double)10);

	gtk_box_pack_start (GTK_BOX (hbox), label1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), str, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), font_button, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), d_button, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), g_button, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), label2, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, FALSE, 0);

	canvas = gtk_drawing_area_new();
	gtk_widget_set_size_request (canvas, 1200, 600);

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_box_pack_start (GTK_BOX (vbox), canvas, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	g_object_set_data (G_OBJECT (g_button), "spin", spin);

	gtk_container_add (GTK_CONTAINER (window), vbox);

	g_signal_connect(window, "destroy",
		G_CALLBACK(gtk_main_quit), NULL);  

	g_signal_connect(G_OBJECT (canvas), "expose_event",
		G_CALLBACK(expose_event), str);  

	g_signal_connect(G_OBJECT (font_button), "font_set",
		G_CALLBACK(font_button_clicked), NULL);  

	g_signal_connect(G_OBJECT (g_button), "clicked",
		G_CALLBACK(g_button_clicked), str);  

	g_signal_connect(G_OBJECT (d_button), "clicked",
		G_CALLBACK(d_button_clicked), canvas);  

	gtk_widget_show_all(window);
	map = pango_ft2_font_map_new ();
	ctx = pango_font_map_create_context (map);
	desc = pango_font_description_from_string ("Gunplay 12");
	set_font (desc);

	gtk_main();

	g_object_unref (map);
	g_object_unref (ctx);
	return 0;
}
