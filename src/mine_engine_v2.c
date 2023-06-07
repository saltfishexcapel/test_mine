#include "mine_engine_v2.h"

#include <stdlib.h>
#include <time.h>

inline Mine*
mine_new ()
{
        Mine* m;
        m            = (Mine*)malloc (sizeof (Mine));

        m->up        = NULL;
        m->down      = NULL;
        m->left      = NULL;
        m->right     = NULL;

        m->around    = 0;
        m->aflags    = 0;

        m->is_opened = MFALSE;
        m->is_flaged = MFALSE;
        m->is_mine   = MFALSE;
        m->is_lock   = MFALSE;

        /*图形界面对象*/
        m->gui_object = NULL;
        m->gui_func   = NULL;
        return m;
}

inline void
mine_destory (Mine* m)
{
        if (m != NULL)
                free (m);
}

void
mine_set_mine (Mine* m)
{
        if (m != NULL)
                m->is_mine = MTRUE;
}

void
mine_open (Mine* m)
{
        if (m != NULL)
                m->is_opened = MTRUE;
}

void
mine_flag (Mine* m)
{
        if (m == NULL)
                return;
        /*是否已打开？*/
        if (m->is_opened)
                return;
        /*此前未有旗标*/
        if (!m->is_flaged) {
                /*此处循环改变周围的旗数*/
                MCHANGE_AROUND_FLAGD (m, +);

        } else {
                MCHANGE_AROUND_FLAGD (m, -);
        }
        m->is_flaged = !m->is_flaged;
}

inline void
mine_add_count (Mine* m)
{
        if (m != NULL && !m->is_mine)
                m->around += 1;
}

MStatus
mine_click_l (Mine* m, mint* map_opened)
{
        /* 地雷方格收到了一个左单击事件，
         *
         * 在方格未打开的情况下，
         * 若该方格是雷，点开则游戏失败；
         * 若该方格是数字，则只打开该方格本身；
         * 若该方格为空，则点开周围未点开的方格；
         *
         * 在方格有旗标的情况下，左键点击不进行任何操作；
         *
         * 在方格已经打开的情况下，
         * 若该方格为空，则不进行任何操作；
         * 若该方格为数字，则打开周围所有非旗标的未开方格；
         * */
        if (m == NULL)
                return MPROGERR;

        if (m->is_lock)
                return MGAMING;
        mine_lock (m);
        /*该方格有旗标，不进行任何操作*/
        if (m->is_flaged) {
                mine_unlock (m);
                return MGAMING;
        }
        /*该方格已打开*/
        else if (m->is_opened) {
                /*周围有雷：该方格为数字，如果周围旗标数大于等于雷数，则打开周围方格*/
                if (m->around && m->aflags >= m->around) {
                        MOPEN_AROUND_IS_UNOPEN (m, map_opened);
                        /*周围无雷，不进行任何操作*/
                } else {
                        mine_unlock (m);
                        return MGAMING;
                }
                /*该方格未打开*/
        } else {
                /*打开本方格*/
                mine_open (m);
                *map_opened += 1;
                /*点到雷，失败*/
                if (m->is_mine) {
                        MRUN_GUI_FUNC (m);
                        return MFAILED;
                        /*点开周围没点开的地雷方格*/
                } else if (!m->around) {
                        MOPEN_AROUND_IS_UNOPEN (m, map_opened);
                }
        }
        MRUN_GUI_FUNC (m);
        mine_unlock (m);
        return MGAMING;
}

void
mine_lock (Mine* m)
{
        if (m != NULL)
                m->is_lock = MTRUE;
}

void
mine_unlock (Mine* m)
{
        if (m != NULL)
                m->is_lock = MFALSE;
}

void
mine_click_r (Mine* m, mint* flaged)
{
        if (m != NULL) {
                mine_flag (m);
                if (m->is_flaged)
                        *flaged += 1;
                else
                        *flaged -= 1;

                MRUN_GUI_FUNC (m);
        }
}

void
mine_set_gui_object_func (Mine* m, void* gui_object, void* gui_func)
{
        if (m == NULL)
                return;
        m->gui_func   = gui_func;
        m->gui_object = gui_object;
}

MineMap*
mine_map_new ()
{
        MineMap* map;
        map          = (MineMap*)malloc (sizeof (MineMap));
        map->_x      = 0;
        map->_y      = 0;
        map->m       = 0;
        map->flag    = 0;
        map->opened  = 0;
        map->field   = NULL;
        map->is_over = MFALSE;
        return map;
}

void
mine_map_destory (MineMap* map)
{
        if (map == NULL)
                return;
        mine_map_reset (map);
        free (map);
}

void
mine_map_init (MineMap** map, mint _x, mint _y, mint m)
{
        if (*map == NULL)
                *map = mine_map_new ();
        mine_map_reset (*map);
        mine_map_init_set (*map, _x, _y, m);
        mine_map_alloc_map (*map);
        mine_map_related_mines (*map);
}

void
mine_map_first_click (MineMap* map, mint clkx, mint clky)
{
        mine_map_put_mines (map, clkx, clky);
        mine_map_count_quantity (map);
        mine_map_start_clock (map);
        mine_map_click (map, clkx, clky, MCLKL);
}

void
mine_map_reset (MineMap* map)
{
        if (map == NULL || map->field == NULL)
                return;
        for (int i = 0; i < map->_y; ++i) {
                for (int j = 0; j < map->_x; ++j)
                        mine_destory (GOBJXY (map->field, j, i));
                free (map->field[i]);
        }
        free (map->field);
        map->_x      = 0;
        map->_y      = 0;
        map->m       = 0;
        map->flag    = 0;
        map->opened  = 0;
        map->field   = NULL;
        map->is_over = MFALSE;
}

void
mine_map_init_set (MineMap* map, mint _x, mint _y, mint m)
{
        if (map == NULL)
                return;
        map->_x     = _x;
        map->_y     = _y;
        map->m      = m;
        map->opened = 0;
        map->flag   = 0;
}

void
mine_map_alloc_map (MineMap* map)
{
        if (map == NULL)
                return;
        map->field = (_mine_ptr**)malloc (sizeof (_mine_ptr*) * map->_y);
        for (int i = 0; i < map->_y; ++i) {
                map->field[i] =
                        (_mine_ptr*)malloc (sizeof (_mine_ptr) * map->_x);
                for (int j = 0; j < map->_x; ++j)
                        map->field[i][j] = mine_new ();
        }
}

/*横纵地雷方格间绑定*/
#define MRELATEDLR(m1, m2)        \
        if (m1 && m2) {           \
                (m1)->right = m2; \
                (m2)->left  = m1; \
        }
#define MRELATEDUD(m1, m2)       \
        if (m1 && m2) {          \
                (m1)->down = m2; \
                (m2)->up   = m1; \
        }

void
mine_map_related_mines (MineMap* map)
{
        if (map == NULL || map->field == NULL)
                return;
        /*横向绑定*/
        for (int y = 0; y < map->_y; ++y) {
                for (int x = 0; x < map->_x - 1; ++x) {
                        MRELATEDLR (GOBJXY (map->field, x, y),
                                    GOBJXY (map->field, x + 1, y));
                }
        }
        /*纵向绑定*/
        for (int x = 0; x < map->_x; ++x) {
                for (int y = 0; y < map->_y - 1; ++y) {
                        MRELATEDUD (GOBJXY (map->field, x, y),
                                    GOBJXY (map->field, x, y + 1));
                }
        }
}

#undef MRELATEDLR
#undef MRELATEDUD

void
mine_map_put_mines (MineMap* map, mint clkx, mint clky)
{
        mint  mined = 0, seted = 0, dnum, m_rands;
        mbool always_mine = 0, will_mined = 0;
        if (map == NULL)
                return;
        dnum    = map->_x * map->_y;
        m_rands = dnum / map->m;
        srand ((muint)time (NULL));
        for (int y = 0; y < map->_y; ++y) {
                for (int x = 0; x < map->_x; ++x) {
                        /*当剩余区域数等于剩余地雷数时总是雷*/
                        if (dnum - seted <= (map->m) - mined)
                                always_mine = MTRUE;
                        /*当无剩余雷数时退出布雷*/
                        if ((map->m) - mined == 0)
                                return;
                        will_mined = (!(rand () % m_rands) || always_mine);
                        if ((x >= clkx - 1 && x <= clkx + 1) &&
                            (y >= clky - 1 && y <= clky + 1))
                                will_mined = MFALSE;
                        if (will_mined) {
                                mine_set_mine (GOBJXY (map->field, x, y));
                                ++mined;
                        }
                        ++seted;
                }
        }
}

void
mine_map_count_quantity (MineMap* map)
{
        if (map == NULL || map->field == NULL)
                return;
        for (int y = 0; y < map->_y; ++y) {
                for (int x = 0; x < map->_x; ++x) {
                        if (!GOBJXY (map->field, x, y)->is_mine)
                                continue;
                        mine_add_count (MINE1 (GOBJXY (map->field, x, y)));
                        mine_add_count (MINE2 (GOBJXY (map->field, x, y)));
                        mine_add_count (MINE3 (GOBJXY (map->field, x, y)));
                        mine_add_count (MINE4 (GOBJXY (map->field, x, y)));
                        mine_add_count (MINE5 (GOBJXY (map->field, x, y)));
                        mine_add_count (MINE6 (GOBJXY (map->field, x, y)));
                        mine_add_count (MINE7 (GOBJXY (map->field, x, y)));
                        mine_add_count (MINE8 (GOBJXY (map->field, x, y)));
                }
        }
}

void
mine_map_start_clock (MineMap* map)
{
        if (map == NULL)
                return;
        MINE_GET_TIME (map->tm_start);
}

mlf_clock
mine_map_end_clock (MineMap* map)
{
        if (map == NULL)
                return 0.0;
        MINE_GET_TIME (map->tm_end);
        return MINE_GET_TM_DIFF (map->tm_start, map->tm_end);
}

Mine*
mine_map_get_mine (MineMap* map, mint x, mint y)
{
        if (map != NULL || map->field != NULL)
                return GOBJXY (map->field, x, y);
        return NULL;
}

MStatus
mine_map_click (MineMap* map, mint clkx, mint clky, MClick clk)
{
        Mine*   m;
        MStatus status;
        if (map == NULL)
                return MPROGERR;
        if (map->is_over)
                return MGAMING;
        m = GOBJXY (map->field, clkx, clky);
        if (clk == MCLKR) {
                mine_click_r (m, &(map->flag));
                return MGAMING;
        } else {
                status = mine_click_l (m, &(map->opened));
                if (status == MGAMING &&
                    mine_map_game_is_win (map) == MSUCCESS) {
                        map->is_over = MTRUE;
                        mine_map_end_clock (map);
                        return MSUCCESS;
                } else if (status == MFAILED) {
                        mine_map_end_clock (map);
                        map->is_over = MTRUE;
                        return MFAILED;
                } else {
                        return status;
                }
        }
}

MStatus
mine_map_game_is_win (MineMap* map)
{
        if (map == NULL)
                return MPROGERR;
        /*总数 == 已打开 + 雷数 即为胜利*/
        if (map->_x * map->_y == map->opened + map->m)
                return MSUCCESS;
        else
                return MGAMING;
}