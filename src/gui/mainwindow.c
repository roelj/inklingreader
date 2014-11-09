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
#include "../converters/csv.h"
#include "../parsers/wpi.h"
#include "../datatypes/element.h"
#include "../high/conversion.h"

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <librsvg/rsvg.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PT_TO_MM 2.8333

extern dt_configuration settings;
extern dt_preset_dimensions formats[];

static GtkWidget* header;
static GtkWidget* timing_button;
static GtkWidget* export_button;
static GtkWidget* zoom_input;
static GtkWidget* orientation_input;
static GtkWidget* dimensions_input;
static GtkWidget* pressure_input;
static GtkWidget* document_view;
static GtkWidget* hbox_color_buttons;
static GtkWidget* hbox_timing;
static GtkWidget* clock_scale;
static GSList* parsed_data;
static dt_metadata* metadata;
static RsvgHandle* handle;
static char* last_file_extension;
static char* last_dir;
static guint timeout_id = 0;
static gboolean is_playing = FALSE;
static GtkWidget* play_button;

static const char* file_mimetypes[]  = { 
  "application/pdf", 
  "image/png", 
  "image/svg+xml", 
  "application/json",
  "text/csv"
};

static const char* file_extensions[] = { 
  "PDF - Portable Document Format",
  "PNG - Portable Network Graphics",
  "SVG - Scalable Vector Graphics",
  "JSON - JavaScript Object Notation",
  "CSV - Comma separated values"
};

typedef enum
{
  FILE_OPEN,
  FILE_EXPORT
} FileActions;

void
gui_mainwindow_init (int argc, char** argv, const char* filename)
{
  /*--------------------------------------------------------------------------.
   | WIDGETS                                                                  |
   '--------------------------------------------------------------------------*/
  GtkWidget* window;
  GtkWidget* open_button;

  GtkWidget* vbox_settings;
  GtkWidget* settings_button;
  GtkWidget* settings_popover;

  GtkWidget* timing_popover;
  
  GtkWidget* hbox_colors;
  GtkWidget* hbox_bg_color;
  GtkWidget* hbox_zoom;
  GtkWidget* hbox_pressure;
  GtkWidget* hbox_dimensions;

  GtkWidget* document_viewport;
  GtkWidget* document_container;

  GtkWidget* vbox_window;

  GtkWidget* new_color_button;
  GtkWidget* bg_color_button;
  GtkWidget* bg_color_label;
  GtkWidget* fg_color_label;
  GtkWidget* pressure_label;
  GtkWidget* zoom_label;
  GtkWidget* dimensions_label;
  GtkWidget* forward_button;
  GtkWidget* backward_button;
  GtkWidget* zoom_toggle;
  GtkWidget* pressure_toggle;
  GtkWidget* save_config_button;

  /*--------------------------------------------------------------------------.
   | INIT AND CREATION OF WIDGETS                                             |
   '--------------------------------------------------------------------------*/
  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  /*--------------------------------------------------------------------------.
   | HEADER BAR AND BUTTONS                                                   |
   '--------------------------------------------------------------------------*/
  header = gtk_header_bar_new ();
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header), TRUE);
  gtk_header_bar_set_title (GTK_HEADER_BAR (header), "InklingReader");
  gtk_header_bar_set_has_subtitle (GTK_HEADER_BAR (header), FALSE);
  gtk_window_set_titlebar (GTK_WINDOW (window), header);

  open_button = gtk_button_new_with_label ("Open");
  export_button = gtk_button_new_with_label ("Export");
  settings_button = gtk_menu_button_new ();
  timing_button = gtk_menu_button_new ();
  save_config_button = gtk_button_new_with_label ("Save configuration");

  gtk_button_set_label (GTK_BUTTON (settings_button), "Settings");
  gtk_button_set_label (GTK_BUTTON (timing_button), "Timeline");

  settings_popover = gtk_popover_new (settings_button);
  timing_popover = gtk_popover_new (timing_button);

  gtk_menu_button_set_popover (GTK_MENU_BUTTON (settings_button), settings_popover);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (timing_button), timing_popover);
  
  gtk_header_bar_pack_start (GTK_HEADER_BAR (header), open_button);
  gtk_header_bar_pack_start (GTK_HEADER_BAR (header), export_button);
  gtk_header_bar_pack_end (GTK_HEADER_BAR (header), settings_button);
  gtk_header_bar_pack_end (GTK_HEADER_BAR (header), timing_button);

  /*--------------------------------------------------------------------------.
   | INITIALIZATION OF CONTAINERS                                             |
   '--------------------------------------------------------------------------*/
  vbox_window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  hbox_timing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);

  document_container = gtk_scrolled_window_new (NULL, NULL);
  document_viewport = gtk_viewport_new (NULL, NULL);
  document_view = gtk_drawing_area_new ();

  /*--------------------------------------------------------------------------.
   | SETTINGS POPOVER                                                         |
   '--------------------------------------------------------------------------*/

  vbox_settings = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  hbox_colors = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  hbox_color_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  hbox_bg_color = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  hbox_zoom = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  hbox_pressure = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  hbox_dimensions = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  new_color_button = gtk_button_new_with_label ("+");

  pressure_label = gtk_label_new ("");
  pressure_input = gtk_spin_button_new_with_range (0, 1000.0, 0.05);
  pressure_toggle = gtk_switch_new ();

  gtk_label_set_markup (GTK_LABEL (pressure_label), "<b>Pressure</b>");

  gtk_box_pack_start (GTK_BOX (hbox_pressure), pressure_label, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_pressure), pressure_toggle, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_pressure), pressure_input, 0, 0, 5);
  
  gtk_box_pack_start (GTK_BOX (vbox_settings), hbox_pressure, 0, 0, 10);
  gtk_widget_set_size_request (GTK_WIDGET (vbox_settings), 400, 200);
  
  zoom_label = gtk_label_new ("");
  zoom_input = gtk_spin_button_new_with_range (10.0, 1000.0, 10.0);
  zoom_toggle = gtk_switch_new ();

  gtk_label_set_markup (GTK_LABEL (zoom_label), "<b>Zoom</b>");

  gtk_box_pack_start (GTK_BOX (hbox_zoom), zoom_label, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_zoom), zoom_toggle, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_zoom), zoom_input, 0, 0, 5);
  
  gtk_box_pack_start (GTK_BOX (vbox_settings), hbox_zoom, 0, 0, 0);

  dimensions_label = gtk_label_new ("");
  dimensions_input = gtk_combo_box_text_new ();
  orientation_input = gtk_combo_box_text_new ();

  gtk_label_set_markup (GTK_LABEL (dimensions_label), "<b>Dimensions</b>");

  gtk_box_pack_start (GTK_BOX (hbox_dimensions), dimensions_label, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_dimensions), dimensions_input, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_dimensions), orientation_input, 0, 0, 5);

  gtk_box_pack_start (GTK_BOX (vbox_settings), hbox_dimensions, 0, 0, 10);

  fg_color_label = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (fg_color_label), "<b>Document colors</b>");
  gtk_box_pack_start (GTK_BOX (hbox_colors), fg_color_label, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_colors), new_color_button, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_colors), hbox_color_buttons, 0, 0, 5);

  /* Add the (already existing) colors as buttons to the color bar. */
  unsigned int a = 0;
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

      gtk_box_pack_start (GTK_BOX (hbox_color_buttons), btn_color, 0, 0, 0);
    }

  a = 0;
  while (formats[a].name != NULL)
    {
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (dimensions_input),
				 NULL, formats[a].name);
      a++;
    }

  gtk_box_pack_start (GTK_BOX (vbox_settings), hbox_colors, 0, 0, 0);

  GdkRGBA bg_doc_color;
  (settings.background != NULL)
    ? gdk_rgba_parse (&bg_doc_color, settings.background)
    : gdk_rgba_parse (&bg_doc_color, "#fff");

  bg_color_button = gtk_color_button_new_with_rgba (&bg_doc_color);
  bg_color_label = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (bg_color_label), "<b>Background color</b>");

  gtk_box_pack_start (GTK_BOX (hbox_bg_color), bg_color_label, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_bg_color), bg_color_button, 0, 0, 5);

  gtk_box_pack_start (GTK_BOX (vbox_settings), hbox_bg_color, 0, 0, 10);
  gtk_widget_show_all (vbox_settings);

  gtk_combo_box_set_active (GTK_COMBO_BOX (dimensions_input), 0);
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (orientation_input), NULL, "Portrait");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (orientation_input), NULL, "Landscape");

  (settings.page.orientation != NULL && !strcmp (settings.page.orientation, "Landscape"))
    ? gtk_combo_box_set_active (GTK_COMBO_BOX (orientation_input), 1)
    : gtk_combo_box_set_active (GTK_COMBO_BOX (orientation_input), 0);

  gtk_widget_show (save_config_button);
  gtk_box_pack_start (GTK_BOX (vbox_settings), save_config_button, 0, 1, 5);
  gtk_container_add (GTK_CONTAINER (settings_popover), vbox_settings);

  /*--------------------------------------------------------------------------.
   | PLAYBACK                                                                 |
   '--------------------------------------------------------------------------*/
  play_button = gtk_button_new_from_icon_name ("media-playback-start", GTK_ICON_SIZE_LARGE_TOOLBAR);
  forward_button = gtk_button_new_from_icon_name ("media-seek-forward", GTK_ICON_SIZE_LARGE_TOOLBAR);
  backward_button = gtk_button_new_from_icon_name ("media-seek-backward", GTK_ICON_SIZE_LARGE_TOOLBAR);
  
  clock_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 1);
  gtk_scale_set_value_pos (GTK_SCALE (clock_scale), GTK_POS_LEFT);
  
  /*--------------------------------------------------------------------------.
   | FURTHER CONFIGURATION                                                    |
   '--------------------------------------------------------------------------*/
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (window), WINDOW_WIDTH, WINDOW_HEIGHT);

  GdkRGBA bg;
  gdk_rgba_parse (&bg, "#101010");
  gtk_widget_override_background_color (document_view, GTK_STATE_FLAG_NORMAL, &bg);

  gtk_container_add (GTK_CONTAINER (document_viewport), document_view);
  gtk_container_add (GTK_CONTAINER (document_container), document_viewport);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (pressure_input), settings.pressure_factor);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (zoom_input), 100.0);
  gtk_switch_set_active (GTK_SWITCH (zoom_toggle), FALSE);
  gtk_switch_set_active (GTK_SWITCH (pressure_toggle), TRUE);

  /*--------------------------------------------------------------------------.
   | CONTAINERS                                                               |
   '--------------------------------------------------------------------------*/
  gtk_box_pack_start (GTK_BOX (vbox_window), document_container, 1, 1, 0);

  gtk_box_pack_start (GTK_BOX (hbox_timing), play_button, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_timing), backward_button, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_timing), forward_button, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_timing), clock_scale, 1, 1, 5);

  gtk_widget_show_all (GTK_WIDGET (hbox_timing));
  gtk_widget_set_size_request (GTK_WIDGET (hbox_timing), 600, 0);
  
  gtk_container_add (GTK_CONTAINER (timing_popover), hbox_timing);
  gtk_container_add (GTK_CONTAINER (window), vbox_window);

  /*--------------------------------------------------------------------------.
   | SIGNALS                                                                  |
   '--------------------------------------------------------------------------*/
  g_signal_connect (G_OBJECT (window), "destroy", 
		    G_CALLBACK (gui_mainwindow_quit), NULL);

  g_signal_connect (G_OBJECT (new_color_button), "clicked",
		    G_CALLBACK (gui_mainwindow_add_color), NULL);

  g_signal_connect (G_OBJECT (play_button), "clicked",
  		    G_CALLBACK (gui_mainwindow_play), NULL);

  g_signal_connect (G_OBJECT (forward_button), "clicked",
  		    G_CALLBACK (gui_mainwindow_forward), metadata);

  g_signal_connect (G_OBJECT (backward_button), "clicked",
  		    G_CALLBACK (gui_mainwindow_backward), metadata);

  g_signal_connect (G_OBJECT (document_view), "draw",
                    G_CALLBACK (gui_mainwindow_document_view_draw), NULL);

  g_signal_connect (G_OBJECT (bg_color_button), "color-set",
		    G_CALLBACK (gui_mainwindow_set_bg_color), NULL);

  g_signal_connect (G_OBJECT (pressure_input), "value-changed",
		    G_CALLBACK (gui_mainwindow_set_pressure_input), NULL);

  g_signal_connect (G_OBJECT (zoom_input), "value-changed",
		    G_CALLBACK (gui_mainwindow_set_zoom_input), NULL);

  g_signal_connect (G_OBJECT (dimensions_input), "changed",
		    G_CALLBACK (gui_mainwindow_set_dimensions_input), NULL);

  g_signal_connect (G_OBJECT (orientation_input), "changed",
		    G_CALLBACK (gui_mainwindow_set_orientation_input), NULL);

  g_signal_connect (G_OBJECT (clock_scale), "value-changed",
		    G_CALLBACK (gui_mainwindow_set_clock_value), NULL);

  g_signal_connect (G_OBJECT (zoom_toggle), "notify::active",
		    G_CALLBACK (gui_mainwindow_set_zoom_toggle), NULL);

  g_signal_connect (G_OBJECT (pressure_toggle), "notify::active",
		    G_CALLBACK (gui_mainwindow_set_pressure_toggle), NULL);

  g_signal_connect (G_OBJECT (open_button), "clicked",
		    G_CALLBACK (gui_mainwindow_menu_file_activate), (void*)FILE_OPEN);

  g_signal_connect (G_OBJECT (export_button), "clicked",
		    G_CALLBACK (gui_mainwindow_menu_file_activate), (void*)FILE_EXPORT);

  g_signal_connect (G_OBJECT (save_config_button), "clicked",
		    G_CALLBACK (gui_mainwindow_save_settings), NULL);

  /*--------------------------------------------------------------------------.
   | DISPLAY                                                                  |
   '--------------------------------------------------------------------------*/

  gtk_widget_show_all (window);
  gtk_widget_hide (zoom_input);
  gtk_widget_set_sensitive (GTK_WIDGET (export_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (timing_button), FALSE);
  
  if (filename)
    gui_mainwindow_file_activated (window, (char*)filename);

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
  FileActions action = (FileActions)data;
  switch (action)
    {
      case FILE_OPEN:
	gui_mainwindow_file_activated (widget, NULL);
	break;
      case FILE_EXPORT:
	gui_mainwindow_export_activated (widget);
	break;
    }
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

      if (last_dir)
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), last_dir);

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

      if (last_dir)
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), last_dir);

      /* Add filters for supported formats. */
      int a = 0;
      for (; a < (int)(sizeof (file_mimetypes) / sizeof (char*)); a++)
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

      last_dir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog));
      last_file_extension = g_ascii_strdown (filter_name, 4);
      if (g_ascii_isspace (last_file_extension[3])) last_file_extension[3] = '\0';
    }
  gtk_widget_destroy (dialog);

  return filename;
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_UPDATE_CLOCK                                                |
 | This function applies the next step in time for the "play" feature.        |
 '----------------------------------------------------------------------------*/
gboolean
gui_mainwindow_update_clock ()
{
  double value = gtk_range_get_value (GTK_RANGE (clock_scale));
  gtk_range_set_value (GTK_RANGE (clock_scale), value + 1);
  if (value >= settings.process_until)
    g_source_remove (timeout_id),
    timeout_id = 0;

  return TRUE;
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_PLAY                                                        |
 | This callback function handles activating the "Play" button.               |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_play ()
{
  if (timeout_id != 0)
    g_source_remove (timeout_id),
    timeout_id = 0;

  GtkWidget *icon;
  
  if (!is_playing)
    {
      timeout_id = g_timeout_add (100, gui_mainwindow_update_clock, NULL);

      icon = gtk_image_new_from_icon_name ("media-playback-pause",
					   GTK_ICON_SIZE_LARGE_TOOLBAR);

      gtk_button_set_image (GTK_BUTTON (play_button), icon);
      is_playing = TRUE;
    }
  else
    {
      icon = gtk_image_new_from_icon_name ("media-playback-start",
					   GTK_ICON_SIZE_LARGE_TOOLBAR);

      gtk_button_set_image (GTK_BUTTON (play_button), icon);      
      is_playing = FALSE;
    }
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_FORWARD                                                     |
 | This callback function handles activating the "Forward" button.            |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_forward ()
{
  if (metadata == NULL) return;

  metadata->layer_timings = g_slist_reverse (metadata->layer_timings);
  GSList* timings = metadata->layer_timings;
  while (timings != NULL)
    {
      double timings_value = (double)*(int *)timings->data;
      if (timings_value > gtk_range_get_value (GTK_RANGE (clock_scale))) break;
      timings = timings->next;
    }

  if (timings != NULL)
    gtk_range_set_value (GTK_RANGE (clock_scale), *(int *)timings->data);
  else
    gtk_range_set_value (GTK_RANGE (clock_scale), metadata->num_seconds);

  metadata->layer_timings = g_slist_reverse (metadata->layer_timings);
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_BACKWARD                                                    |
 | This callback function handles activating the "Backward" button.           |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_backward ()
{
  if (metadata == NULL) return;
  GSList* timings = metadata->layer_timings;
  while (timings != NULL)
    {
      double timings_value = (double)*(int *)timings->data;
      if (timings_value < gtk_range_get_value (GTK_RANGE (clock_scale))) break;
      timings = timings->next;
    }

  if (timings != NULL)
    gtk_range_set_value (GTK_RANGE (clock_scale), *(int *)timings->data);
  else
    gtk_range_set_value (GTK_RANGE (clock_scale), 0);
}


/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_FILE_ACTIVATED                                              |
 | This callback function handles activating the "Open" menu button.          |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_file_activated (GtkWidget* widget, void* data)
{
  char* filename = NULL;
  GtkWidget *parent = gtk_widget_get_toplevel (widget);

  /* The filename can be passed by 'data'. Otherwise we need to show a
   * dialog to the user to choose a file. */
  if (data)
    filename = (char*)data;
  else
    filename = gui_mainwindow_file_dialog (parent, GTK_FILE_CHOOSER_ACTION_OPEN);

  if (filename != NULL)
    {
      char* window_title = malloc (16 + strlen (filename) + 1);
      sprintf (window_title, "InklingReader: %s", filename);
      gtk_header_bar_set_title (GTK_HEADER_BAR (header), window_title);

      free (window_title);

      /* Clean-up the old parsed data. */
      if (parsed_data)
	{
	  p_wpi_cleanup (parsed_data), parsed_data = NULL;
	  p_wpi_metadata_cleanup (metadata), metadata = NULL;
	}
	  
      parsed_data = p_wpi_parse (filename, &settings.process_until);
      gtk_scale_clear_marks (GTK_SCALE (clock_scale));
      gtk_range_set_range (GTK_RANGE (clock_scale), 0, settings.process_until);
      gtk_range_set_value (GTK_RANGE (clock_scale), settings.process_until);

      metadata = p_wpi_get_metadata (parsed_data);
      if (metadata != NULL)
	{
	  if (metadata->num_layers > 1)
	    gtk_scale_add_mark (GTK_SCALE (clock_scale), 0, GTK_POS_BOTTOM, "Layer 1");
	      
	  short layer_number = metadata->num_layers;
	  GSList* timings = metadata->layer_timings;
	  while (timings != NULL)
	    {
	      char layername[9];
	      memset (&layername, '\0', 9);
	      snprintf ((char *)layername, 9, "Layer %d", layer_number);
	      gtk_scale_add_mark (GTK_SCALE (clock_scale), *(int *)timings->data, GTK_POS_BOTTOM, layername);
	      timings = timings->next;
	      layer_number--;
	    }
	}

      gtk_widget_show_all (hbox_timing);
      gtk_widget_set_sensitive (GTK_WIDGET (export_button), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (timing_button), TRUE);

      /* Clean up the filename if it was gathered using the dialog. */
      if (!data)
	g_free (filename);

      gui_mainwindow_redisplay ();
    }
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_EXPORT_ACTIVATED                                            |
 | This callback function handles activating the "Export" menu button.        |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_export_activated (GtkWidget* widget)
{
  GtkWidget *parent = gtk_widget_get_toplevel (widget);
  char* filename = gui_mainwindow_file_dialog (parent, GTK_FILE_CHOOSER_ACTION_SAVE);

  if (last_file_extension && filename)
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

  else if (!strcmp (ext, ".csv"))
    co_csv_create_file (filename, parsed_data);

  else if (!strcmp (ext, ".svg"))
    high_export_to_file (parsed_data, NULL, filename, &settings);

  g_free (filename);
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_DOCUMENT_VIEW_DRAW                                          |
 | This callback function handles the drawing on the 'document_view' widget.  |
 '----------------------------------------------------------------------------*/
gboolean
gui_mainwindow_document_view_draw (GtkWidget *widget, cairo_t *cr)
{
  if (parsed_data == NULL && handle == NULL) return 0;

  GtkWidget *parent = gtk_widget_get_toplevel (widget);

  double w = gtk_widget_get_allocated_width (parent);
  double ratio = 1.00;
  
  if (!gtk_widget_get_visible (zoom_input))
    ratio = w / (settings.page.width * PT_TO_MM * 1.25) / 1.15;
  else
    ratio = gtk_spin_button_get_value (GTK_SPIN_BUTTON (zoom_input)) / 100.0,
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (zoom_input), ratio * 100);

  double padding = ((w - (settings.page.width * PT_TO_MM * 1.25 * ratio)) / 2) - 30;
  if (padding < 0) padding = 0;
  double h = settings.page.height * PT_TO_MM * 1.25 * ratio + padding * 2;
  w = settings.page.width * PT_TO_MM * 1.25 * ratio + padding;

  gtk_widget_set_size_request (widget, w, h);

  cairo_translate (cr, padding, padding);
  cairo_scale (cr, ratio, ratio);

  if (handle)
    rsvg_handle_render_cairo (handle, cr),
    rsvg_handle_close (handle, NULL);
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
 | GUI_MAINWINDOW_SAVE_SETTINGS                                               |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_save_settings ()
{
  settings.page.orientation = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (orientation_input));
  dt_configuration_store_settings (settings.config_location, &settings);
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_ADD_COLOR                                                   |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_add_color (GtkWidget* widget)
{
  GtkWidget *parent = gtk_widget_get_toplevel (widget);
  GtkWidget* color_chooser;

  /* Show the color selection dialog. */
  color_chooser = gtk_color_chooser_dialog_new ("Choose a color", GTK_WINDOW (parent));
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

      gtk_box_pack_start (GTK_BOX (hbox_color_buttons), color, 0, 0, 0);
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
gui_mainwindow_set_bg_color (GtkWidget* widget)
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
gui_mainwindow_set_pressure_input (GtkWidget* widget)
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
gui_mainwindow_set_zoom_input ()
{
  gui_mainwindow_redisplay();
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_ZOOM_TOGGLE                                             |
 | This function is the callback for enabling or disabling the zoom factor.   |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_zoom_toggle (GtkWidget* widget)
{
  if (gtk_switch_get_active (GTK_SWITCH (widget)))
    {
      GtkWidget *parent = gtk_widget_get_toplevel (widget);
      double w = gtk_widget_get_allocated_width (parent);
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
gui_mainwindow_set_pressure_toggle (GtkWidget* widget)
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
 | GUI_MAINWINDOW_SET_DIMENSIONS_INPUT                                        |
 | This callback is for changing the page dimensions.                         |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_dimensions_input (GtkWidget* widget)
{
  char* dimensions = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (widget));
  if (dimensions == NULL) return;

  gtk_combo_box_set_active (GTK_COMBO_BOX (orientation_input), 0);
  dt_configuration_parse_preset_dimensions (dimensions, &settings);
  gui_mainwindow_redisplay();  
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_ORIENTATION_INPUT                                       |
 | This callback is for changing the page orientation.                        |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_orientation_input ()
{
  /* Since this callback is only called when the orientation changes, 
   * the width and height always swap. */
  double width = settings.page.width;
  settings.page.width = settings.page.height;
  settings.page.height = width;

  gui_mainwindow_redisplay();  
}

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_SET_CLOCK_VALUE                                             |
 | This callback is for setting the clock range to process.                   |
 '----------------------------------------------------------------------------*/
void
gui_mainwindow_set_clock_value (GtkWidget* widget)
{
  settings.process_until = (unsigned short)gtk_range_get_value (GTK_RANGE (widget));

  if (handle != NULL)
    g_object_unref (handle), handle = NULL;

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
