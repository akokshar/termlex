#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <signal.h>

#include <vte/vte.h>

void vte_child_exited_callback(VteTerminal *vte, GPid *child_pid) {
	//GPid pid = *child_pid;
	//int vte_terminal_get_child_exit_status(VteTerminal *terminal);
	gtk_main_quit();
}

void vte_beep_callback(VteTerminal *vte, GtkWindow **window) {
	if (!gtk_window_is_active(*window)) {
		gtk_window_set_urgency_hint(*window, TRUE);
	}
}

void vte_focus_in_event_callback(GtkWidget *widget, GdkEvent *event, GtkWindow **window) {
	if (gtk_window_get_urgency_hint(*window)) {
		gtk_window_set_urgency_hint(*window, FALSE);
	}
}

gboolean vte_key_press_callback(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	if (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
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

gboolean window_delete_event_callback(GtkWidget *widget, GdkEvent *event, GPid *child_pid) {
	GPid pid = *child_pid;

	if (pid != 0) {
		kill(pid, SIGTERM);
		return TRUE;
	}

	gtk_main_quit();

	return FALSE;
}

int main (int argc, char *argv[]) {
	
	GError *error = NULL;

	gtk_init(&argc, &argv);

	gchar *command = NULL;
	const GOptionEntry entries[] = {
		{"command", 'c', 0, G_OPTION_ARG_STRING, &command, "Command to execute. Defaults to user's shell", "COMMAND"},
		{ NULL }
	};

	GOptionContext *option_context = g_option_context_new(NULL);
	g_option_context_set_help_enabled(option_context, TRUE);
	g_option_context_add_main_entries(option_context, entries, NULL);
	g_option_context_parse(option_context, &argc, &argv, &error);
	g_option_context_free(option_context);

	if (error) {
		g_printerr("What did you say?\n %s\n", error->message);
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	if (!command) {
		command = command = vte_get_user_shell();
	}

	if (!command) {
		command = g_strdup("/bin/sh");
	}
	
	gchar **command_argv = NULL;

	g_shell_parse_argv(command, NULL, &command_argv, &error);
	g_free(command);

	if (error) {
		g_printerr("Error parsing command. %s\n", error->message);
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	VteTerminal *vte = NULL;
	GtkScrollbar* scrollbar = NULL;
	GtkBox *box = NULL;
	GtkWindow *window = NULL;

	GPid child_pid = 0;

	vte = (VteTerminal *) vte_terminal_new();
	vte_terminal_set_scrollback_lines(vte, -1); //infinite scrollback
	vte_terminal_set_font_from_string (vte, "PT Mono 12");
	g_signal_connect(vte, "child-exited", G_CALLBACK(vte_child_exited_callback), &child_pid);
	g_signal_connect(vte, "beep", G_CALLBACK(vte_beep_callback), &window);
	g_signal_connect(vte, "focus-in-event", G_CALLBACK (vte_focus_in_event_callback), &window);
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

	g_strfreev(command_argv);

	if (error) {
		g_printerr("Error while running user shell: %s\n", error->message);
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	//scrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, vte_terminal_get_adjustment(vte));

	box = (GtkBox *) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(box, (GtkWidget *) vte, TRUE, TRUE, 0);
	//gtk_box_pack_start(box, (GtkWidget *) scrollbar, FALSE, FALSE, 0);

	window = (GtkWindow *) gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "delete-event", G_CALLBACK (window_delete_event_callback), &child_pid);
	gtk_window_set_role(window, "Termlex");
	gtk_window_set_title(window, "Termlex");
	gtk_container_add((GtkContainer *) window, (GtkWidget *) box);

	gtk_widget_show_all((GtkWidget *) window);
	gtk_main();

	g_free(command);

	return EXIT_SUCCESS;
}
