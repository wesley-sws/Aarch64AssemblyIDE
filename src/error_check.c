#include "../include/error_check.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

static const char *valid_mnemonics[] = {
    "add", "adds", "sub", "subs", "cmp", "cmn", "neg", "negs",
    "and", "ands", "bic", "bics", "eor", "eon", "orr", "orn",
    "tst", "mvn", "mov", "movn", "movz", "movk", "madd", "msub",
    "mul", "mneg", "b", "br", "b.eq", "b.ne", "b.ge", "b.lt",
    "b.gt", "b.le", "b.al", "ldr", "str", ".int"
};

static bool is_valid_mnemonic(char *mnemonic) {
    for (size_t i = 0; i < sizeof(valid_mnemonics) / sizeof(valid_mnemonics[0]); i++) {
        if (strcmp(mnemonic, valid_mnemonics[i]) == 0) {
            return true;
        }
    }
    return false;
}
static char *format_string(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int size = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *result = (char *)malloc(size + 1); // +1 for null terminator
    if (!result) { return NULL; }
    va_start(args, format);
    vsnprintf(result, size + 1, format, args);
    va_end(args);
    return result;
}

static char *produce_error(int operands_count, char **operands, char* mnemonic) {
    if( operands_count < 2 ) {
        return format_string("Too few arguments, %s takes 2 or 3 arguments", mnemonic);
    }
    if( operands_count > 3 ) {
        return format_string("Too many arguments, %s takes 2 or 3 arguments", mnemonic);
    }
    if (operands[0][0] != 'w' && operands[0][0] != 'x') return format_string("First argument must be a register");
    if (operands[1][0] == '#') {
        if (operands_count == 3) {
            if (strncmp(operands[2], "lsl", 3) != 0) return format_string("Third argument must be lsl");
            if (operands[2][4] != '#') return format_string("Immediate must start with #.");
        }
        return NULL;
    } else {
        if (!((operands[1][0] == 'w' && operands[0][0] == 'w') || (operands[1][0] == 'x' && operands[0][0] == 'x'))) 
            return format_string("These two arguments must be registers and must be the same type");
        if (operands_count == 3) {
            if ((strncmp(operands[2], "lsl", 3) != 0) &&  
                (strncmp(operands[2], "lsr", 3) != 0) &&
                (strncmp(operands[2], "asr", 3) != 0) &&
                (strncmp(operands[2], "ror", 3) != 0)) {
                return format_string("Invalid shift argument for argument 4. Must be one of lsl, lsr, asr.");
            }
            if (operands[2][4] != '#') {
                return format_string("Immediate must start with #.");
            }
        }
        return NULL; 
    }
}

static bool space_or_tab(char space) {
    return space == ' ' || space == '\t';
}

static void strip_spaces(char *line) {
    // Strip leading white spaces
    int i = 0;
    int k = 0;
    while (space_or_tab((unsigned char)line[i])) {
        i++;
    }
    while (line[i] != '\0') {
        line[k] = line[i];
        k++;
        i++;
    }
    
    // Null-terminate the string
    line[k] = '\0';
    
    // Strip trailing white spaces
    k--;
    while (k >= 0 && isspace((unsigned char)line[k])) 
        k--;
}

static char *produce_error_message(char *line) {

    // Strip spaces
    strip_spaces(line);

    // Get the length of the line
    int length = strlen(line);

    // Check if line is empty
    if (length == 0)
    {
        return NULL;
    }

    // Strip newline character
    if (line[length - 1] == '\n')
    {
        line[length - 1] = '\0';
    }

    // Check if the line is a label
    length = strlen(line);
    if (line[length - 1] == ':')
    {
        // is a label, valid line
        return NULL;
    } else {
        char *line_copy = line;

        // Parse the mnemonic
        char *mnemonic = strtok(line_copy, " ");


        // Parse the first 6 operands (operands are things seperated by commas)
        char *operands[5];
        int operands_count = 0;
        for( char *token; (token = strtok(NULL, ",")) != NULL && operands_count <5; operands_count ++) {
            strip_spaces(token);
            operands[operands_count] = token;
        }

        // First check: Is it a valid mnemonic
        if( !is_valid_mnemonic(mnemonic) ) {
            char *formatted_string = format_string("%s is not a valid mnemonic", mnemonic);
            return formatted_string;
        }


        // Second check: Is there correct num of argumnets for each mnemonic
        
        // Instruction that take 3, 4 arguments
        if( strcmp(mnemonic, "add") == 0||
            strcmp(mnemonic, "adds") == 0 ||
            strcmp(mnemonic, "sub") == 0 ||
            strcmp(mnemonic, "subs") == 0 ||
            strcmp(mnemonic, "and") == 0 ||
            strcmp(mnemonic, "ands") == 0 ||
            strcmp(mnemonic, "bic") == 0 ||
            strcmp(mnemonic, "bics") == 0 ||
            strcmp(mnemonic, "eor") == 0 ||
            strcmp(mnemonic, "eon") == 0 ||
            strcmp(mnemonic, "orr") == 0 ||
            strcmp(mnemonic, "orn") == 0
        ) {
            if( operands_count < 3 ) {
                return format_string("Too few arguments, %s takes 3 or 4 arguments", mnemonic);
            }
            if( operands_count > 4 ) {
                return format_string("Too many arguments, %s takes 3 or 4 arguments", mnemonic);
            }
            if ( strcmp(mnemonic, "and") == 0 ||
            strcmp(mnemonic, "ands") == 0 ||
            strcmp(mnemonic, "bic") == 0 ||
            strcmp(mnemonic, "bics") == 0 ||
            strcmp(mnemonic, "eor") == 0 ||
            strcmp(mnemonic, "eon") == 0 ||
            strcmp(mnemonic, "orr") == 0 ||
            strcmp(mnemonic, "orn") == 0 )  {
                if (!(operands[0][0] != 'w' && operands[1][0] != 'w' && operands[2][0] != 'w') &&
                    !(operands[0][0] != 'x' && operands[1][0] != 'x' && operands[2][0] != 'x')) {
                        return format_string("Arguments must be all w or x registers.");
                    }
                if (operands_count == 4) {
                    if ((strncmp(operands[3], "lsl", 3) != 0) &&  
                        (strncmp(operands[3], "lsr", 3) != 0) &&
                        (strncmp(operands[3], "ror", 3) != 0) &&
                        (strncmp(operands[3], "asr", 3) != 0) ) {
                        return format_string("Invalid shift argument for argument 4. Must be one of lsl, lsr, asr, ror.");
                    }
                    if (operands[3][4] != '#') {
                        return format_string("Immediate must start with #.");
                    }
                }
                
            }
            if(strcmp(mnemonic, "add") == 0||
            strcmp(mnemonic, "adds") == 0 ||
            strcmp(mnemonic, "sub") == 0 ||
            strcmp(mnemonic, "subs") == 0 ) {
                if (!(operands[0][0] == 'w' && operands[1][0] == 'w') && !(operands[0][0] == 'x' && operands[1][0] == 'x')) {
                    return format_string("Arguments must be all w or x registers.");
                }
                if (operands_count == 3) {
                    if (operands[2][0] != '#' && operands[2][0] != 'w' && operands[2][0] != 'x'){
                        return format_string("3rd argument must be either a register or an immediate value");
                    }
                    if (operands[2][0] != operands[1][0] && operands[2][0] != '#') {
                        return format_string("3rd argument has the wrong type of register");
                    }
                } else if (operands_count == 4) {
                    if ((strncmp(operands[3], "lsl", 3) != 0) &&  
                        (strncmp(operands[3], "lsr", 3) != 0) &&
                        (strncmp(operands[3], "asr", 3) != 0) ) {
                        return format_string("Invalid shift argument for argument 4. Must be one of lsl, lsr, asr.");
                    }
                    if (operands[3][4] != '#') {
                        return format_string("Immediate must start with #.");
                    }
                }
            }
        }

        // Instruction that take 2, 3 arguments
        if( strcmp(mnemonic, "cmp") == 0 ||
            strcmp(mnemonic, "cmn") == 0 ||
            strcmp(mnemonic, "neg") == 0 ||
            strcmp(mnemonic, "negs") == 0 ||
            strcmp(mnemonic, "tst") == 0 ||
            strcmp(mnemonic, "mvn") == 0 ||
            strcmp(mnemonic, "movn") == 0 ||
            strcmp(mnemonic, "movk") == 0 ||
            strcmp(mnemonic, "movz") == 0
        ) return produce_error(operands_count, operands, mnemonic);

        if ( strcmp(mnemonic, "str") == 0 || 
             strcmp(mnemonic, "ldr") == 0
        ) {
            if( operands_count < 2 ) {
                return format_string("Too few arguments, %s takes 2 or 3 arguments", mnemonic);
            }
            if( operands_count > 3 ) {
                return format_string("Too many arguments, %s takes 2 or 3 arguments", mnemonic);
            }
        }
        // Instruction that take 4 arguments
        if( strcmp(mnemonic, "madd") == 0 ||
            strcmp(mnemonic, "msub") == 0
        ) {
            if( operands_count < 4 ) {
                return format_string("Too few arguments, %s takes 4 arguments", mnemonic);
            }
            if( operands_count > 4 ) {
                return format_string("Too many arguments, %s takes 4 arguments", mnemonic);
            }
            if (!(operands[0][0] != 'w' && operands[1][0] != 'w' && operands[2][0] != 'w' && operands[3][0] != 'w') &&
                !(operands[0][0] != 'x' && operands[1][0] != 'x' && operands[2][0] != 'x' && operands[3][0] != 'x')) {
                    return format_string("Arguments must be all w or x registers.");
            }
        }

        // Instruction that take 3 arguments
        if( strcmp(mnemonic, "mul") == 0 ||
            strcmp(mnemonic, "mneg") == 0
        ) {
            if( operands_count < 3 ) {
                return format_string("Too few arguments, %s takes 3 arguments", mnemonic);
            }
            if( operands_count > 3 ) {
                return format_string("Too many arguments, %s takes 3 arguments", mnemonic);
            }
            if (!(operands[0][0] != 'w' && operands[1][0] != 'w' && operands[2][0] != 'w') &&
                !(operands[0][0] != 'x' && operands[1][0] != 'x' && operands[2][0] != 'x')) {
                    return format_string("Arguments must be all w or x registers.");
            }
        }

        // Instruction that take 2 arguments
        if( strcmp(mnemonic, "mov") == 0) {
            if( operands_count < 2 ) {
                return format_string("Too few arguments, %s takes 2 arguments", mnemonic);
            }
            if( operands_count > 2 ) {
                return format_string("Too many arguments, %s takes 2 arguments", mnemonic);
            }
            if (!(operands[0][0] != 'w' && operands[1][0] != 'w') &&
                        !(operands[0][0] != 'x' && operands[1][0] != 'x')) {
                            return format_string("Arguments must be all w or x registers.");
                        }
        }

        // Instruction that take 1 arguments
        if( strcmp(mnemonic, "b") == 0 ||
            strcmp(mnemonic, "br") == 0 ||
            strcmp(mnemonic, "b.eq") == 0 ||
            strcmp(mnemonic, "b.ne") == 0 ||
            strcmp(mnemonic, "b.ge") == 0 ||
            strcmp(mnemonic, "b.lt") == 0 ||
            strcmp(mnemonic, "b.gt") == 0 ||
            strcmp(mnemonic, "b.le") == 0 ||
            strcmp(mnemonic, "b.al") == 0 ||
            strcmp(mnemonic, ".int") == 0
        ) {
            if( operands_count < 1 ) {
                return format_string("Too few arguments, %s takes 1 arguments", mnemonic);
            }
            if( operands_count > 1 ) {
                return format_string("Too many arguments, %s takes 1 arguments", mnemonic);
            }
        }
    }
    return NULL;
}

ErrorCheckData* create_err_data(GtkTextView* textView, GtkTextBuffer* buffer) {
    ErrorCheckData* errData = g_new0(ErrorCheckData, 1);
    errData->textView = textView;
    errData->timer_id = 0;
    errData->error_messages = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
    errData->error_popup = gtk_window_new(GTK_WINDOW_POPUP);
    errData->buffer = buffer;
    GtkWidget *error_label = gtk_label_new(NULL);
    gtk_container_add(GTK_CONTAINER(errData->error_popup), error_label);
    return errData;
}



void free_errData(GtkWidget *widget, gpointer data) {
    ErrorCheckData *errData = (ErrorCheckData *)data; 
    if (errData->timer_id != 0) {
        g_source_remove(errData->timer_id);
    }
    if (errData->error_messages) {
        g_hash_table_destroy(errData->error_messages);
    }
    if (errData->error_popup) {
        gtk_widget_destroy(errData->error_popup);
    }
    g_free(errData);
}

gboolean check_errors(gpointer user_data) {

    ErrorCheckData *errorData = (ErrorCheckData *)user_data;
    GtkTextBuffer *buffer = errorData->buffer;

    GtkTextIter start, end;
    GtkTextTag *error_tag;

    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(buffer);
    error_tag = gtk_text_tag_table_lookup(tag_table, "error");

    // If the error tag does not exist, create it
    if (error_tag == NULL) {
        error_tag = gtk_text_buffer_create_tag(buffer, "error",
                                               "underline", PANGO_UNDERLINE_ERROR,
                                               NULL);
    }
    // Remove previous error underlines

    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);

    gtk_text_buffer_remove_tag_by_name(buffer, "error", &start, &end);


    // Get the text of the buffer
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    g_free(text);


    // Iterate through each line and check for syntax errors
    for (int i = 0; lines[i] != NULL; i++) {
        // check condition
        char *error_message = produce_error_message(lines[i]);
        if (error_message != NULL) {
            GtkTextIter line_start_iter, line_end_iter;
            gtk_text_buffer_get_iter_at_line(buffer, &line_start_iter, i);
            gtk_text_buffer_get_iter_at_line(buffer, &line_end_iter, i);
            gtk_text_iter_forward_to_line_end(&line_end_iter);

            // Move line_start_iter to the first non-space character
            while (gtk_text_iter_compare(&line_start_iter, &line_end_iter) < 0 &&
                   g_unichar_isspace(gtk_text_iter_get_char(&line_start_iter))) {
                gtk_text_iter_forward_char(&line_start_iter);
            }

            // Move last_non_space_iter to the last non-space character
            GtkTextIter last_non_space_iter = line_end_iter;

            // Ensure there is a valid range before applying the tag
            if (gtk_text_iter_compare(&line_start_iter, &last_non_space_iter) < 0) {
                // Apply the error tag to the line with a syntax error
                gtk_text_buffer_apply_tag(buffer, error_tag, &line_start_iter, &last_non_space_iter);

                // Store the error message for this line
                g_hash_table_insert(errorData->error_messages, GINT_TO_POINTER(i), g_strdup(error_message));
            }
            free(error_message);
        }
        else {
            g_hash_table_insert(errorData->error_messages, GINT_TO_POINTER(i), NULL);
        }
    }

    g_strfreev(lines);

    // Returning FALSE means the timeout will not be called again
    errorData->timer_id = 0;
    return FALSE;
}
