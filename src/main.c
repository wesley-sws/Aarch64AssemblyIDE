#include <gtk/gtk.h>
#include "../include/events.h"
#include "../include/auto_complete.h"
#include "../include/apply_css.h"
#include <gtksourceview/gtksource.h>
#include "../include/menu.h"
#include "../include/tabs.h"

int main(int argc, char *argv[]) {

    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *menubar;
    GtkWidget *fileMenu;
    GtkWidget *buildMenu;
    GtkWidget *themeMenu;
    GtkWidget *fileMi;
    // GtkWidget *buildMi;
    GtkWidget *themeMi;
    GtkWidget *openMi;
    GtkWidget *saveMi;
    // GtkWidget *buildBinMi;
    // GtkWidget *buildExcMi;
    GtkWidget *sourceView;
    GtkWidget *scrollWin;
    GtkWidget *notebook; 
    GtkWidget *button_light;
    GtkWidget *button_dark;
    GtkWidget *sidebar;
    GtkWidget *paned;
    GtkWidget *listbox;
    GtkWidget *button_catppuccin;

    gtk_init(&argc, &argv);

    // Apply CSS for styling
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    apply_css("style_cat.css", provider);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Assembly IDE");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    menubar = gtk_menu_bar_new();
    fileMenu = gtk_menu_new();
    buildMenu = gtk_menu_new();
    themeMenu = gtk_menu_new();
    notebook = gtk_notebook_new();

    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);

    fileMi = gtk_menu_item_new_with_label("File");
    openMi = gtk_menu_item_new_with_label("Open");
    saveMi = gtk_menu_item_new_with_label("Save");

    // buildMi = gtk_menu_item_new_with_label("Build");
    // buildBinMi = gtk_menu_item_new_with_label("Save->Build to Bin");
    // buildExcMi = gtk_menu_item_new_with_label("Save->Build to Bin->Emulate");

    themeMi = gtk_menu_item_new_with_label("Themes");
    button_light = gtk_menu_item_new_with_label("Light");
    button_dark = gtk_menu_item_new_with_label("Dark");
    button_catppuccin = gtk_menu_item_new_with_label("Catppuccin");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), openMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);

    // gtk_menu_item_set_submenu(GTK_MENU_ITEM(buildMi), buildMenu);
    // gtk_menu_shell_append(GTK_MENU_SHELL(buildMenu), buildBinMi);
    // gtk_menu_shell_append(GTK_MENU_SHELL(buildMenu), buildExcMi);
    // gtk_menu_shell_append(GTK_MENU_SHELL(menubar), buildMi);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(themeMi), themeMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(themeMenu), button_light);
    gtk_menu_shell_append(GTK_MENU_SHELL(themeMenu), button_dark);
    gtk_menu_shell_append(GTK_MENU_SHELL(themeMenu), button_catppuccin);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), themeMi);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // Create a GtkPaned widget to hold the notebook and sidebar
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);

    // Create the sidebar
    sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(sidebar, "sidebar");
    gtk_widget_set_size_request(sidebar, 150, -1);
    gtk_paned_pack1(GTK_PANED(paned), sidebar, FALSE, FALSE);

    // Create a GtkListBox for the sidebar
    listbox = gtk_list_box_new();
    gtk_list_box_set_activate_on_single_click(GTK_LIST_BOX(listbox), TRUE);

    scrollWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_name(listbox, "sidebar-listbox"); // Set a unique name
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrollWin), listbox);
    gtk_box_pack_start(GTK_BOX(sidebar), scrollWin, TRUE, TRUE, 0);
    gtk_paned_pack2(GTK_PANED(paned), notebook, TRUE, FALSE);

    // // Create and initialize CallbackData
    CallbackData *callback_data = g_new(CallbackData, 1);
    callback_data->notebook = notebook;
    callback_data->listbox = listbox;
    add_new_tab(callback_data, "/Untitled.s", "");

    // g_signal_connect(G_OBJECT(buildBinMi), "activate", G_CALLBACK(on_build_bin), notebook);
    // g_signal_connect(G_OBJECT(buildExcMi), "activate", G_CALLBACK(on_build_exc), notebook);

    g_signal_connect(G_OBJECT(openMi), "activate", G_CALLBACK(on_open_file), callback_data);
    g_signal_connect(G_OBJECT(saveMi), "activate", G_CALLBACK(on_save_file), notebook);
    g_signal_connect(G_OBJECT(notebook), "switch-page", G_CALLBACK(change_sidebar), listbox);

    // Connect signals for theme change
    g_signal_connect(G_OBJECT(button_light), "activate", G_CALLBACK(apply_light_theme), provider);
    g_signal_connect(G_OBJECT(button_dark), "activate", G_CALLBACK(apply_dark_theme), provider);
    g_signal_connect(G_OBJECT(button_catppuccin), "activate", G_CALLBACK(apply_catppuccin_theme), provider);
    gtk_widget_show_all(sidebar);
    gtk_widget_show_all(scrollWin);
    gtk_widget_show_all(listbox);
    gtk_widget_show_all(window);
    gtk_main();

    // Free the CallbackData struct
    g_free(callback_data);

    return 0;
}