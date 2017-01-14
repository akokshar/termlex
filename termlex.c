#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <vte/vte.h>

void vte_child_exited_callback(VteTerminal *vte, gpointer user_data) {
	GPid *data = (GPid *) user_data;
	GPid child_pid = *data;

	if (child_pid != 0) {

	}

	gtk_main_quit();
}

void vte_beep_callback(VteTerminal *vte, gpointer user_data) {
	GtkWindow **data = (GtkWindow **) user_data;
	GtkWindow *window = (GtkWindow *) *data;

	if (window != NULL) {
		gtk_window_set_urgency_hint(window, TRUE);
	}
}

gboolean vte_key_press_callback(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	if (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
//	if (event->state & GDK_CONTROL_MASK == GDK_CONTROL_MASK) {
		switch (gdk_keyval_to_upper(event->keyval)) {
			case GDK_KEY_C:
				vte_terminal_copy_clipboard((VteTerminal *) widget);
				return TRUE;
			case GDK_KEY_V:
				vte_terminal_paste_clipboard((VteTerminal *) widget);
				return TRUE;
		}
	}

	return FALSE;	
}

int main (int argc, char *argv[]) {
	
	GtkWindow *window = NULL;
	GtkBox *box = NULL;
	VteTerminal *vte = NULL;

	GPid child_pid = 0;
	GError *error = NULL;

	gchar *command = command = vte_get_user_shell();
	if (command == NULL) {
		command = g_strdup("/bin/sh");
	}
	gchar *command_argv[] = { command, NULL };

	gtk_init(&argc, &argv);

	vte = (VteTerminal *) vte_terminal_new();
	vte_terminal_set_scrollback_lines(vte, -1); //infinite scrollback
	vte_terminal_set_font_from_string (vte, "PT Mono 12");
	g_signal_connect(vte, "child-exited", G_CALLBACK(vte_child_exited_callback), &child_pid);
	g_signal_connect(vte, "beep", G_CALLBACK(vte_beep_callback), &window);
	g_signal_connect(vte, "key-press-event", G_CALLBACK (vte_key_press_callback), NULL);

	vte_terminal_fork_command_full(
		(VteTerminal *) vte,
		(VTE_PTY_NO_HELPER),
		NULL,
		command_argv,
		NULL,
		(G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH | G_SPAWN_LEAVE_DESCRIPTORS_OPEN),
		NULL,
		NULL,
		&child_pid,
		&error);

	box = (GtkBox *) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(box, (GtkWidget *) vte, TRUE, TRUE, 0);

	window = (GtkWindow *) gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", gtk_main_quit, NULL);
	gtk_window_set_role(window, "Termlex");
	gtk_window_set_title(window, "Termlex");
	gtk_container_add((GtkContainer *) window, (GtkWidget *) box);

	gtk_widget_show_all((GtkWidget *) window);
	gtk_main();

	g_free(command);

	return EXIT_SUCCESS;
}
