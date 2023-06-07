/* main.c
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

#include "test_mine-application.h"
#include "test_mine-config.h"
#include "object-hash.h"
#include "test_mine-field.h"

#include <glib/gi18n.h>

int
main (int argc, char* argv[])
{
        g_autoptr (TestMineApplication) app = NULL;
        int ret;

        /* Set up gettext translations */
        bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);
        /*
         * Create a new GtkApplication. The application manages our main loop,
         * application windows, integration with the window manager/compositor,
         * and desktop features such as file opening and single-instance
         * applications.
         */
        app = test_mine_application_new ("org.example.testmine",
                                         G_APPLICATION_FLAGS_NONE);

        /*
         * Run the application. This function will block until the application
         * exits. Upon return, we have our exit code to return to the shell.
         * (This is the code you see when you do `echo $?` after running a
         * command in a terminal.
         *
         * Since GtkApplication inherits from GApplication, we use the parent
         * class method "run". But we need to cast, which is what the
         * "G_APPLICATION()" macro does.
         */
        ret = g_application_run (G_APPLICATION (app), argc, argv);

        object_hash_destory (hash);
        mine_map_destory (map);

        return ret;
}
