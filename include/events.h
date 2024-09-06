#include <gtk/gtk.h>
#include "auto_complete.h"
#include "error_check.h"

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
gboolean on_leave_notify_ERR(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);
