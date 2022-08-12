#include <gtk/gtk.h>
#include "sqlite3.h"
#include <stdio.h>
#include <string.h>
    //Виджеты главного меню
    GtkWidget *window, *treeView, *listStore;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeSelection **selection;
    GtkButton *btnAddEntry, *btnDeleteEntry;
    GtkBuilder *ui_builder;
    //Виджеты Dialog
    GtkDialog *dialog;
    GtkEntry *entryNameCar, *entryNumberCar;
    //Виджеты messageDialog
    GtkWidget *messageDialog;

//Перечисления
enum {
    LIST_ID,
    LIST_NAME,
    LIST_NUMBER,
    N_COLUMNS
};

//Считывание данных
int callback(void *model, int argc, char **argv, char **azColName) {
    GtkTreeIter iter;

    for (int i = 0; i < argc; i++) {

        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

    }

    printf("\n");

    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter, LIST_ID, argv[0],
                        LIST_NAME, argv[1],
                        LIST_NUMBER, argv[2],
                        -1);

    return 0;
}

//Добавить новую запись
static void insert_into_sql(gchar *ent1, gchar *ent2){
    sqlite3 *db;
    gchar *err_msg = 0;
    //открыть бд
    int rc = sqlite3_open("../bdSQLite/bd.sqlite", &db);
    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n",
                sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    gchar sql[1024];
    //Склеивание строки для sql запроса
    snprintf(sql, 1024, "INSERT INTO Cars(Name, Number) VALUES('%s', '%s');", ent1, ent2);
    rc = sqlite3_exec(db, sql, callback, listStore, &err_msg);

    if (rc != SQLITE_OK ) {

        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }

    gchar *s = "SELECT * FROM Cars ORDER BY ID DESC LIMIT 1";
    rc = sqlite3_exec(db, s, callback, listStore, &err_msg);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), GTK_TREE_MODEL(listStore));
    //закрыть бд
    sqlite3_close(db);
}

//Удаление записи
static void remove_item(GtkWidget *widget, gpointer data) {

    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter  iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeView)));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

    if (gtk_tree_model_get_iter_first(model, &iter) == FALSE) {
        return;
    }

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),
                                        &model, &iter)) {
        //Переменная куда записывается ID значение с model
        gchar *s;
        //Берем ID
        gtk_tree_model_get (model, &iter, 0, &s, -1);
        //Создание Sql запроса на удаление
        gchar sql[1024];
        snprintf(sql, 1024, "DELETE FROM Cars WHERE ID = '%s'", s);
        //Подключение БД
        sqlite3 *db;
        gchar *err_msg = 0;
        //открыть бд
        int rc = sqlite3_open("../bdSQLite/bd.sqlite", &db);
        if (rc != SQLITE_OK) {

            fprintf(stderr, "Cannot open database: %s\n",
                    sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }
        rc = sqlite3_exec(db, sql, callback, listStore, &err_msg);
        if (rc != SQLITE_OK ) {

            fprintf(stderr, "Failed to select data\n");
            fprintf(stderr, "SQL error: %s\n", err_msg);

            sqlite3_free(err_msg);
            sqlite3_close(db);
            return;
        }
        //закрыть бд
        sqlite3_close(db);

        //Удаляем строку с list
        gtk_list_store_remove(store, &iter);
    }
}

//Открыть диалог
static void on_button_clicked_open_dialog(GtkWidget *object, gpointer dialog){
    gchar *str1, *str2;
    gint result = gtk_dialog_run(dialog);
    switch (result) {
        case GTK_RESPONSE_YES:
            str1 = gtk_entry_get_text(entryNameCar);
            str2 = gtk_entry_get_text(entryNumberCar);
            insert_into_sql(str1, str2);
            break;
        case GTK_RESPONSE_NO:
            gtk_widget_hide(dialog);
            break;
        default:
            break;
    }
    gtk_widget_hide(dialog);
}

//открыть Message Dialog
static void on_button_clicked_open_message_dialog(GtkWidget *object, gpointer messageDialog){
    gint result = gtk_dialog_run(messageDialog);
    switch (result) {
        case GTK_RESPONSE_YES:
            remove_item(NULL,NULL);
            gtk_widget_hide(messageDialog);
            break;
        case GTK_RESPONSE_NO:
            gtk_widget_hide(messageDialog);
            break;
        default:
            break;
    }
    gtk_widget_hide(messageDialog);
}

//обработчик сигнала закрытия главного окна
void on_window_destroy (GtkWidget *object, gpointer user_data)
{
    gtk_main_quit ();	//выйти из главного цикла
}

int main(int argc, char** argv){
    //инициализация gtk
    gtk_init(&argc, &argv);
    //ошибки
    GError *err = NULL;
    //инициализируем GtkBuilder
    ui_builder = gtk_builder_new();
    //загрузка файла с UI
    if(!gtk_builder_add_from_file(ui_builder, "../window.ui", &err)){
        g_critical("file does not open %s", err->message);
        g_error_free(err);
        return 1;
    }

    //Берем виджеты
    window = GTK_WIDGET(gtk_builder_get_object(ui_builder, "window"));
    treeView = GTK_WIDGET(gtk_builder_get_object(ui_builder, "treeView"));
    selection = GTK_TREE_SELECTION(gtk_builder_get_object(ui_builder, "treeSelection"));
    listStore = GTK_LIST_STORE(gtk_builder_get_object(ui_builder, "listStore"));
    btnAddEntry = GTK_BUTTON(gtk_builder_get_object(ui_builder, "btnAddEntry"));
    btnDeleteEntry = GTK_BUTTON(gtk_builder_get_object(ui_builder, "btnDeleteEntry"));

    //Виджеты диалога
    dialog = GTK_DIALOG(gtk_builder_get_object(ui_builder, "dialog"));
    messageDialog = GTK_DIALOG(gtk_builder_get_object(ui_builder, "messageDialog"));
    entryNameCar = GTK_ENTRY(gtk_builder_get_object(ui_builder, "entryNameCar"));
    entryNumberCar = GTK_ENTRY(gtk_builder_get_object(ui_builder, "entryNumberCar"));

    //Сигналы
    g_signal_connect(GTK_WIDGET(window), "destroy", G_CALLBACK(on_window_destroy), NULL);
    g_signal_connect(GTK_BUTTON(btnAddEntry), "clicked", G_CALLBACK(on_button_clicked_open_dialog), dialog);
    g_signal_connect(GTK_BUTTON(btnDeleteEntry), "clicked", G_CALLBACK(on_button_clicked_open_message_dialog), messageDialog);

    //БД
    sqlite3 *db;
    gchar *err_msg = 0;
    //открыть бд
    int rc = sqlite3_open("../bdSQLite/bd.sqlite", &db);
    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n",
                sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    gchar *sql;

    sql = "SELECT * FROM Cars";

    rc = sqlite3_exec(db, sql, callback, listStore, &err_msg);

    if (rc != SQLITE_OK ) {

        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    //закрыть бд
    sqlite3_close(db);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("ID",
                                                      renderer, "text", LIST_ID, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("NAME",
                                                      renderer, "text", LIST_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("NUMBER",
                                                      renderer, "text", LIST_NUMBER, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);

    gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), GTK_TREE_MODEL(listStore));

    // связываем сигналы с объектами графического интерфейса
    gtk_builder_connect_signals (ui_builder, NULL);

    // освобождение памяти
    g_object_unref(G_OBJECT(ui_builder));

    // Показываем форму и виджеты на ней
    gtk_widget_show(window);

    // запуск главного цикла приложения
    gtk_main();
}
