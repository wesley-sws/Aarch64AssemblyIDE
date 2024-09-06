#include <gtk/gtk.h>
#include <glib.h>

#ifndef E_STRUCT

typedef struct {
    GtkTextView *textView;
    guint timer_id;
    GHashTable *error_messages; // Add this field to store error messages
    GtkWidget *error_popup; // Add this field for the error popup
    GtkTextBuffer *buffer;
} ErrorCheckData;

ErrorCheckData* create_err_data(GtkTextView* textView, GtkTextBuffer* buffer);
void free_errData(GtkWidget *widget, gpointer data);
gboolean check_errors(gpointer user_data);

#endif


