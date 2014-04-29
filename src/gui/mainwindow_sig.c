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

#include "mainwindow_sig.h"
#include "../converters/svg.h"
#include "../converters/png.h"
#include "../converters/pdf.h"
#include "../converters/json.h"
#include "../parsers/wpi.h"
#include "../datatypes/element.h"
#include "../datatypes/configuration.h"

#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include <string.h>
#include <librsvg/rsvg.h>
#include <cairo-pdf.h>

#define PT_TO_MM 2.8333

extern GtkWidget* document_view;
extern GtkWidget* hbox_colors;
extern GtkWidget* window;
extern GSList* documents;
extern dt_configuration settings;

static GSList* parsed_data = NULL;
static RsvgHandle* handle = NULL;

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_REDISPLAY                                                   |
 '----------------------------------------------------------------------------*/
static void
gui_mainwindow_redisplay ()
{
  /* Clean up the (old) RsvgHandle data when it's set at this point. */
  if (handle != NULL)
    g_object_unref (handle), handle = NULL;

  gtk_widget_hide (document_view);
  gtk_widget_show_all (document_view);
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
    dialog = gtk_file_chooser_dialog_new ("Open file", 
      GTK_WINDOW (parent), action, "Cancel", GTK_RESPONSE_CANCEL, 
     "Open", GTK_RESPONSE_ACCEPT, NULL);
  else if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
    dialog = gtk_file_chooser_dialog_new ("Save file", 
      GTK_WINDOW (parent), action, "Cancel", GTK_RESPONSE_CANCEL, 
     "Save", GTK_RESPONSE_ACCEPT, NULL);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

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

      gtk_widget_hide (document_view);
      gtk_widget_show_all (document_view);
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

  if (filename != NULL)
    {
      char* ext = filename + strlen (filename) - 4;
      if (!strcmp (ext, ".png") || !strcmp (ext, ".svg") || !strcmp (ext, ".pdf") || !strcmp (ext, "json"))
	{
	  if (!strcmp (ext, ".png") && CAIRO_HAS_PNG_FUNCTIONS)
	    co_png_export_to_file_from_handle (filename, handle);

	  else if (!strcmp (ext, ".pdf"))
	    co_pdf_export_to_file_from_handle (filename, handle);

	  else if (!strcmp (ext, "json"))
	    co_json_create_file (filename, parsed_data);

	  else if (!strcmp (ext, ".svg"))
	    {
	      FILE* file;
	      file = fopen (filename, "w");
	      if (file != NULL)
		{
		  char* svg_data = co_svg_create (parsed_data, NULL);
		  fwrite (svg_data, strlen (svg_data), 1, file);
		}
	      fclose (file);
	    }
	}
	
      g_free (filename);
    }
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_DOCUMENT_VIEW_DRAW                                          |
 | This callback function handles the drawing on the 'document_view' widget.  |
 '----------------------------------------------------------------------------*/
gboolean
gui_mainwindow_document_view_draw (GtkWidget *widget, cairo_t *cr, void* data)
{
  if (parsed_data == NULL && handle == NULL) return 0;

  double w = gtk_widget_get_allocated_width (widget);
  double ratio = w / (settings.page.width * PT_TO_MM * 1.25) / 1.10;
  double padding = (w - (settings.page.width * PT_TO_MM * 1.25 * ratio)) / 2;
  double h = w * (settings.page.height * PT_TO_MM) / (settings.page.width * PT_TO_MM) - padding;

  cairo_translate (cr, padding, padding);
  cairo_scale (cr, ratio, ratio);

  if (handle)
    {
      rsvg_handle_render_cairo (handle, cr);
      rsvg_handle_close (handle, NULL);
    }
  else if (parsed_data)
    {
      char* svg_data = co_svg_create (parsed_data, NULL);

      handle = rsvg_handle_new_from_data ((unsigned char*)svg_data, strlen (svg_data), NULL);
      free (svg_data), svg_data = NULL;
      rsvg_handle_render_cairo (handle, cr);
      rsvg_handle_close (handle, NULL);
    }

  gtk_widget_set_size_request (widget, 0, h);

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
  settings.pressure_factor = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
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
