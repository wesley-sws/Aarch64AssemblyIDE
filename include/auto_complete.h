#ifndef AUTO_COMPLETE_H
#define AUTO_COMPLETE_H

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "menu.h"

#define MAX_KEYWORD_LENGTH 50
#define ALPHABET_SIZE 26

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE + 1];
    bool is_end_of_word;
} TrieNode;

TrieNode* create_trie_node();
void insert_keyword(TrieNode *root, const char *keyword);
void free_trie_node(TrieNode *root);
void get_suggestions_from_node(TrieNode *node, char *prefix, int prefix_len, GtkListStore *store);
void get_suggestions(TrieNode *root, const char *prefix, GtkListStore *store);

typedef struct {
    GtkTextView *textView;
    GtkWidget *popup;
    GtkListStore *store;
    GtkTreeView *treeView;
    TrieNode *trieRoot;
    CallbackData *data;
} AutoCompleteData;

void show_autocompletion(AutoCompleteData *acData, gchar *word, GtkTextIter start);
void move_selection(AutoCompleteData *acData, bool upTdF);
void on_row_activated(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data);
AutoCompleteData* create_auto_complete_data(GtkTextView *textView, CallbackData *data);
void free_acData(GtkWidget *widget, gpointer data);

#endif // AUTO_COMPLETE_H
