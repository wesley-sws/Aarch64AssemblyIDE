#ifndef OPEN_SAVE_H
#define OPEN_SAVE_H
#include <gtk/gtk.h>
// #include "../emulate_lib/cpu.h"
// #include "../emulate_lib/memory.h"
// #include "../emulate_lib/instruction.h"
// #include "../assemble_lib/parse.h"
// #include "../assemble_lib/output.h"

typedef struct {
    GtkWidget *notebook;
    GtkWidget *listbox;
} CallbackData;

void on_open_file(GtkWidget *widget, gpointer notebook);
void on_save_file(GtkWidget *widget, gpointer data);
void on_build_exc(GtkWidget *widget, gpointer data);
void on_build_bin(GtkWidget *widget, gpointer data);

#endif