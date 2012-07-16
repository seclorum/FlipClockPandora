#include <gtk/gtk.h>

static void callback(GtkWidget * widget, gpointer data)
{
	g_print("%s\n",
			gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget)));
}

static gboolean delete_event(GtkWidget * widget, GdkEvent * event,
							 gpointer data)
{
	gtk_main_quit();
	return FALSE;
}


#define MB_YESNO 0x0001

#define IDYES 	0x0001
#define IDNO 	0x0002
#define IDOK 	0x0004

int MessageBox(const char *text, const char *caption, unsigned int type)
{
	GtkWidget *dialog;

	/* Instead of 0, use GTK_DIALOG_MODAL to get a modal dialog box */

	if (type & MB_YESNO)
		dialog =
			gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION,
								   GTK_BUTTONS_YES_NO, text);
	else
		dialog =
			gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO,
								   GTK_BUTTONS_OK, text);


	gtk_window_set_title(GTK_WINDOW(dialog), caption);
	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));

	if (type & MB_YESNO) {
		switch (result) {
		default:
		case GTK_RESPONSE_DELETE_EVENT:
		case GTK_RESPONSE_NO:
			return IDNO;
		case GTK_RESPONSE_YES:
			return IDYES;
		}
		return IDOK;
	}

	return IDNO;
}



void gtk_sample_window()
{
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *box1;


	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(window), "File Chooser Button");

	g_signal_connect(G_OBJECT(window), "delete_event",
					 G_CALLBACK(delete_event), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(window), 10);

	box1 = gtk_hbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(window), box1);

	button =
		gtk_file_chooser_button_new(("Select A File"),
									GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box1), button, TRUE, TRUE, 0);
	gtk_widget_set_size_request(button, 250, -1);

	g_signal_connect((gpointer) button, "selection_changed",
					 G_CALLBACK(callback), NULL);

	gtk_widget_show(box1);
	gtk_widget_show(window);

//  gtk_main();

}



void borked_fselector()
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Open File",
										 NULL,
										 GTK_FILE_CHOOSER_ACTION_OPEN,
										 GTK_STOCK_CANCEL,
										 GTK_RESPONSE_CANCEL,
										 GTK_STOCK_OPEN,
										 GTK_RESPONSE_ACCEPT, NULL);

	printf("Running dialog..\n");
	gtk_dialog_run(GTK_DIALOG(dialog));
	printf("Ended dialog..\n");

}

int main(int argc, char *argv[])
{

	gtk_init(&argc, &argv);
	printf("MB returns: %d\n",
		   MessageBox("Yo dog...", "captioin", MB_YESNO));

//  gtk_sample_window();

	borked_fselector();

	return 0;
}
