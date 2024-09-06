#include "../include/tabs.h"
#include "../include/events.h"
#include "../include/menu.h"
#include "../include/auto_complete.h"

// Returns the last occurence 
static char *strrstr(const char *haystack, const char *needle) {
    if (!*needle) {
        // If needle is empty, return haystack (standard behavior)
        return (char *)haystack;
    }

    char *result = NULL;
    char *current = strstr(haystack, needle);

    while (current) {
        result = current;
        current = strstr(current + 1, needle);
    }

    return result;
}

// Ensures equal tab size 
static void set_tab_size(GtkWidget* notebook) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(notebook, &allocation);
    gint pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    if (pages == 0) {
        return; 
    }
    gint tab_width = allocation.width / pages; 
    for (int i = 0; i < pages; i++) {
        GtkWidget *tab_label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), i));
        gtk_widget_set_size_request(tab_label, tab_width, -1);
    } 
}

void go_to_tab(GtkWidget *notebook, gint page_no) {
    gint num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)); 
    if (page_no == 9) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), num_pages - 1); 
    } else if (page_no <= num_pages) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page_no - 1); 
    }
}

// Add untitled tab 
void add_untitled_tab(GtkWidget *widget, gpointer data) {
    static int number = 1; 
    char filename[50];
    snprintf(filename, sizeof(filename), "/Untitled %d.s", number);
    filename[strlen(filename)] = '\0';
    add_new_tab(data, filename, "");
    number++; 
}

// Function to delete tab 
void delete_current_tab(GtkWidget *widget, gpointer data) {
    CallbackData *cb_data = (CallbackData *)data;
    GtkNotebook *notebook = GTK_NOTEBOOK(cb_data->notebook);
    GtkListBox *listbox = GTK_LIST_BOX(cb_data->listbox);
    gint current_page_no = gtk_notebook_get_current_page(notebook);
    gint page_num = gtk_notebook_get_n_pages(notebook); 
    if (page_num > 1) {
        if (current_page_no != -1) {
            gtk_notebook_remove_page(notebook, current_page_no);
            GtkListBoxRow *row = gtk_list_box_get_row_at_index(listbox, current_page_no);
            gtk_widget_destroy(GTK_WIDGET(row));
            set_tab_size(GTK_WIDGET(notebook));
        }
    }
}

// Changing sidebar tab when notebook tab is changed
void change_sidebar(GtkNotebook *notebook, GtkWidget *widget, guint page_num, gpointer data) {
    GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(data), page_num);
    gtk_list_box_select_row(GTK_LIST_BOX(data), row);
}

// Changing notebook tab when sidebar row clicked
static void row_clicked(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    gint index = gtk_list_box_row_get_index(row);

    // Set notebook current page based on row index
    GtkWidget *notebook = GTK_WIDGET(user_data);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), index);
}

// Function to add a new tab
void add_new_tab(gpointer data, const char *filename, const char *content) {
    CallbackData *cb_data = (CallbackData *)data; 
    GtkWidget *notebook = cb_data->notebook; 
    GtkWidget *listbox = cb_data->listbox; 
    // Creating scrolled window, source view and buffer 
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    GtkWidget *newSourceView = gtk_source_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), newSourceView);
    gtk_widget_show(newSourceView);
    gtk_widget_show(scrolled_window);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(newSourceView), TRUE);
    gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(newSourceView), TRUE);
    GtkTextView *textView = GTK_TEXT_VIEW(newSourceView);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(textView);

    // Setting content into buffer 
    gtk_text_buffer_set_text(buffer, content, -1);

    // Sets language to buffer 
    GtkSourceLanguageManager *lm = gtk_source_language_manager_get_default();
    strcpy(gtk_source_language_manager_get_search_path(lm)[0], g_get_current_dir());
    GtkSourceLanguage *lang = gtk_source_language_manager_get_language(lm, "aarch");
    gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer), lang);

    // Adds styling to our IDE
    GtkSourceStyleSchemeManager *sm = gtk_source_style_scheme_manager_get_default();
    gtk_source_style_scheme_manager_append_search_path(sm, g_get_current_dir());
    GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(sm, "catppuccin");
    gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(buffer), scheme);

    // Extract the correct file name 
    char *needle = "/";
    char *filename_pointer = strrstr(filename, needle); 
    // Adding a new tab 
    GtkWidget *file_label = gtk_label_new(filename_pointer + strlen(needle));
    gtk_widget_set_hexpand(file_label, TRUE);
    gtk_widget_set_halign(file_label, GTK_ALIGN_FILL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, file_label);
    set_tab_size(notebook);
    gint n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), n_pages - 1);
    
    AutoCompleteData *acData = create_auto_complete_data(textView, cb_data);
    g_signal_connect(textView, "key-press-event", G_CALLBACK(on_key_press), acData);
    ErrorCheckData* errData = create_err_data(textView, buffer);
    g_signal_connect(textView, "destroy", G_CALLBACK(free_acData), acData);
    g_signal_connect(textView, "destroy", G_CALLBACK(free_errData), errData);
    g_signal_connect(textView, "key-release-event", G_CALLBACK(on_key_release), errData);
    g_signal_connect(textView, "motion-notify-event", G_CALLBACK(on_mouse_motion), errData);
    gtk_widget_add_events(GTK_WIDGET(newSourceView), GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(GTK_WIDGET(newSourceView), GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(G_OBJECT(newSourceView), "leave-notify-event", G_CALLBACK(on_leave_notify_ERR), errData);

    // Add tab label to the sidebar listbox
    GtkWidget *listbox_row = gtk_list_box_row_new();
    GtkWidget *tab_label = gtk_label_new(filename_pointer + strlen(needle));
    gtk_container_add(GTK_CONTAINER(listbox_row), tab_label);
    g_signal_connect(listbox, "row-activated", G_CALLBACK(row_clicked), notebook);
    gtk_list_box_insert(GTK_LIST_BOX(listbox), listbox_row, -1);
    gtk_list_box_select_row(GTK_LIST_BOX(listbox), GTK_LIST_BOX_ROW(listbox_row));

    // gtk_widget_grab_focus(newSourceView);
    gtk_widget_show_all(notebook);
    gtk_widget_show_all(listbox);
}