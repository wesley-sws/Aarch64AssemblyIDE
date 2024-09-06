#include "../include/events.h"
#include "../include/menu.h"
#include "../include/tabs.h"


gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {

    AutoCompleteData *acData = (AutoCompleteData *)user_data;
    

    gboolean is_c;
    #ifdef __APPLE__
        // macOS
        is_c = (event->state & GDK_META_MASK) || (event->state & GDK_MOD2_MASK); // Command key
    #else
        is_c = event->state & GDK_CONTROL_MASK; // Control key
    #endif
    if (event->keyval == GDK_KEY_Escape) {
        if (gtk_widget_get_visible(acData->popup)) { 
            gtk_widget_hide(acData->popup);
        }
        return TRUE;
    }
    if (is_c) {
        if (gtk_widget_get_visible(acData->popup)) { 
            gtk_widget_hide(acData->popup);
        }
        GtkWidget *notebook = acData->data->notebook;
        switch(event->keyval) {
            case GDK_KEY_1: 
                go_to_tab(notebook, 1);
                return TRUE; 
            case GDK_KEY_2: 
                go_to_tab(notebook, 2);
                return TRUE; 
            case GDK_KEY_3: 
                go_to_tab(notebook, 3);
                return TRUE; 
            case GDK_KEY_4:
                go_to_tab(notebook, 4);
                return TRUE;  
            case GDK_KEY_5: 
                go_to_tab(notebook, 5);
                return TRUE; 
            case GDK_KEY_6: 
                go_to_tab(notebook, 6);
                return TRUE; 
            case GDK_KEY_7: 
                go_to_tab(notebook, 7);
                return TRUE; 
            case GDK_KEY_8: 
                go_to_tab(notebook, 8);
                return TRUE; 
            case GDK_KEY_9: 
                go_to_tab(notebook, 9);
                return TRUE; 
            case 's':
                on_save_file(NULL, acData->data->notebook);
                return TRUE;
            case 'w':
                delete_current_tab(NULL, acData->data);
                return TRUE;
            case 't':
                add_untitled_tab(NULL, acData->data);
                return TRUE; 
            case 'o':
                on_open_file(NULL, acData->data);
                return TRUE; 
        }
    }
    if (gtk_widget_get_visible(acData->popup)) {
        switch (event->keyval) {
            case GDK_KEY_Up: {
                move_selection(acData, TRUE);
                return TRUE;
            }
            case GDK_KEY_Down:
                move_selection(acData, FALSE);
                return TRUE;
            case GDK_KEY_Return: {
                GtkTreePath *path = NULL;
                GtkTreeViewColumn *focus_column = NULL;
                gtk_tree_view_get_cursor(acData->treeView, &path, &focus_column);
                if (path != NULL) {
                    on_row_activated(acData->treeView, path, focus_column, acData);
                    gtk_tree_path_free(path);
                    return TRUE;
                }
            }

        }
    }
    // if it gets to here we will show autocomplete
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(acData->textView);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

    // Get the current word being typed
    GtkTextIter start = iter;
    while (gtk_text_iter_backward_char(&start)) {
        if (gtk_text_iter_get_char(&start) == ' ' || gtk_text_iter_get_char(&start) == '\n') {
            gtk_text_iter_forward_char(&start);
            break;
        }
    }
    gchar *word = gtk_text_buffer_get_text(buffer, &start, &iter, FALSE);
    if (!word) {
        return FALSE; // Ensure word is not NULL
    }
    guint keyval = event->keyval;
    gunichar typed_char = gdk_keyval_to_unicode(keyval);

    if (keyval == GDK_KEY_Down || keyval == GDK_KEY_Up) {
        g_free(word);
        if (gtk_widget_get_visible(acData->popup))
            gtk_widget_hide(acData->popup);
        return FALSE;
    }
    gchar *new_word;
    GtkTextIter next_iter = iter;
    if (keyval == GDK_KEY_Right) {
        gchar next_char = gtk_text_iter_get_char(&iter);            
        new_word = g_strdup_printf("%s%c", word, next_char);
    }
    else {
        new_word = (keyval == GDK_KEY_BackSpace || keyval == GDK_KEY_Left) && strlen(word) > 1 ? 
        g_strndup(word, strlen(word) - 1) : g_strdup_printf("%s%c", word, typed_char);
    }
    assert(new_word != NULL);

    show_autocompletion(acData, new_word, start);
    g_free(new_word);
    g_free(word);
    return FALSE;

}

gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    ErrorCheckData *errorData = (ErrorCheckData *)user_data;
    // Reset the timer if it's already running

    if (errorData->timer_id != 0) {
        g_source_remove(errorData->timer_id);
    }

    // Start a new timer to check for errors after 0.5 second (500 milliseconds)
    errorData->timer_id = g_timeout_add(500, check_errors, errorData);

    return FALSE;
}

gboolean on_mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    ErrorCheckData *errorData = (ErrorCheckData *)user_data;
    GtkTextView *text_view = GTK_TEXT_VIEW(widget);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);

    // Get the text iterator at the mouse position
    GtkTextIter iter;
    int buffer_x, buffer_y;
    gtk_text_view_window_to_buffer_coords(text_view, GTK_TEXT_WINDOW_WIDGET,
                                          (int) event->x, (int) event->y,
                                          &buffer_x, &buffer_y);
    gboolean location_valid = gtk_text_view_get_iter_at_location(text_view, &iter, buffer_x, buffer_y);
    gint line_number = gtk_text_iter_get_line(&iter);

    // Get the iterator to the start and end of the current line
    GtkTextIter line_start_iter, line_end_iter;
    gtk_text_buffer_get_iter_at_line(buffer, &line_start_iter, line_number);
    gtk_text_buffer_get_iter_at_line(buffer, &line_end_iter, line_number);
    gtk_text_iter_forward_to_line_end(&line_end_iter);

    GtkTextIter first_non_space_iter = line_start_iter;
    while (gtk_text_iter_compare(&first_non_space_iter, &line_end_iter) < 0 && g_unichar_isspace(gtk_text_iter_get_char(&first_non_space_iter))) {
        gtk_text_iter_forward_char(&first_non_space_iter);
    }


    // Get the coordinates of the start and end of the line
    GdkRectangle start_rect, end_rect;
    gtk_text_view_get_iter_location(text_view, &first_non_space_iter, &start_rect);
    gtk_text_view_get_iter_location(text_view, &line_end_iter, &end_rect);

    // Convert buffer coordinates to window coordinates
    int start_x, start_y, end_x, end_y;

    gtk_text_view_buffer_to_window_coords(text_view, GTK_TEXT_WINDOW_WIDGET, start_rect.x, start_rect.y, &start_x, &start_y);
    gtk_text_view_buffer_to_window_coords(text_view, GTK_TEXT_WINDOW_WIDGET, end_rect.x, end_rect.y + end_rect.height, &end_x, &end_y);
    start_x -= 50;
    end_x -= 50;
    // Check if the mouse is within the bounds of the line's text area
    if (buffer_x >= start_x && buffer_x <= end_x && buffer_y >= start_y && buffer_y <= end_y) {
        
        // Check if there is an error message for this line
        gchar *error_message = g_hash_table_lookup(errorData->error_messages, GINT_TO_POINTER(line_number));
        if (error_message != NULL) {
            // Show the error popup with the message
            gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(errorData->error_popup))), error_message);
            gtk_widget_show_all(errorData->error_popup);

            // Position the popup near the mouse cursor
            gtk_window_move(GTK_WINDOW(errorData->error_popup), (int) event->x_root + 10, (int) event->y_root + 10); // Adjust the popup position as needed
        } else if (gtk_widget_get_visible(errorData->error_popup)) 
            gtk_widget_hide(errorData->error_popup);
        
    } else if (gtk_widget_get_visible(errorData->error_popup)) {
        // Hide the popup if the mouse is not within the bounds of the current line's text
        gtk_widget_hide(errorData->error_popup);
    }

    return FALSE;
}

gboolean on_leave_notify_ERR(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
    ErrorCheckData *errorData = (ErrorCheckData *)user_data;
    if (gtk_widget_get_visible(errorData->error_popup))
        gtk_widget_hide(errorData->error_popup);
    return FALSE; // Return FALSE to propagate the event further.
}


gboolean on_leave_notify_AC(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
    AutoCompleteData *acData = (AutoCompleteData *)user_data;
    if (gtk_widget_get_visible(acData->popup))
        gtk_widget_hide(acData->popup);
    return FALSE; // Return FALSE to propagate the event further.
}