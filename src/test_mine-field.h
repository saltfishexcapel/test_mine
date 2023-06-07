#ifndef TEST_MINE_FIELD_H
#define TEST_MINE_FIELD_H

#include "mine_engine_v2.h"
#include "object-hash.h"
#include "test_mine-window.h"

extern void     test_mine_field_remove_old_field (GtkGrid* grid);

extern void     test_mine_field_add_mines (TestMineWindow* self,
                                           GtkGrid*        grid,
                                           int             _x,
                                           int             _y,
                                           int             mnum);

extern MineMap* map;

// #define MINE_IMGA0
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/waitopen.png"
// #define MINE_IMGB0
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/opened.png"
// #define MINE_IMGB1
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/1mines.svg"
// #define MINE_IMGB2
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/2mines.svg"
// #define MINE_IMGB3
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/3mines.svg"
// #define MINE_IMGB4
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/4mines.svg"
// #define MINE_IMGB5
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/5mines.svg"
// #define MINE_IMGB6
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/6mines.svg"
// #define MINE_IMGB7
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/7mines.svg"
// #define MINE_IMGB8
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/8mines.svg"
// #define MINE_IMGBM
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/exploded.svg"
// #define MINE_IMGCF
// "/home/korack/programtest/testdir/c_c++/test_mine/data/picture/flag.svg"

#define MINEA0 "/org/example/testmine/../data/picture/waitopen.png"
#define MINEB0 "/org/example/testmine/../data/picture/opened.png"
#define MINEB1 "/org/example/testmine/../data/picture/1mines.svg"
#define MINEB2 "/org/example/testmine/../data/picture/2mines.svg"
#define MINEB3 "/org/example/testmine/../data/picture/3mines.svg"
#define MINEB4 "/org/example/testmine/../data/picture/4mines.svg"
#define MINEB5 "/org/example/testmine/../data/picture/5mines.svg"
#define MINEB6 "/org/example/testmine/../data/picture/6mines.svg"
#define MINEB7 "/org/example/testmine/../data/picture/7mines.svg"
#define MINEB8 "/org/example/testmine/../data/picture/8mines.svg"
#define MINEBM "/org/example/testmine/../data/picture/exploded.svg"
#define MINECF "/org/example/testmine/../data/picture/flag.svg"

extern void* test_mine_start_clock (void*);
extern void test_mine_end_clock ();
extern void test_mine_suspend_clock (GtkWidget* widget, gpointer data);
extern void test_mine_reset_clock ();

#endif