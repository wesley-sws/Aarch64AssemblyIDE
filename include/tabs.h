#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

void delete_current_tab(GtkWidget *widget, gpointer data);
gboolean on_tab_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
void change_sidebar(GtkNotebook *notebook, GtkWidget *widget, guint page_num, gpointer data);
static void row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data);
void add_new_tab(gpointer data, const char *filename, const char *content);
void add_untitled_tab(GtkWidget *widget, gpointer data);
void go_to_tab(GtkWidget *notebook, gint page_no);