#include "../include/apply_css.h"

void apply_css(const gchar *css_file, GtkCssProvider *provider) {
    GFile *file = g_file_new_for_path(css_file);
    GError *error = NULL;

    gtk_css_provider_load_from_file(provider, file, &error);
    g_object_unref(file);

    if (error != NULL) {
        g_printerr("Error loading CSS file: %s\n", error->message);
        g_clear_error(&error);
    }
}

void apply_light_theme(GtkWidget *widget, gpointer data) {
    apply_css("style_light.css", data);
}

void apply_dark_theme(GtkWidget *widget, gpointer data) {
    apply_css("style_dark.css", data);
}

void apply_catppuccin_theme(GtkWidget *widget, gpointer data) {
    apply_css("style_cat.css", data);
}
