/*
 * Copyright (C) 2013  Roel Janssen <roel@moefel.org>
 *
 * This file is part of InklingReader
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "mainwindow_sig.h"
#include <gtk/gtk.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

GtkWidget* lbl_status = NULL;
GSList* documents = NULL;

void
gui_init_mainwindow (int argc, char** argv)
{
  /*--------------------------------------------------------------------------.
   | WIDGETS                                                                  |
   '--------------------------------------------------------------------------*/
  GtkWidget* window = NULL;

  GtkWidget* btn_load = NULL;
  GtkWidget* menu_load = NULL;
  GtkWidget* menu_load_file = NULL;
  GtkWidget* menu_load_directory = NULL;

  GtkWidget* document_view = NULL;

  GtkWidget* vbox_window = NULL;
  GtkWidget* hbox_menu_top = NULL;


  /*--------------------------------------------------------------------------.
   | INIT AND CREATION OF WIDGETS                                             |
   '--------------------------------------------------------------------------*/
  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  btn_load = gtk_menu_button_new ();
  menu_load = gtk_menu_new ();
  menu_load_file = gtk_menu_item_new_with_label ("Open file");
  menu_load_directory = gtk_menu_item_new_with_label ("Open directory");

  vbox_window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  hbox_menu_top = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);

  document_view = gtk_drawing_area_new ();
  lbl_status = gtk_label_new ("Please open a file or folder.");


  /*--------------------------------------------------------------------------.
   | FURTHER CONFIGURATION                                                    |
   '--------------------------------------------------------------------------*/
  gtk_window_set_title (GTK_WINDOW (window), "InklingTool");
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_widget_set_size_request (window, WINDOW_WIDTH, WINDOW_HEIGHT);

  gtk_menu_button_set_direction (GTK_MENU_BUTTON (btn_load), GTK_ARROW_DOWN);
  gtk_menu_button_set_popup (GTK_MENU_BUTTON (btn_load), menu_load);

  gtk_menu_attach (GTK_MENU (menu_load), menu_load_file, 0, 1, 0, 1);
  gtk_menu_attach (GTK_MENU (menu_load), menu_load_directory, 0, 1, 1, 2);

  GdkRGBA bg;
  gdk_rgba_parse (&bg, "#101010");
  gtk_widget_override_background_color (document_view, GTK_STATE_NORMAL, &bg);


  /*--------------------------------------------------------------------------.
   | CONTAINERS                                                               |
   '--------------------------------------------------------------------------*/
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), btn_load, 0, 0, 10);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), lbl_status, 1, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_window), hbox_menu_top, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_window), document_view, 1, 1, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox_window);


  /*--------------------------------------------------------------------------.
   | SIGNALS                                                                  |
   '--------------------------------------------------------------------------*/
  g_signal_connect (G_OBJECT (window), "destroy", 
		    G_CALLBACK (gtk_main_quit), NULL);

  g_signal_connect (G_OBJECT (menu_load_file), "activate", 
		    G_CALLBACK (gui_mainwindow_file_activated), NULL);

  g_signal_connect (G_OBJECT (menu_load_directory), "activate", 
		    G_CALLBACK (gui_mainwindow_directory_activated), NULL);

  g_signal_connect (G_OBJECT (document_view), "draw",
                    G_CALLBACK (gui_mainwindow_document_view_draw), NULL);


  /*--------------------------------------------------------------------------.
   | DISPLAY                                                                  |
   '--------------------------------------------------------------------------*/
  gtk_widget_show (GTK_WIDGET (menu_load_file));
  gtk_widget_show (GTK_WIDGET (menu_load_directory));

  gtk_widget_show_all (window);
  gtk_main ();
}
