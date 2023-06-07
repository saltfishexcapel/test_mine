#include "test_mine-field.h"

/*移除旧的雷区*/
static int field_row = 0;
/*首次点击*/
static int first_click = 1;

/*扫雷引擎地图*/
MineMap* map;

/* Grid 容器*/
static GtkGrid* v_grid;
/* TestMineWindow self */
static TestMineWindow* tmw_self;
/*展示已打开方格的 GtkLabel */
static GtkLabel* opened_label;
static gchar     opened_info[10];
/*时钟 Label */
static GtkLabel* clock_label;
static gchar     clock_array[20];
static mbool     is_stop;

/*样式表
static char* sample_table2[31] = {
        MINEA0, MINEA0, MINEA0, MINEA0, MINEA0, MINEA0, MINEA0, MINEA0,
        MINEA0, MINEA0, MINEB0, MINEB1, MINEB2, MINEB3, MINEB4, MINEB5,
        MINEB6, MINEB7, MINEB8, MINEBM, MINECF, MINECF, MINECF, MINECF,
        MINECF, MINECF, MINECF, MINECF, MINECF, MINECF};
*/
static gint
_test_mine_idle_start_clock (void* data G_GNUC_UNUSED)
{
        gtk_label_set_label (clock_label, clock_array);
        return 0;
}

#define TEST_MINE_MS(us) us * 1000

void*
test_mine_start_clock (void* args G_GNUC_UNUSED)
{
        MTimeVal  now;
        mlf_clock tm_diff;

        is_stop = MFALSE;
        while (!is_stop) {
                MINE_GET_TIME (now);
                tm_diff = MINE_GET_TM_DIFF (map->tm_start, now);
                sprintf (clock_array, "计时: %.2lf", tm_diff);
                g_idle_add (_test_mine_idle_start_clock, NULL);
                usleep (TEST_MINE_MS (30));
        }
        return NULL;
}

#undef TEST_MINE_MS

void
test_mine_end_clock ()
{
        is_stop = MTRUE;
}

void
test_mine_suspend_clock (GtkWidget* widget G_GNUC_UNUSED,
                         gpointer data     G_GNUC_UNUSED)
{
        clock_label = GTK_LABEL (widget);
}

void
test_mine_reset_clock ()
{
        gtk_label_set_label (GTK_LABEL (clock_label), "计时: 0.00");
}

void
test_mine_field_remove_old_field (GtkGrid* grid)
{
        if (field_row == 0)
                return;
        for (int i = 0; i < field_row; ++i)
                gtk_grid_remove_row (grid, 0); /*逐行释放按钮*/
}

/*此函数刷新地雷方格 m 的 gui 对象状态*/
void
test_mine_fresh_mine_state (Mine* m)
{
        GtkWidget* image;
        GtkButton* button;
        if (m == NULL || m->gui_object == NULL)
                return;
        button = GTK_BUTTON (m->gui_object);
        /*地雷方格未打开且插有旗标*/
        if (!m->is_opened)
                if (m->is_flaged)
                        image = gtk_image_new_from_resource (MINECF);
                else
                        image = gtk_image_new_from_resource (MINEA0);
        /*地雷方格已经打开*/
        else if (m->is_opened && m->is_mine)
                image = gtk_image_new_from_resource (MINEBM);
        else {
                switch (m->around) {
                case 0:
                        image = gtk_image_new_from_resource (MINEB0);
                        break;
                case 1:
                        image = gtk_image_new_from_resource (MINEB1);
                        break;
                case 2:
                        image = gtk_image_new_from_resource (MINEB2);
                        break;
                case 3:
                        image = gtk_image_new_from_resource (MINEB3);
                        break;
                case 4:
                        image = gtk_image_new_from_resource (MINEB4);
                        break;
                case 5:
                        image = gtk_image_new_from_resource (MINEB5);
                        break;
                case 6:
                        image = gtk_image_new_from_resource (MINEB6);
                        break;
                case 7:
                        image = gtk_image_new_from_resource (MINEB7);
                        break;
                default:
                        image = gtk_image_new_from_resource (MINEB8);
                        break;
                }
        }
        gtk_button_set_child (button, image);
}

#define IS_X 1
#define IS_Y 0

static void
test_mine_result_printer (const char* charset)
{
        GtkWindow* parent_window;
        GtkWidget* window;
        parent_window = GTK_WINDOW (tmw_self);
        window        = gtk_message_dialog_new (parent_window,
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_CLOSE,
                                         "%s",
                                         charset);
        g_signal_connect (window,
                          "response",
                          G_CALLBACK (gtk_window_destroy),
                          NULL);
        gtk_window_present (GTK_WINDOW (window));
}

static void
test_mine_field_clicker_left (GtkWidget* widget, gpointer data G_GNUC_UNUSED)
{
        guint     x, y;
        MStatus   ms;
        pthread_t tm_th;

        /*从哈希表获取当前按钮的 xy 值*/
        x = object_hash_get_xy (hash, widget, 1);
        y = object_hash_get_xy (hash, widget, 0);

        if (first_click) {
                mine_map_first_click (map, x, y);
                /*发送计时信号*/
                pthread_create (&tm_th, NULL, test_mine_start_clock, NULL);
                pthread_detach (tm_th);
                // g_signal_emit_by_name (clock_label, "clock_start", NULL);
                first_click = 0;
        } else {
                ms = mine_map_click (map, x, y, MCLKL);
        }

        switch (ms) {
        case MFAILED:
                test_mine_result_printer ("游戏失败！\n");
                mine_map_end_clock (map);
                test_mine_end_clock ();
                break;
        case MSUCCESS:
                test_mine_result_printer ("游戏成功！\n");
                mine_map_end_clock (map);
                test_mine_end_clock ();
                break;
        case MPROGERR:
                mine_map_end_clock (map);
                test_mine_end_clock ();
                break;
        default:
                break;
        }
}

static void
test_mine_field_clicker_right (GtkWidget* widget, gpointer data G_GNUC_UNUSED)
{
        guint x, y;
        /*从哈希表获取当前按钮的 xy 值*/
        x = object_hash_get_xy (hash, widget, IS_X);
        y = object_hash_get_xy (hash, widget, IS_Y);
        mine_map_click (map, x, y, MCLKR);
        sprintf (opened_info, "%d / %d", map->flag, map->m);
        gtk_label_set_label (opened_label, opened_info);
}

#undef IS_X
#undef IS_Y

/*为按钮连接信号*/
static void
test_mine_field_connect_signal (GtkWidget* button, gint x, gint y)
{
        GtkGesture* gesture;
        gesture = gtk_gesture_click_new ();

        /*连接鼠标左右键信号*/
        g_signal_connect (button,
                          "clicked",
                          G_CALLBACK (test_mine_field_clicker_left),
                          NULL);
        g_signal_connect (gesture,
                          "released",
                          G_CALLBACK (test_mine_field_clicker_right),
                          NULL);

        gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 0);
        gtk_widget_add_controller (button, GTK_EVENT_CONTROLLER (gesture));

        object_hash_set_data (hash, (gpointer)button, x, y);
        object_hash_set_data (hash, (gpointer)gesture, x, y);
}

void
test_mine_field_add_mines (TestMineWindow* self,
                           GtkGrid*        grid,
                           int             _x,
                           int             _y,
                           int             mnum)
{
        GObject*   olab;
        GtkWidget *button, *image;
        if (grid == NULL)
                return;
        else
                v_grid = grid;

        if (tmw_self == NULL) {
                tmw_self     = self;
                olab         = gtk_widget_get_template_child (GTK_WIDGET (self),
                                                      TEST_MINE_TYPE_WINDOW,
                                                      "map_opened");
                opened_label = GTK_LABEL (olab);
        }
        clock_label =
                GTK_LABEL (gtk_widget_get_template_child (GTK_WIDGET (self),
                                                          TEST_MINE_TYPE_WINDOW,
                                                          "time_label"));
        test_mine_reset_clock ();
        test_mine_field_remove_old_field (grid);
        /*刷新信号哈希表*/
        object_hash_destory (hash);
        hash = object_hash_new ();
        /*刷新扫雷引擎地图*/
        mine_map_init (&map, _x, _y, mnum);
        g_print ("刷新了扫雷引擎地图\n");
        /*刷新首次点击状态*/
        first_click = 1;
        sprintf (opened_info, "0 / %d", mnum);
        gtk_label_set_label (opened_label, opened_info);

        for (int y = 0; y < _y; ++y) {
                for (int x = 0; x < _x; ++x) {
                        /*创建新的按钮，并为每个按钮连接信号*/
                        // button = test_mine_fbutton_new_with_position (c, r);
                        // 太难蚌了
                        button = gtk_button_new ();
                        gtk_widget_set_size_request (button, 35, 35);
                        /*为按钮连接鼠标信号，将按钮注册到哈希表*/
                        test_mine_field_connect_signal (button, x, y);
                        /*将按钮绑定到对应地雷方格*/
                        mine_set_gui_object_func (GOBJXY (map->field, x, y),
                                                  button,
                                                  test_mine_fresh_mine_state);
                        /*设置初始图片*/
                        image = gtk_image_new_from_resource (MINEA0);
                        gtk_button_set_child (GTK_BUTTON (button), image);
                        gtk_button_set_has_frame (GTK_BUTTON (button), FALSE);
                        gtk_grid_attach (grid, button, x, y, 1, 1);
                }
        }
        /*保存当前新行数以供释放*/
        field_row = _y;
}