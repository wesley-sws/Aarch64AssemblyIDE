#include <gtk/gtk.h>
#include "auto_complete.h"

void apply_css(const gchar *css_file, GtkCssProvider *provider);
void apply_light_theme(GtkWidget *widget, gpointer data);
void apply_dark_theme(GtkWidget *widget, gpointer data);
void apply_catppuccin_theme(GtkWidget *widget, gpointer data);
