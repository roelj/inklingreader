#ifndef GUI_MAINWINDOW_SIG_H
#define GUI_MAINWINDOW_SIG_H

#include <gtk/gtk.h>

char* gui_mainwindow_file_dialog (GtkWidget* parent, GtkFileChooserAction action);
void gui_mainwindow_file_activated (GtkWidget* widget, void* data);
void gui_mainwindow_directory_activated (GtkWidget* widget, void* data);
gboolean gui_mainwindow_document_view_draw (GtkWidget *widget, cairo_t *cr, void* data);

#endif//GUI_MAINWINDOW_SIG_H
