#include "../include/auto_complete.h"

#define MAX_KEYWORDS 100
#define MAX_KEYWORD_LENGTH 50

static char *keywords[MAX_KEYWORDS];
static int num_keywords = 0;

TrieNode* create_trie_node() {
    TrieNode *node = (TrieNode*)malloc(sizeof(TrieNode));
    node->is_end_of_word = false;
    for (int i = 0; i < ALPHABET_SIZE + 1; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void insert_keyword(TrieNode *root, const char *keyword) {
    TrieNode *node = root;
    for (int i = 0; keyword[i] != '\0'; i++) {
        int index = keyword[i] == '.' ? 26 : keyword[i] - 'a';
        if (!node->children[index]) {
            node->children[index] = create_trie_node();
        }
        node = node->children[index];
    }
    node->is_end_of_word = true;
}

void free_trie_node(TrieNode *node) {
    if (node != NULL) {
        for (int i = 0; i < ALPHABET_SIZE + 1; i++) {
            free_trie_node(node->children[i]);
        }
        free(node); 
    }
}

void get_suggestions_from_node(TrieNode *node, char *prefix, int prefix_len, GtkListStore *store) {
    if (node->is_end_of_word) {
        prefix[prefix_len] = '\0';
        GtkTreeIter treeIter;
        gtk_list_store_append(store, &treeIter);
        gtk_list_store_set(store, &treeIter, 0, prefix, -1);
    }

    for (int i = 0; i < ALPHABET_SIZE + 1; i++) {
        if (node->children[i]) {
            prefix[prefix_len] = i == 26 ? '.' : 'a' + i;
            get_suggestions_from_node(node->children[i], prefix, prefix_len + 1, store);
        }
    }
}

void get_suggestions(TrieNode *root, const char *prefix, GtkListStore *store) {
    TrieNode *node = root;
    int prefix_len = 0;
    for (int i = 0; prefix[i] != '\0'; i++) {
        int index = prefix[i] == '.' ? 26 : prefix[i] - 'a';
        if (!(index <= 26 && index >= 0) || !node->children[index]) {
            return;
        }
        node = node->children[index];
        prefix_len++;
    }
    char buffer[MAX_KEYWORD_LENGTH];
    strncpy(buffer, prefix, prefix_len);
    get_suggestions_from_node(node, buffer, prefix_len, store);
}

void show_autocompletion(AutoCompleteData *acData, gchar *word, GtkTextIter start) {

    gtk_list_store_clear(acData->store);

    // Get suggestions from the trie
    get_suggestions(acData->trieRoot, word, acData->store);

    int number_of_rows = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(acData->store), NULL);
    if (number_of_rows == 0 || number_of_rows == 38) {
        // 38 indicates no characters
        if (gtk_widget_get_visible(acData->popup))
            gtk_widget_hide(acData->popup);
        return;
    }

    gtk_window_resize(GTK_WINDOW(acData->popup), 200, number_of_rows * 28);

    GdkRectangle strong, weak;
    // gtk_text_view_get_iter_location(acData->textView, start, &location);
    gtk_text_view_get_cursor_locations(acData->textView, &start, &strong, &weak);
    int wx, wy;
    gtk_text_view_buffer_to_window_coords((acData->textView), GTK_TEXT_WINDOW_WIDGET, strong.x, strong.y, &wx, &wy);
    
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(acData->textView)));
    GdkWindow *win = gtk_widget_get_window(GTK_WIDGET(window));
    int x, y;
    GtkAllocation allocation;
    gtk_widget_get_allocation(acData->data->listbox, &allocation);
    gint tab_width = allocation.width; 
    gdk_window_get_origin(win, &x, &y);
    gtk_window_move(GTK_WINDOW(acData->popup), wx + x + tab_width, wy + y + 100); // Adjust the popup position as needed
    gtk_widget_show_all(acData->popup);
}

void move_selection(AutoCompleteData *acData, bool upTdF) {
    int number_of_rows = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(acData->store), NULL);
    GtkTreeView* treeView = acData->treeView;
    GtkTreePath* path = NULL;
    GtkTreeViewColumn* focus_column = NULL;
    gtk_tree_view_get_cursor(treeView, &path, &focus_column);
    if (path == NULL) {
        path = gtk_tree_path_new_first();
    }
    else if (gtk_tree_path_compare(path, gtk_tree_path_new_first()) == 0 && upTdF) {
        path = gtk_tree_path_new_from_indices(number_of_rows - 1, -1);
    }  
    else if (gtk_tree_path_compare(path, gtk_tree_path_new_from_indices(number_of_rows - 1, -1)) == 0 && !upTdF) {
        path = gtk_tree_path_new_first();
    }
    else {
        if (upTdF)
            gtk_tree_path_prev(path);
        else 
            gtk_tree_path_next(path);
    }
    
    gtk_tree_view_set_cursor(treeView, path, NULL, FALSE);
    gtk_tree_path_free(path);
    return;
}

void on_row_activated(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    AutoCompleteData *acData = (AutoCompleteData *)user_data;
    GtkTreeModel *model = gtk_tree_view_get_model(treeView);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gchar *value;
        gtk_tree_model_get(model, &iter, 0, &value, -1);

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(acData->textView);
        GtkTextIter cursor_iter, word_start, word_end;

        // Get the current position of the cursor
        gtk_text_buffer_get_iter_at_mark(buffer, &cursor_iter, gtk_text_buffer_get_insert(buffer));

        // Initialize word_start and word_end to the current cursor position
        word_start = word_end = cursor_iter;

        // Move word_start backwards to the beginning of the word
        // Move word_start backwards to the beginning of the word
        while (gtk_text_iter_backward_char(&word_start)) {
            gunichar ch = gtk_text_iter_get_char(&word_start);
            if (ch == ' ' || ch == '\t') {
                gtk_text_iter_forward_char(&word_start);
                break;
            }
        }

        // Move word_end forward to the end of the word within the same line
        while (!gtk_text_iter_is_end(&word_end) && !gtk_text_iter_ends_line(&word_end)) {
            gunichar ch = gtk_text_iter_get_char(&word_end);
            if (ch == ' ' || ch == '\t') {
                break;
            }
            gtk_text_iter_forward_char(&word_end);
        }
            

        // Delete the text between word_start and word_end
        gtk_text_buffer_delete(buffer, &word_start, &word_end);

        // Insert the new value at the position of word_start
        gtk_text_buffer_insert(buffer, &word_start, value, -1);

        g_free(value);
        if (gtk_widget_get_visible(acData->popup)) {
            gtk_widget_hide(acData->popup);
        }
    }
}

AutoCompleteData* create_auto_complete_data(GtkTextView *textView, CallbackData *data) {
    AutoCompleteData *acData = g_new0(AutoCompleteData, 1);
    acData->textView = textView;
    acData->data = data;

    acData->popup = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_decorated(GTK_WINDOW(acData->popup), FALSE);
    gtk_widget_set_size_request(acData->popup, 200, 0);

    acData->store = gtk_list_store_new(1, G_TYPE_STRING);
    acData->treeView = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(acData->store)));
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Suggestions", renderer, "text", 0, NULL);
    gtk_tree_view_set_headers_visible(acData->treeView, FALSE); // disabled header for UI purposes
    gtk_tree_view_append_column(GTK_TREE_VIEW(acData->treeView), column);
    gtk_container_add(GTK_CONTAINER(acData->popup), GTK_WIDGET(acData->treeView));

    g_signal_connect(acData->treeView, "row-activated", G_CALLBACK(on_row_activated), acData);

    acData->trieRoot = create_trie_node();
    
    FILE *file = fopen("words.txt", "r");
    if (file != NULL) {
        char keyword[256];
        while (fscanf(file, "%s", keyword) != EOF) {
            insert_keyword(acData->trieRoot, keyword);
        }
        fclose(file);
    } else {
        g_warning("Failed to open file: %s", "words.txt");
    }

    return acData;
}

void free_acData(GtkWidget *widget, gpointer data) {
     AutoCompleteData *acData = (AutoCompleteData *)data;
     if (acData->popup) {
        gtk_widget_destroy(acData->popup);
     }
     if (acData->store) {
        g_object_unref(acData->store);
     }
     free_trie_node(acData->trieRoot);
     g_free(acData);
}