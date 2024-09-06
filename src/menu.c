#include "../include/menu.h"
#include "../include/auto_complete.h"
#include "../include/events.h"
#include "../include/tabs.h"
#include <gtksourceview/gtksource.h>

void on_open_file(GtkWidget *widget, gpointer data) {

    CallbackData *callback_data = (CallbackData *)data;

    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Open File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    while (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(chooser);

        // Check if the file has the correct extension
        if (g_str_has_suffix(filename, ".s")) {
            gchar *content;
            if (g_file_get_contents(filename, &content, NULL, NULL)) {
                add_new_tab(callback_data, filename, content);
                g_free(content);
                g_free(filename);
                break; // Exit the loop and close the file chooser
            } else {
                // Show an error dialog if the file could not be read
                GtkWidget *errorDialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                                GTK_MESSAGE_ERROR,
                                                                GTK_BUTTONS_OK,
                                                                "Could not read the file: %s", filename);
                gtk_dialog_run(GTK_DIALOG(errorDialog));
                gtk_widget_destroy(errorDialog);
            }
        } else {
            // Show an error dialog if the file is not an assembly file
            GtkWidget *errorDialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            "The selected file is not an assembly file (.s). Please choose a valid file.");
            gtk_dialog_run(GTK_DIALOG(errorDialog));
            gtk_widget_destroy(errorDialog);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static gchar *strip_spaces(const gchar *str) {
    GString *result = g_string_new(NULL);
    while (*str) {
        if (!(*str == ' ' || *str == '\t')) {
            g_string_append_c(result, *str);
        }
        str++;
    }
    return g_string_free(result, FALSE);
}

// static void load_binary_file(const char *filename, uint8_t *memory, size_t memory_size){
//   FILE *file = fopen(filename, "rb");
//   fseek(file, 0, SEEK_END);
//   long file_size = ftell(file);
//   rewind(file);

//   fread(memory, 1, file_size, file);
//   fclose(file);
// }
// static void printToOutput(FILE *output, CPU *cpu, Memory *memory ) {
//     // Iterating through registers array for values in registers
//     fprintf( output, "Registers:\n" );
//     for ( int i = 0; i < 31; i++ ) {
//         if ( i < 10 ) {
//             fprintf( output, "X0%d    = %.16llx\n", i, cpu->registers[i] );
//         } else {
//             fprintf( output, "X%d    = %.16llx\n", i, cpu->registers[i] );
//         }
//     }

//     // Printing value in PC
//     fprintf( output, "PC     = %.16llx\n", cpu->pc );

//     // Printing condition flags
//     fprintf( output, "PSTATE : " );
//     if (cpu->pstateN) {
//         fprintf( output, "N" );
//     } else {
//         fprintf( output, "-" );
//     }
//     if (cpu->pstateZ) {
//         fprintf( output, "Z" );
//     } else {
//         fprintf( output, "-" );
//     }
//     if (cpu->pstateC) {
//         fprintf( output, "C" );
//     } else {
//         fprintf( output, "-" );
//     }
//     if (cpu->pstateV) {
//         fprintf( output, "V\n" );
//     } else {
//         fprintf( output, "-\n" );
//     }

//     // Outputting any non-zero memory
//     fprintf( output, "Non-Zero Memory:\n" );
//     for ( uint64_t address = 0; address < MEMORY_SIZE; address += 4 ) {
//         uint32_t word = reverse_word( get_word( memory, address ) );
//         if ( word != 0 ) {
//             fprintf( output, "0x%.8llx : %.8x\n", address, word );
//         }
//     }
// }

static void saving(GtkNotebook* notebook, char mode) {
    gint current_page_no = gtk_notebook_get_current_page(notebook); 
    GtkWidget *page = gtk_notebook_get_nth_page(notebook, current_page_no);
    GtkTextView *textView = GTK_TEXT_VIEW(gtk_bin_get_child(GTK_BIN(page)));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(textView);
    ErrorCheckData *errData = create_err_data(textView, buffer);
    check_errors(errData);
    gboolean no_error = TRUE;
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);

    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    gchar *last_line = NULL;
    g_free(text);
    for (int i = 0; lines[i] != NULL; i++) {
        if (g_hash_table_lookup(errData->error_messages, GINT_TO_POINTER(i))) {
            no_error = FALSE;
            break;
        }
        gchar* stripped_line = strip_spaces(lines[i]);
        if (lines[i] && g_strcmp0(stripped_line, "") != 0) {
            g_free(last_line);
            last_line = g_strdup(lines[i]);
        }
        g_free(stripped_line);
    }
    g_strfreev(lines);

    // now we got last line and whether there are no error messages or not
    if (no_error && last_line) { // allow saving an empty file
        gchar* stripped_final = strip_spaces(last_line);
        no_error = g_strcmp0(stripped_final, "andx0,x0,x0") == 0;
        g_free(stripped_final);
    }
    g_free(last_line);
    free_errData(NULL, errData);

    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Save File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Save",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    while (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        // Check if the file has the correct extension
        if (g_str_has_suffix(filename, ".s") && no_error) {

            GtkTextIter start, end;
            gtk_text_buffer_get_start_iter(buffer, &start);
            gtk_text_buffer_get_end_iter(buffer, &end);
            gchar *content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

            g_file_set_contents(filename, content, -1, NULL);

            // if (mode == 'b' || mode == 'e') {
            //     program program = parse_assembly_file(filename);
            //     strtok(filename, ".");
            //     strcat(filename, ".bin\0");
            //     encode(program, filename);
            //     free_program(program);
            //     if (mode == 'e') {
            //         CPU cpu;
            //         Memory memory;
            //         init_cpu(&cpu);
            //         init_memory(&memory);
            //         load_binary_file(filename, memory.data, MEMORY_SIZE);


            //         strtok(filename, ".");
            //         strcat(filename, ".txt\0");
            //         // .out
            //         while (1) {
            //             uint32_t instruction = reverse_word(get_word(&memory, cpu.pc));
            //             // Decode here
            //             if (instruction == 0x8a000000) { // Custom halt instruction
            //                 break;
            //             }
            //             decode_and_execute(&cpu, &memory, instruction);
            //         }
            //         FILE *outFile = fopen( filename, "w");
            //         printToOutput( outFile, &cpu, &memory);
            //         fclose( outFile);
            //     }
            // }

            g_free(content);
            g_free(filename);
            break; // Exit the loop and close the file chooser
            
        } else {
            char *err_message = no_error ? "The selected file is not an assembly file (.s). Please choose a valid filename." : 
            "Selected file has errors: Either the code is not written correctly or terminating statement is missing.";
            // Show an error dialog if the file is not an assembly file
            GtkWidget *errorDialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            "%s", err_message);
            gtk_dialog_run(GTK_DIALOG(errorDialog));
            gtk_widget_destroy(errorDialog);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Function to strip all spaces

void on_save_file(GtkWidget *widget, gpointer data) {
    saving(GTK_NOTEBOOK(data), 's');
}

void on_build_bin(GtkWidget *widget, gpointer data) {
    saving(GTK_NOTEBOOK(data), 'b');
}

void on_build_exc(GtkWidget *widget, gpointer data) {
    saving(GTK_NOTEBOOK(data), 'e');
}