/* test_mine-window.c
 *
 * Copyright 2023 korack
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test_mine-window.h"

#include "test_mine-config.h"
#include "test_mine-field.h"

static int now_grade;

struct _TestMineWindow
{
        GtkApplicationWindow parent_instance;

        /* Template widgets */
        GtkHeaderBar* header_bar;
        GtkBox*       main_box;
        GtkStack*     main_stack;
        GtkGrid*      choose_grid;
        GtkButton*    level_low_button;
        GtkButton*    level_middle_button;
        GtkButton*    level_high_button;
        GtkButton*    level_custom_button;
        GtkBox*       play_box;
        GtkGrid*      mines_field;
        GtkBox*       tool_box;
        GtkLabel*     tool_box_label;
        GtkLabel*     map_opened;
        GtkGrid*      info_grid;
        GtkButton*    reset_button;
        GtkButton*    change_button;
        GtkGrid*      button_grid;
        GtkLabel*     time_label;
};

G_DEFINE_TYPE (TestMineWindow, test_mine_window, GTK_TYPE_APPLICATION_WINDOW)

static void
test_mine_window_class_init (TestMineWindowClass* klass)
{
        GtkWidgetClass* widget_class = GTK_WIDGET_CLASS (klass);

        gtk_widget_class_set_template_from_resource (
                widget_class,
                "/org/example/testmine/test_mine-window.ui");
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              header_bar);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              main_box);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              main_stack);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              choose_grid);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              level_low_button);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              level_middle_button);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              level_high_button);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              level_custom_button);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              play_box);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              mines_field);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              tool_box);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              tool_box_label);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              map_opened);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              info_grid);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              reset_button);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              change_button);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              button_grid);
        gtk_widget_class_bind_template_child (widget_class,
                                              TestMineWindow,
                                              time_label);

        /*将 Label 绑定到时钟开启与时钟停止信号
        g_signal_new ("clock_start",
                      GTK_TYPE_LABEL,
                      G_SIGNAL_ACTION,
                      0,
                      NULL,
                      NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);
        g_signal_new ("clock_end",
                      GTK_TYPE_LABEL,
                      G_SIGNAL_ACTION,
                      0,
                      NULL,
                      NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);
        g_signal_new ("clock_suspend",
                      GTK_TYPE_LABEL,
                      G_SIGNAL_ACTION,
                      0,
                      NULL,
                      NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);
        g_signal_new ("clock_reset",
                      GTK_TYPE_LABEL,
                      G_SIGNAL_ACTION,
                      0,
                      NULL,
                      NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);*/
}

static void
test_mine_window_change (GAction* action     G_GNUC_UNUSED,
                         GVariant* parameter G_GNUC_UNUSED,
                         TestMineWindow*     self)
{
        static int status = 0;
        GtkStack*  main_stack =
                GTK_STACK (gtk_widget_get_template_child (GTK_WIDGET (self),
                                                          TEST_MINE_TYPE_WINDOW,
                                                          "main_stack"));
        gtk_stack_set_visible_child_name (
                main_stack,
                (status ? "choose_screen" : "play_screen"));
        status = !status;
}

static void
test_mine_window_low_game (GAction* action     G_GNUC_UNUSED,
                           GVariant* parameter G_GNUC_UNUSED,
                           TestMineWindow*     self)
{
        GObject* mines_field;
        now_grade = 0;
        test_mine_window_change (action, parameter, self);
        gtk_window_set_default_size (GTK_WINDOW (self), 510, 400);
        mines_field = gtk_widget_get_template_child (GTK_WIDGET (self),
                                                     TEST_MINE_TYPE_WINDOW,
                                                     "mines_field");
        test_mine_field_add_mines (self, GTK_GRID (mines_field), 8, 8, 10);
}

static void
test_mine_window_middle_game (GAction* action     G_GNUC_UNUSED,
                              GVariant* parameter G_GNUC_UNUSED,
                              TestMineWindow*     self)
{
        GObject* mines_field;
        now_grade = 1;
        test_mine_window_change (action, parameter, self);
        mines_field = gtk_widget_get_template_child (GTK_WIDGET (self),
                                                     TEST_MINE_TYPE_WINDOW,
                                                     "mines_field");
        test_mine_field_add_mines (self, GTK_GRID (mines_field), 16, 16, 40);
}

static void
test_mine_window_high_game (GAction* action     G_GNUC_UNUSED,
                            GVariant* parameter G_GNUC_UNUSED,
                            TestMineWindow*     self)
{
        GObject* mines_field;
        now_grade = 2;
        test_mine_window_change (action, parameter, self);
        mines_field = gtk_widget_get_template_child (GTK_WIDGET (self),
                                                     TEST_MINE_TYPE_WINDOW,
                                                     "mines_field");
        test_mine_field_add_mines (self, GTK_GRID (mines_field), 30, 16, 99);
}

static void
test_mine_window_custom_game (GAction* action      G_GNUC_UNUSED,
                              GVariant* parameter  G_GNUC_UNUSED,
                              TestMineWindow* self G_GNUC_UNUSED)
{
        /*用户指定雷数*/
}

static void
test_mine_window_reset_game (GAction* action     G_GNUC_UNUSED,
                             GVariant* parameter G_GNUC_UNUSED,
                             TestMineWindow*     self)
{
        GObject* mines_field;
        mines_field = gtk_widget_get_template_child (GTK_WIDGET (self),
                                                     TEST_MINE_TYPE_WINDOW,
                                                     "mines_field");
        switch (now_grade) {
        case 0:
                test_mine_field_add_mines (self,
                                           GTK_GRID (mines_field),
                                           8,
                                           8,
                                           10);
                break;
        case 1:
                test_mine_field_add_mines (self,
                                           GTK_GRID (mines_field),
                                           16,
                                           16,
                                           40);
                break;
        case 2:
                test_mine_field_add_mines (self,
                                           GTK_GRID (mines_field),
                                           30,
                                           16,
                                           99);
                break;
        default:
                break;
        }
}

#define CUSTOM_MACRO_A(name, func)                 \
        g_autoptr (GSimpleAction) name##_action =  \
                g_simple_action_new (#name, NULL); \
        g_signal_connect (name##_action,           \
                          "activate",              \
                          G_CALLBACK ((func)),     \
                          self);                   \
        g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (name##_action));

static void
test_mine_window_init (TestMineWindow* self)
{
        //GObject* _clock_label;
        gtk_widget_init_template (GTK_WIDGET (self));
        CUSTOM_MACRO_A (changes, test_mine_window_change);
        CUSTOM_MACRO_A (low_game, test_mine_window_low_game);
        CUSTOM_MACRO_A (middle_game, test_mine_window_middle_game);
        CUSTOM_MACRO_A (high_game, test_mine_window_high_game);
        CUSTOM_MACRO_A (custom_game, test_mine_window_custom_game);
        CUSTOM_MACRO_A (reset_game, test_mine_window_reset_game);

        /*绑定计时 Label 实例到信号函数
        _clock_label = gtk_widget_get_template_child (GTK_WIDGET (self),
                                                     TEST_MINE_TYPE_WINDOW,
                                                     "time_label");
        g_signal_connect (_clock_label,
                          "clock_start",
                          G_CALLBACK (test_mine_start_clock),
                          NULL);
        g_signal_connect (_clock_label,
                          "clock_start",
                          G_CALLBACK (test_mine_end_clock),
                          NULL);
        g_signal_connect (_clock_label,
                          "clock_start",
                          G_CALLBACK (test_mine_suspend_clock),
                          NULL);
        g_signal_connect (_clock_label,
                          "clock_reset",
                          G_CALLBACK (test_mine_reset_clock),
                          NULL);*/
}

#undef CUSTOM_MACRO_A
