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

/**
 * @file   gui/mainwindow.h
 * @brief  The main window of the graphical user interface.
 * @author Roel Janssen
 */

/**
 * @namespace gui::mainwindow
 * The graphical user interface's main window.
 * 
 * @note The prefix for this namespace is "gui_mainwindow_".
 */

#ifndef GUI_MAINWINDOW_H
#define GUI_MAINWINDOW_H

#include <gtk/gtk.h>

/**
 * The GUI functionality starts here.
 */
void gui_mainwindow_init (int argc, char** argv, const char* filename);

/**
 * This helper function opens a file dialog and returns the filename after
 * closing it. When no file is chosen, NULL is returned.
 */
char* gui_mainwindow_file_dialog (GtkWidget* parent, GtkFileChooserAction action);

/**
 * This callback function handles activating the "Open" menu button.
 */
void gui_mainwindow_file_activated (GtkWidget* widget, void* data);

/**
 * This callback function handles activating the "Export" menu button.
 */
void gui_mainwindow_export_activated (GtkWidget* widget);

/**
 * This event handler handles the activation of a menu item within the "File" menu.
 */
void gui_mainwindow_menu_file_activate (GtkWidget* widget, void* data);

/**
 * This callback function handles the drawing on the 'document_view' widget.
 */
gboolean gui_mainwindow_document_view_draw (GtkWidget* widget, cairo_t *cr);

/**
 * This callback function handles adding a color to the color list.
 */
void gui_mainwindow_add_color (GtkWidget* widget);

/**
 * This function is the callback for setting the background color.
 */
void gui_mainwindow_set_bg_color (GtkWidget* widget);

/**
 * This function is the callback for resetting a foreground color.
 */
void gui_mainwindow_set_fg_color (GtkWidget* widget, void* data);

/**
 * This function is the callback for setting the pressure factor.
 */
void gui_mainwindow_set_pressure_input (GtkWidget* widget);

/**
 * This function is the callback for setting the zoom factor.
 */
void gui_mainwindow_set_zoom_input ();

/**
 * This function is the callback for enabling or disabling the zoom factor.
 */
void gui_mainwindow_set_zoom_toggle (GtkWidget* widget);

/**
 * This function is the callback for enabling or disabling the pressure factor.
 */
void gui_mainwindow_set_pressure_toggle (GtkWidget* widget);

/**
 * This function is the callback for enabling or disabling the page dimensions.
 */
void gui_mainwindow_set_dimensions_input (GtkWidget* widget);

/**
 * This function is the callback for enabling or disabling the page dimensions.
 */
void gui_mainwindow_set_orientation_input ();

/**
 * This function is the callback for changing the "process_until" value based
 * on the clock.
 */
void gui_mainwindow_set_clock_value (GtkWidget* widget);

/**
 * This callback function handles activating the "Play" button.
 */
void gui_mainwindow_play ();

/**
 * This callback function handles activating the "Forward" button.
 */
void gui_mainwindow_forward ();

/**
 * This callback function handles activating the "Backward" button.
 */
void gui_mainwindow_backward ();

/**
 * This function applies the next step in time for the "play" feature.
 */
gboolean gui_mainwindow_update_clock ();

/**
 * Clean up when quitting.
 */
void gui_mainwindow_quit ();

#endif//GUI_MAINWINDOW_H
