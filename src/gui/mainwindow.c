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
#include "../datatypes/configuration.h"
#include "../converters/svg.h"
#include "../converters/png.h"
#include "../converters/pdf.h"
#include "../converters/json.h"
#include "../parsers/wpi.h"
#include "../datatypes/element.h"
#include "../high/conversion.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <librsvg/rsvg.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PT_TO_MM 2.8333
#define MINIMAL_PADDING 10

extern dt_configuration settings;

static GtkWidget* zoom_toggle = NULL;
static GtkWidget* zoom_input = NULL;
static GtkWidget* pressure_toggle = NULL;
static GtkWidget* pressure_input = NULL;
static GtkWidget* document_view = NULL;
static GtkWidget* hbox_colors = NULL;
static GtkWidget* window = NULL;
static GSList* parsed_data = NULL;
static RsvgHandle* handle = NULL;
static char* last_file_extension = NULL;
static const int file_filters_num = 4;

static const char* file_mimetypes[]  = { 
  "application/pdf", 
  "image/png", 
  "image/svg+xml", 
  "application/json"
};

static const char* file_extensions[] = { 
  "PDF - Portable Document Format",
  "PNG - Portable Network Graphics",
  "SVG - Scalable Vector Graphics",
  "JSON - JavaScript Object Notation"
};

#define MENU_ITEMS_NUM 3
const char* menu_items[] = { "Open", "Export", "Quit" };

void
gui_mainwindow_init (int argc, char** argv, const char* filename)
{
  /*--------------------------------------------------------------------------.
   | WIDGETS                                                                  |
   '--------------------------------------------------------------------------*/
  GtkWidget* document_viewport = NULL;
  GtkWidget* document_container = NULL;

  GtkWidget* vbox_window = NULL;
  GtkWidget* hbox_menu_top = NULL;

  GtkWidget* menu_bar = NULL;
  GtkWidget* menu_bar_file = NULL;
  GtkWidget* menu_file = NULL;

  GtkWidget* new_color_button = NULL;
  GtkWidget* bg_color_button = NULL;
  GtkWidget* bg_color_label = NULL;
  GtkWidget* fg_color_label = NULL;
  GtkWidget* pressure_label = NULL;
  GtkWidget* zoom_label = NULL;

  /*--------------------------------------------------------------------------.
   | INIT AND CREATION OF WIDGETS                                             |
   '--------------------------------------------------------------------------*/
  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  vbox_window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  hbox_menu_top = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  hbox_colors = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  document_container = gtk_scrolled_window_new (NULL, NULL);
  document_viewport = gtk_viewport_new (NULL, NULL);
  document_view = gtk_drawing_area_new ();

  menu_bar = gtk_menu_bar_new ();
  menu_bar_file = gtk_menu_item_new_with_label ("File");
  menu_file = gtk_menu_new ();

  new_color_button = gtk_button_new_with_label ("+");

  pressure_label = gtk_label_new ("");
  pressure_input = gtk_spin_button_new_with_range (0, 1000.0, 0.05);
  pressure_toggle = gtk_switch_new ();

  zoom_label = gtk_label_new ("");
  zoom_input = gtk_spin_button_new_with_range (10.0, 1000.0, 10.0);
  zoom_toggle = gtk_switch_new ();

  GdkRGBA bg_doc_color;
  if (settings.background != NULL)
    gdk_rgba_parse (&bg_doc_color, settings.background);
  else
    gdk_rgba_parse (&bg_doc_color, "#fff");

  bg_color_button = gtk_color_button_new_with_rgba (&bg_doc_color);
  bg_color_label = gtk_label_new ("");
  fg_color_label = gtk_label_new ("");

  /* Add the (already existing) colors as buttons to the color bar. */
  int a = 0;
  for (; a < settings.num_colors; a++)
    {
      GdkRGBA color;
      gdk_rgba_parse (&color, settings.colors[a]);
      GtkWidget* btn_color = gtk_color_button_new_with_rgba (&color);

      /* Attach a signal to it for resetting the color. */
      int* number = malloc (sizeof (int));
      *number = a;

      g_signal_connect (G_OBJECT (btn_color), "color-set",
			G_CALLBACK (gui_mainwindow_set_fg_color), number);

      gtk_box_pack_start (GTK_BOX (hbox_colors), btn_color, 0, 0, 0);
    }

  /* Add the menu items to the menu. */
  for (a = 0; a < MENU_ITEMS_NUM; a++)
    {
      GtkWidget* menu_item = gtk_menu_item_new_with_label (menu_items[a]);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu_file), menu_item);

      g_signal_connect (G_OBJECT (menu_item), "activate",
			G_CALLBACK (gui_mainwindow_menu_file_activate), NULL);

    }

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_file), menu_file);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), menu_bar_file);

  /*--------------------------------------------------------------------------.
   | FURTHER CONFIGURATION                                                    |
   '--------------------------------------------------------------------------*/
  gtk_window_set_title (GTK_WINDOW (window), "InklingReader");
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_widget_set_size_request (window, WINDOW_WIDTH, WINDOW_HEIGHT);

  GdkRGBA bg;
  gdk_rgba_parse (&bg, "#101010");
  gtk_widget_override_background_color (document_view, GTK_STATE_FLAG_NORMAL, &bg);

  gtk_container_add (GTK_CONTAINER (document_viewport), document_view);
  gtk_container_add (GTK_CONTAINER (document_container), document_viewport);

  gtk_label_set_markup (GTK_LABEL (bg_color_label), "<b>B:</b>");
  gtk_label_set_markup (GTK_LABEL (fg_color_label), "<b>F:</b>");
  gtk_label_set_markup (GTK_LABEL (pressure_label), "<b>P:</b>");
  gtk_label_set_markup (GTK_LABEL (zoom_label), "<b>Z:</b>");

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (pressure_input), settings.pressure_factor);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (zoom_input), 100.0);
  gtk_switch_set_active (GTK_SWITCH (zoom_toggle), FALSE);
  gtk_switch_set_active (GTK_SWITCH (pressure_toggle), TRUE);

  /*--------------------------------------------------------------------------.
   | CONTAINERS                                                               |
   '--------------------------------------------------------------------------*/
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), menu_bar, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_window), hbox_menu_top, 0, 0, 0);

  gtk_box_pack_start (GTK_BOX (hbox_menu_top), bg_color_label, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), bg_color_button, 0, 0, 5);

  gtk_box_pack_start (GTK_BOX (hbox_menu_top), fg_color_label, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), hbox_colors, 0, 0, 0);
  gtk_box_pack_end (GTK_BOX (hbox_colors), new_color_button, 0, 0, 5);

  gtk_box_pack_start (GTK_BOX (hbox_menu_top), pressure_label, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), pressure_input, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), pressure_toggle, 0, 0, 5);

  gtk_box_pack_start (GTK_BOX (hbox_menu_top), zoom_label, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), zoom_input, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), zoom_toggle, 0, 0, 5);

  gtk_box_pack_start (GTK_BOX (vbox_window), document_container, 1, 1, 0);

  gtk_container_add (GTK_CONTAINER (window), vbox_window);

  /*--------------------------------------------------------------------------.
   | SIGNALS                                                                  |
   '--------------------------------------------------------------------------*/
  g_signal_connect (G_OBJECT (window), "destroy", 
		    G_CALLBACK (gui_mainwindow_quit), NULL);

  g_signal_connect (G_OBJECT (new_color_button), "clicked",
		    G_CALLBACK (gui_mainwindow_add_color), NULL);

  g_signal_connect (G_OBJECT (document_view), "draw",
                    G_CALLBACK (gui_mainwindow_document_view_draw), NULL);

  g_signal_connect (G_OBJECT (bg_color_button), "color-set",
		    G_CALLBACK (gui_mainwindow_set_bg_color), NULL);

  g_signal_connect (G_OBJECT (pressure_input), "value-changed",
		    G_CALLBACK (gui_mainwindow_set_pressure_input), NULL);

  g_signal_connect (G_OBJECT (zoom_input), "value-changed",
		    G_CALLBACK (gui_mainwindow_set_zoom_input), NULL);

  g_signal_connect (G_OBJECT (zoom_toggle), "notify::active",
		    G_CALLBACK (gui_mainwindow_set_zoom_toggle), NULL);

  g_signal_connect (G_OBJECT (pressure_toggle), "notify::active",
		    G_CALLBACK (gui_mainwindow_set_pressure_toggle), NULL);

  /*--------------------------------------------------------------------------.
   | DISPLAY                                                                  |
   '--------------------------------------------------------------------------*/

  gtk_widget_show_all (window);
  gtk_widget_hide (zoom_input);

  if (filename)
    gui_mainwindow_file_activated (NULL, (char*)filename);

  gtk_main ();
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_REDISPLAY                                                   |
 '----------------------------------------------------------------------------*/
static void
gui_mainwindow_redisplay ()
{
  /* Clean up the (old) RsvgHandle data when it's set at this point. */
  if (handle != NULL)
    g_object_unref (handle), handle = NULL;

  gtk_widget_queue_draw (document_view);
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_MENU_FILE_ACTIVATE                                          |
 | This event handler handles the activation of a menu item within the "File" |
 | menu.                                                                      |
 '----------------------------------------------------------------------------*/
void 
gui_mainwindow_menu_file_activate (GtkWidget* widget, void* data)
{
  const char* label = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));

  if (!strcmp (label, "Open"))
    gui_mainwindow_file_activated (widget, data);

  else if (!strcmp (label, "Export"))
    gui_mainwindow_export_activated (widget, data);

  else if (!strcmp (label, "Quit"))
    gui_mainwindow_quit ();
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_FILE_DIALOG                                                 |
 | This helper function opens a file dialog and returns the filename after    |
 | closing it. When no file is chosen, NULL is returned.                      |
 '----------------------------------------------------------------------------*/
char*
gui_mainwindow_file_dialog (GtkWidget* parent, GtkFileChooserAction action)
{
  char *filename = NULL;
  GtkWidget *dialog = NULL;

  if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
    {
      dialog = gtk_file_chooser_dialog_new ("Open file", 
	         GTK_WINDOW (parent), action, "Cancel", GTK_RESPONSE_CANCEL, 
		 "Open", GTK_RESPONSE_ACCEPT, NULL);

      /* Set a filter for WPI files (it doesn't have a specific mimetype). */
      GtkFileFilter* filter = gtk_file_filter_new ();
      gtk_file_filter_set_name (filter, "WPI - Wacom Proprietary Ink");
      gtk_file_filter_add_mime_type (filter, "application/octet-stream");
      gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
    }
  else if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
    {
      dialog = gtk_file_chooser_dialog_new ("Save file", 
	         GTK_WINDOW (parent), action, "Cancel", GTK_RESPONSE_CANCEL, 
	         "Save", GTK_RESPONSE_ACCEPT, NULL);

      /* Add filters for supported formats. */
      int a = 0;
      for (; a < file_filters_num; a++)
	{
	  GtkFileFilter* filter = gtk_file_filter_new ();
	  gtk_file_filter_set_name (filter, file_extensions[a]);
	  gtk_file_filter_add_mime_type (filter, file_mimetypes[a]);
	  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	}
    }

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      GtkFileFilter* chosen_filter = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (dialog));
      const char* filter_name = gtk_file_filter_get_name (chosen_filter);

      /* Clean up the memory of the old string. */
      g_free (last_file_extension);

      last_file_extension = g_ascii_strdown (filter_name, 4);
      if (g_ascii_isspace (last_file_extension[3])) last_file_extension[3] = '\0';
    }
  gtk_widget_destroy (dialog);

  return filename;
}


/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_FILE_ACTIVATED                                              |
 | This callback function handles activating the "Open" menu button.          |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_file_activated (GtkWidget* widget, void* data)
{
  char* filename = NULL;

  /* The filename can be passed by 'data'. Otherwise we need to show a
   * dialog to the user to choose a file. */
  if (data)
    filename = (char*)data;
  else
    {
      GtkWidget *parent = gtk_widget_get_toplevel (widget);
      filename = gui_mainwindow_file_dialog (parent, GTK_FILE_CHOOSER_ACTION_OPEN);
    }

  if (filename != NULL)
    {
      char* window_title = malloc (16 + strlen (filename) + 1);
      sprintf (window_title, "InklingReader: %s", filename);
      gtk_window_set_title (GTK_WINDOW (window), window_title);

      free (window_title);

      /* When the filename is not NULL anymore, we can process it. */
      if (filename)
	{
	  /* Clean-up the old parsed data. */
	  if (parsed_data)
	    p_wpi_cleanup (parsed_data);

	  parsed_data = p_wpi_parse (filename);

	  /* Clean up the filename if it was gathered using the dialog. */
	  if (!data)
	    g_free (filename);

	  /* Clean up the (old) RsvgHandle data when it's set at this point. */
	  if (handle)
	    g_object_unref (handle), handle = NULL;
	}

      gtk_widget_queue_draw (document_view);
    }
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_EXPORT_ACTIVATED                                            |
 | This callback function handles activating the "Export" menu button.        |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_export_activated (GtkWidget* widget, void* data)
{
  GtkWidget *parent = gtk_widget_get_toplevel (widget);
  char* filename = gui_mainwindow_file_dialog (parent, GTK_FILE_CHOOSER_ACTION_SAVE);

  if (last_file_extension)
    if (strcmp (last_file_extension, filename + strlen (filename) - strlen (last_file_extension)))
      {
        char* total_filename = g_strconcat (filename, ".", last_file_extension, NULL);
        g_free (filename), filename = total_filename;
      }

  if (filename == NULL) return;

  char* ext = strrchr (filename, '.');
  if (!strcmp (ext, ".png") && CAIRO_HAS_PNG_FUNCTIONS)
    co_png_export_to_file_from_handle (filename, handle);

  else if (!strcmp (ext, ".pdf"))
    co_pdf_export_to_file_from_handle (filename, handle);

  else if (!strcmp (ext, ".json"))
    co_json_create_file (filename, parsed_data);

  else if (!strcmp (ext, ".svg"))
    high_export_to_file (parsed_data, NULL, filename, &settings);

  g_free (filename);
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_DOCUMENT_VIEW_DRAW                                          |
 | This callback function handles the drawing on the 'document_view' widget.  |
 '----------------------------------------------------------------------------*/
gboolean
gui_mainwindow_document_view_draw (GtkWidget *widget, cairo_t *cr, void* data)
{
  if (parsed_data == NULL && handle == NULL) return 0;

  double w = gtk_widget_get_allocated_width (window);
  double ratio = 1.00;
  
  if (!gtk_widget_get_visible (zoom_input))
    ratio = w / (settings.page.width * PT_TO_MM * 1.25) / 1.10;
  else
    {
      ratio = gtk_spin_button_get_value (GTK_SPIN_BUTTON (zoom_input)) / 100.0;
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (zoom_input), ratio * 100);
    }

  double padding = (w - (settings.page.width * PT_TO_MM * 1.25 * ratio)) / 2;
  if (padding < 0) padding = 0;
  double h = settings.page.height * PT_TO_MM * 1.25 * ratio + padding * 2;
  w = settings.page.width * PT_TO_MM * 1.25 * ratio + padding + MINIMAL_PADDING;

  gtk_widget_set_size_request (widget, w, h);

  cairo_translate (cr, padding, padding);
  cairo_scale (cr, ratio, ratio);

  if (handle)
    {
      rsvg_handle_render_cairo (handle, cr);
      rsvg_handle_close (handle, NULL);
    }
  else if (parsed_data)
    {
      char* svg_data = co_svg_create (parsed_data, NULL, &settings);

      handle = rsvg_handle_new_from_data ((unsigned char*)svg_data, strlen (svg_data), NULL);
      free (svg_data), svg_data = NULL;
      rsvg_handle_render_cairo (handle, cr);
      rsvg_handle_close (handle, NULL);
    }

  return 0;
}


/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_ADD_COLOR                                                   |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_add_color (GtkWidget* widget, void* data)
{
  /* Show the color selection dialog. */
  GtkWidget* color_chooser = gtk_color_chooser_dialog_new ("Choose a color", NULL);
  GdkRGBA chosen_color;

  gtk_color_chooser_set_use_alpha (GTK_COLOR_CHOOSER (color_chooser), 0);
  int response = gtk_dialog_run (GTK_DIALOG (color_chooser));

  if (response == GTK_RESPONSE_OK)
    {
      gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (color_chooser), &chosen_color);

      /* Create a new color button. */
      GtkWidget* color = gtk_color_button_new_with_rgba (&chosen_color);

      /* Attach a signal to it for resetting the color. */
      int* number = malloc (sizeof (int));
      *number = settings.num_colors;

      g_signal_connect (G_OBJECT (color), "color-set",
			G_CALLBACK (gui_mainwindow_set_fg_color), number);

      /* It's not exact due to rounding, but for now it's close enough.. */
      unsigned int r = chosen_color.red * 0xFF,
	           g = chosen_color.green * 0xFF,
	           b = chosen_color.blue * 0xFF;

      if (!settings.colors)
	settings.colors = malloc (1 * sizeof (char*));
      else
	settings.colors = realloc (settings.colors, (settings.num_colors + 1) * sizeof (char*));

      settings.colors[settings.num_colors] = malloc (8);
      snprintf (settings.colors[settings.num_colors], 8, "#%02X%02x%02x", r, g, b);
      settings.num_colors++;

      gtk_box_pack_start (GTK_BOX (hbox_colors), color, 0, 0, 0);
      gtk_widget_show (color);

      gui_mainwindow_redisplay();
    }
  gtk_widget_destroy (color_chooser);
}



/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_BG_COLOR                                                |
 | This function is the callback for setting the background color.            |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_bg_color (GtkWidget* widget, void* data)
{
  GdkRGBA color;
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (widget), &color);

  /* It's not exact due to rounding, but for now it's close enough.. */
  unsigned int r = color.red * 0xFF,
               g = color.green * 0xFF,
               b = color.blue * 0xFF;

  if (settings.background != NULL)
    settings.background = malloc (8);
  else
    {
      free (settings.background), settings.background = NULL;
      settings.background = malloc (8);
    }
  snprintf (settings.background, 8, "#%02X%02x%02x", r, g, b);

  gui_mainwindow_redisplay();
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_FG_COLOR                                                |
 | This function is the callback for resetting a foreground color.            |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_fg_color (GtkWidget* widget, void* data)
{
  int number = *(int*)data;

  GdkRGBA color;
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (widget), &color);

  /* It's not exact due to rounding, but for now it's close enough.. */
  unsigned int r = color.red * 0xFF,
               g = color.green * 0xFF,
               b = color.blue * 0xFF;

  /* A color could be 'red', which is only 3 characters. We must make sure 
   * we have 8 bytes available. */
  if (strlen (settings.colors[number]) <= 7)
    {
      if (settings.colors[number] != NULL)
	free (settings.colors[number]), settings.colors[number] = NULL;
      settings.colors[number] = malloc (8);
    }

  snprintf (settings.colors[number], 8, "#%02X%02x%02x", r, g, b);

  gui_mainwindow_redisplay();
}


/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_PRESSURE_INPUT                                          |
 | This function is the callback for setting the pressure factor.             |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_pressure_input (GtkWidget* widget, void* data)
{
  if (gtk_widget_get_visible (pressure_input))
      settings.pressure_factor = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
  else
    settings.pressure_factor = 0.0;

  gui_mainwindow_redisplay();
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_ZOOM_INPUT                                              |
 | This function is the callback for setting the zoom factor.                 |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_zoom_input (GtkWidget* widget, void* data)
{
  gui_mainwindow_redisplay();
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_ZOOM_TOGGLE                                             |
 | This function is the callback for enabling or disabling the zoom factor.   |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_zoom_toggle (GtkWidget* widget, void* data)
{
  if (gtk_switch_get_active (GTK_SWITCH (widget)))
    {
      double w = gtk_widget_get_allocated_width (window);
      double ratio = w / (settings.page.width * PT_TO_MM * 1.25) / 1.10;
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (zoom_input), ratio * 100);

      gtk_widget_show (zoom_input);
    }
  else
    {
      gtk_widget_hide (zoom_input);
      gui_mainwindow_redisplay();
    }
}  

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_PRESSURE_TOGGLE                                         |
 | This callback is for enabling or disabling the pressure factor.            |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_pressure_toggle (GtkWidget* widget, void* data)
{
  if (gtk_switch_get_active (GTK_SWITCH (widget)))
    {
      settings.pressure_factor = gtk_spin_button_get_value (GTK_SPIN_BUTTON (pressure_input));
      gtk_widget_show (pressure_input);
    }
  else
    {
      settings.pressure_factor = 0.0;
      gtk_widget_hide (pressure_input);
    }
  gui_mainwindow_redisplay();
}  


/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_QUIT                                                        |
 | Clean up when quitting.                                                    |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_quit ()
{
  if (handle != NULL)
    g_object_unref (handle);

  if (parsed_data != NULL)
    p_wpi_cleanup (parsed_data);

  gtk_main_quit();
}
