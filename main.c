#include <gtk/gtk.h>
#include <i3ipc-glib/i3ipc-glib.h>
#include <stdio.h>
#include <webkit2/webkit2.h>
#include <unistd.h>
#include <limits.h>

const int BAR_W = 1920;
const int BAR_H = 50;
char bar_uri[PATH_MAX];
char web_ext_dir[PATH_MAX];

// Initialise web view
static WebKitWebView *web_view_init() {
  WebKitWebView *web_view;
  GdkRGBA rgba = {.alpha = 0.0};

  WebKitSettings *web_view_settings = webkit_settings_new();
  webkit_settings_set_enable_write_console_messages_to_stdout(web_view_settings, TRUE);
  web_view = WEBKIT_WEB_VIEW(webkit_web_view_new_with_settings(web_view_settings));

  webkit_web_view_set_background_color(web_view, &rgba);

  return web_view;
}

// Initialise web extensions
static void *initialize_web_extensions_cb(WebKitWebContext *context,
                                          gpointer user_data) {
  static guint32 unique_id = 0;
  webkit_web_context_set_web_extensions_directory(context, web_ext_dir);
  webkit_web_context_set_web_extensions_initialization_user_data(
      context, g_variant_new_uint32(unique_id++));
}

int main(int argc, char *argv[]) {
  WebKitWebView *web_view;
  GtkWidget *layout;
  GtkWidget *window;
  GdkScreen *screen;
  GdkVisual *visual;

  char *currentDir = getcwd(NULL, 0);

  sprintf(bar_uri, "file://%s/ui/ui.html", currentDir);
  sprintf(web_ext_dir, "%s/jsc/bin", currentDir);

  printf("Bar started...\n");
  gtk_init(&argc, &argv);

  web_view = web_view_init();
  layout = gtk_layout_new(NULL, NULL);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  screen = gtk_widget_get_screen(window);
  visual = gdk_screen_get_rgba_visual(screen);

  // GTK properties
  gtk_window_set_default_size(GTK_WINDOW(window), BAR_W, BAR_H);
  gtk_window_stick(GTK_WINDOW(window));
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
  gtk_window_set_gravity(GTK_WINDOW(window), GDK_GRAVITY_STATIC);
  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
  gtk_widget_set_size_request(GTK_WIDGET(web_view), BAR_W, BAR_H);
  gtk_widget_set_visual(GTK_WIDGET(web_view), visual);
  gtk_widget_set_visual(window, visual);
  gtk_widget_set_app_paintable(window, TRUE);

  // Add web view to layout, add layer to main window
  gtk_container_add(GTK_CONTAINER(layout), GTK_WIDGET(web_view));
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(layout));

  // Handle signals
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(web_view, "context-menu", G_CALLBACK(gtk_true), NULL);
  g_signal_connect(webkit_web_context_get_default(),
                   "initialize-web-extensions",
                   G_CALLBACK(initialize_web_extensions_cb), NULL);

  // Load the bar interface in the web view
  webkit_web_view_load_uri(web_view, bar_uri);

  // Make sure that when the browser area becomes visible, it will get mouse
  // and keyboard events
  gtk_widget_grab_focus(GTK_WIDGET(web_view));

  // Make sure the main window and all its contents are visible
  gtk_widget_show_all(window);

  // Run the main gtk event loop
  gtk_main();

  return 0;
}