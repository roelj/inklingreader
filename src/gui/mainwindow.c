#include "mainwindow.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

void
gui_init_mainwindow (int argc, char** argv)
{

  /*--------------------------------------------------------------------------.
   | WIDGETS                                                                  |
   '--------------------------------------------------------------------------*/
  GtkWidget* window = NULL;
  GtkWidget* window_bg = NULL;
  GtkWidget* window_vbox = NULL;

  GdkRGBA bg_color;

  /*--------------------------------------------------------------------------.
   | INIT                                                                     |
   '--------------------------------------------------------------------------*/
  gtk_init (&argc, &argv);
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (window), "InklingTool");
  gtk_widget_set_size_request (window, WINDOW_WIDTH, WINDOW_HEIGHT);

  /*--------------------------------------------------------------------------.
   | SETUP                                                                    |
   '--------------------------------------------------------------------------*/
  window_bg = gtk_layout_new (NULL, NULL);

  gdk_rgba_parse (&bg_color, "#333333");
  gtk_widget_override_background_color (window_bg, GTK_STATE_NORMAL, &bg_color);

  /*--------------------------------------------------------------------------.
   | CONTAINERS                                                               |
   '--------------------------------------------------------------------------*/
  window_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  gtk_box_pack_start (GTK_BOX (window_vbox), window_bg, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (window), window_vbox);

  /*--------------------------------------------------------------------------.
   | SIGNALS                                                                  |
   '--------------------------------------------------------------------------*/
  g_signal_connect ( G_OBJECT (window), "destroy",
		     G_CALLBACK (gtk_main_quit), NULL);


  gtk_widget_show_all (window);
  gtk_main ();
}
