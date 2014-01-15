#include "mainwindow_sig.h"
#include "../converters/cairo_svg.h"
#include "../parsers/wpi.h"

#include <malloc.h>
#include <string.h>

extern GtkWidget* lbl_status;
GSList* parsed_data = NULL;

/*----------------------------------------------------------------------------.
 | GUI_MAINWINDOW_FILE_DIALOG                                                 |
 | This helper function opens a file dialog and returns the filename after    |
 | closing it. When no file is chosen, NULL is returned.                      |
 '----------------------------------------------------------------------------*/
char*
gui_mainwindow_file_dialog (GtkWidget* parent, GtkFileChooserAction action)
{
  char *filename = NULL;
  GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File", 
    GTK_WINDOW (parent), action, "Cancel", GTK_RESPONSE_CANCEL, 
    "Open", GTK_RESPONSE_ACCEPT, NULL);

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
  GtkWidget *parent = gtk_widget_get_toplevel (widget);
  char* filename = gui_mainwindow_file_dialog (parent, GTK_FILE_CHOOSER_ACTION_OPEN);

  if (filename != NULL)
    {
      size_t status_len = 17 + strlen (filename);
      char* status = malloc (status_len + 1);
      status = memset (status, '\0', status_len + 1); 
      status = strcat (status, "Now displaying: ");
      status = strcat (status, filename);
      status[status_len] = '\0';

      gtk_label_set_text (GTK_LABEL (lbl_status), status);

      parsed_data = parse_wpi (filename);

      free (status);
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
    {
      gtk_label_set_text (GTK_LABEL (lbl_status), "You can now choose a file below.");
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
  if (parsed_data != NULL)
    co_display_cairo_surface (cr, parsed_data);

  return 0;
}
