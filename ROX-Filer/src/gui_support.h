/*
 * $Id$
 *
 * ROX-Filer, filer for the ROX desktop project
 * By Thomas Leonard, <tal197@ecs.soton.ac.uk>.
 */

#ifndef _GUI_SUPPORT_H
#define _GUI_SUPPORT_H

#include <gtk/gtk.h>

#define WIN_STATE_STICKY          (1<<0) /* Fixed relative to screen */
#define WIN_STATE_HIDDEN          (1<<4) /* Not on taskbar but window visible */
#define WIN_STATE_FIXED_POSITION  (1<<8) /* Window is fixed in position even */
#define WIN_STATE_ARRANGE_IGNORE  (1<<9) /* Ignore for auto arranging */

void gui_support_init();
int get_choice(char *title,
	       char *message,
	       int number_of_buttons, ...);
void report_error(char *title, char *message);
void set_cardinal_property(GdkWindow *window, GdkAtom prop, guint32 value);
void make_panel_window(GdkWindow *window);
gint hide_dialog_event(GtkWidget *widget, GdkEvent *event, gpointer window);
void delayed_error(char *title, char *error);
gboolean load_file(char *pathname, char **data_out, long *length_out);

#endif /* _GUI_SUPPORT_H */
