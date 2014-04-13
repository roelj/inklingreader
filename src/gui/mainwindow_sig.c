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
#include "../parsers/wpi.h"
#include "../datatypes/element.h"

#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <librsvg/rsvg.h>
#include <cairo-pdf.h>

#define BTN_DOCUMENT_WIDTH  100
#define BTN_DOCUMENT_HEIGHT 100
#define A4_WIDTH 595.0
#define A4_HEIGHT 842.0

extern GtkWidget* document_view;
extern GtkWidget* lbl_status;
extern GSList* documents;

#define DOCUMENT_PADDING 25.0

GSList* parsed_data = NULL;
RsvgHandle* handle = NULL;
char* svg_data = NULL;
int current_view = -1;
char* directory_name = NULL;


/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_MENU_FILE_ACTIVATE                                          |
 | This event handler handles the activation of a menu item within the "File" |
 | menu.                                                                      |
 '----------------------------------------------------------------------------*/
void 
gui_mainwindow_menu_file_activate (GtkWidget* widget, void* data)
{
  const char* label = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));

  if (!strcmp (label, "Open file"))
    gui_mainwindow_file_activated (widget, data);

  else if (!strcmp (label, "Open directory"))
    gui_mainwindow_directory_activated (widget, data);

  else if (!strcmp (label, "Export file"))
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
 | This callback function handles activating the "Open file" menu button.     |
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

  /* When the filename is not NULL anymore, we can process it. */
  if (filename)
    {
      parsed_data = p_wpi_parse (filename);

      /* Only continue when data has been parsed. */
      if (parsed_data)
	{
	  size_t status_len = 18 + strlen (filename);
	  char* status = malloc (status_len);
	  if (status)
	    {
	      snprintf (status, status_len, "Now displaying: %s", filename);
	      gtk_label_set_text (GTK_LABEL (lbl_status), status);
	      free (status);
	    }
	}

      /* Clean up the filename if it was gathered using the dialog. */
      if (!data)
	g_free (filename);

      /* Clean up the (old) RsvgHandle data when it's set at this point. */
      if (handle)
	{
	  g_object_unref (handle);
	  handle = NULL;
	}

      /* Clean up the (old) SVG data. */
      if (svg_data)
	{
	  free (svg_data);
	  svg_data = NULL;
	}

      /* Make sure we are in VIEW_DOCUMENT mode. */
      current_view = VIEW_DOCUMENT;
    }

  gtk_widget_hide (document_view);
  gtk_widget_show_all (document_view);
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_EXPORT_ACTIVATED                                            |
 | This callback function handles activating the "Export file" menu button.   |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_export_activated (GtkWidget* widget, void* data)
{
  GtkWidget *parent = gtk_widget_get_toplevel (widget);
  char* filename = gui_mainwindow_file_dialog (parent, GTK_FILE_CHOOSER_ACTION_SAVE);

  if (filename != NULL)
    {
      char* ext = filename + strlen (filename) - 3;
      if (!strcmp (ext, "png") || !strcmp (ext, "svg") || !strcmp (ext, "pdf"))
	{	
	  size_t status_len = 29 + strlen (filename);
	  char* status = malloc (status_len);
	  if (status != NULL)
	    {
	      if (!strcmp (ext, "png") && CAIRO_HAS_PNG_FUNCTIONS)
		co_png_export_to_file_from_handle (filename, handle);

	      else if (!strcmp (ext, "pdf"))
		co_pdf_export_to_file_from_handle (filename, handle);

	      else if (!strcmp (ext, "svg"))
		{
		  FILE* file;
		  file = fopen (filename, "w");
		  if (file != NULL)
		    fwrite (svg_data, strlen (svg_data), 1, file);

		  fclose (file);
		}

	      snprintf (status, status_len, "The file has been saved as: %s", filename);
	      gtk_label_set_text (GTK_LABEL (lbl_status), status);
	      free (status);
	    }
	}
      else
	gtk_label_set_text (GTK_LABEL (lbl_status), 
	  "Only PNG (.png), SVG (.svg) and PDF (.pdf) are supported.");
	
      g_free (filename);
    }
}


/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_DIRECTORY_ACTIVATED                                         |
 | This callback function handles activating the "Open directory" menu item.  |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_directory_activated (GtkWidget* widget, void* data)
{
  GtkWidget *parent = gtk_widget_get_toplevel (widget);
  char* filename = gui_mainwindow_file_dialog (parent, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

  if (filename != NULL)
    gtk_label_set_text (GTK_LABEL (lbl_status), "You can now choose a file below.");

  current_view = VIEW_DIRECTORY;
  directory_name = filename;
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_DIRECTORY_DRAW                                              |
 | This function draws documents on 'widget' that are found in 'directory'.   |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_directory_draw (GtkWidget* widget, const char* path)
{
  DIR* directory;
  struct dirent* entry;

  directory = opendir (path);
  while ((entry = readdir (directory)) != NULL)
    {
      /* Don't show files starting with a dot, '.' and '..' and only show 
       * files with the WPI extension (others are not relevant). */
      char* extension = entry->d_name + strlen (entry->d_name) - 3;
      int x = 1, y = 1;
      if (entry->d_name[0] != '.' && !strcmp (extension, "WPI"))
	{
	  size_t name_len = strlen (path) + strlen (entry->d_name) + 2;
	  char* name = malloc (name_len);
	  if (name == NULL) break;

	  /* Construct a string that holds "path/name". */
	  snprintf (name, name_len, "%s/%s", path, entry->d_name);
	  GtkWidget* btn_document = gtk_button_new ();
	  GtkWidget* da_document = gtk_drawing_area_new ();
	  gtk_widget_set_size_request (da_document, BTN_DOCUMENT_WIDTH, BTN_DOCUMENT_HEIGHT);
	  gtk_container_add (GTK_CONTAINER (btn_document), da_document);

	  free (name);
	  x++; 
	  y++;
	}
    }

  closedir (directory);
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_DIR_ENTRY_DRAW                                              |
 | This function draws a placeholder icon and the file's name for a document. |
 '----------------------------------------------------------------------------*/
void 
gui_mainwindow_dir_entry_draw (GtkWidget* widget, cairo_t* cr, void* data)
{
  //char* filename = (char*)data;
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_DOCUMENT_VIEW_DRAW                                          |
 | This callback function handles the drawing on the 'document_view' widget.  |
 '----------------------------------------------------------------------------*/
gboolean
gui_mainwindow_document_view_draw (GtkWidget *widget, cairo_t *cr, void* data)
{
  switch (current_view)
    {
    case VIEW_DOCUMENT:
      {
	double w = gtk_widget_get_allocated_width (widget);
	double ratio = w / (A4_WIDTH * 1.25) / 1.25;
	double padding = (w - (A4_WIDTH * 1.25 * ratio)) / 2;
	double h = w * A4_HEIGHT / A4_WIDTH - padding * 2;

	cairo_translate (cr, padding, 30.0);
	cairo_scale (cr, ratio, ratio);

	if (parsed_data && !handle)
	  {
	    svg_data = co_svg_create (parsed_data, NULL);
	    size_t svg_data_len = strlen (svg_data);

	    handle = rsvg_handle_new_from_data ((unsigned char*)svg_data, svg_data_len, NULL);
	    rsvg_handle_render_cairo (handle, cr);
	    rsvg_handle_close (handle, NULL);
	  }
	else if (handle)
	  {
	    rsvg_handle_render_cairo (handle, cr);
	    rsvg_handle_close (handle, NULL);
	  }

	gtk_widget_set_size_request(widget, 0, h);

      }
      break;
    case VIEW_DIRECTORY:
      {
	gui_mainwindow_directory_draw (widget, directory_name);
	free (directory_name); directory_name = NULL;
      }
      break;
    }
  return 0;
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_QUIT                                                        |
 | Clean up when quitting.                                                    |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_quit ()
{
  if (svg_data != NULL)
    free (svg_data);

  if (handle != NULL)
    g_object_unref (handle);

  if (parsed_data != NULL)
    g_slist_free (parsed_data);

  gtk_main_quit();
}
