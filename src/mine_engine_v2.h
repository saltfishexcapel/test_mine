#ifndef MINE_ENGINE_V2_H
#define MINE_ENGINE_V2_H

/*若为 Windows 系统，则注释掉本行即可*/
#define _MINE_ENGINE_USE_IN_UNIX

typedef unsigned int muint;
typedef int          mint;
typedef unsigned int mbool;
typedef double       mlf_clock;

#ifndef _MINE_ENGINE_USE_IN_UNIX
        #define _MINE_ENGINE_USE_IN_WIN
#endif
#ifdef _MINE_ENGINE_USE_IN_UNIX
        #include <sys/time.h>
typedef struct timeval MTimeVal;
        #define MINE_GET_TIME(_tm) gettimeofday (&(_tm), NULL);
        #define MINE_GET_TM_DIFF(_tmstart, _tmend)                     \
                ((mlf_clock)((_tmend).tv_sec - (_tmstart).tv_sec) +    \
                 ((mlf_clock)((_tmend).tv_usec - (_tmstart).tv_usec) / \
                  1000000.0))
#else
        #ifdef _MINE_ENGINE_USE_IN_WIN
                #include <window.h>
typedef SYSTEMTIME MTimeVal;
                #define MINE_GET_TIME(_tm) GetLocalTime (&(_tm))
                #define MINE_GET_TM_DIFF(_tmstart, _tmend)                    \
                        ((mlf_clock)((_tmend).wSecond - (_tmstart).wSecond) + \
                         (mlf_clock)((_tmend).wMilliseconds -                 \
                                     (_tmstart).wMilliseconds) /              \
                                 1000.0)
        #else
                #include <time.h>
typedef struct tm MTimeVal;
                #define MINE_GET_TIME(_tm) time (&(_tm))
                #define MINE_GET_TM_DIFF(_tmstart, _tmend) \
                        ((mlf_clock)((_tmend).tm_sec - (_tmstart).tm_sec))
        #endif
#endif

#define MTRUE             1
#define MFALSE            0

#define GOBJXY(obj, X, Y) (obj)[Y][X]
#define GOBJXYN(obj, X, Y, site, nsite)          \
        (GOBJXY (obj, X, Y)->site == NULL ? NULL \
                                          : GOBJXY (obj, X, Y)->site->nsite)

typedef enum
{
        MSUCCESS,
        MFAILED,
        MGAMING,
        MPROGERR,
} MStatus;

typedef enum
{
        MCLKL = 1,
        MCLKR = 0,
} MClick;

/*地雷方格对象*/
typedef struct _Mine Mine;
typedef Mine*        _mine_ptr;
struct _Mine
{
        /*上下左右的地雷方格对象*/
        Mine* up;
        Mine* down;
        Mine* left;
        Mine* right;
        /*围绕周围的雷数*/
        mint around;
        /*围绕周围的旗标数*/
        mint aflags;
        /*是否已经被打开？*/
        mbool is_opened;
        /*是否有旗标？*/
        mbool is_flaged;
        /*是否是雷？*/
        mbool is_mine;
        /*是否正在处理点击事件？为方格上锁*/
        mbool is_lock;
        /*图形按钮对象*/
        void* gui_object;
        /*图形按钮方法*/
        void (*gui_func) (Mine*);
};

/*构造器*/
Mine* mine_new ();
void  mine_destory (Mine*);
/*设置为地雷*/
void mine_set_mine (Mine*);
/*打开本地雷方格*/
void mine_open (Mine*);
/*旗标（调用后旗标状态相反）*/
void mine_flag (Mine*);
/*增加一个附近地雷计数*/
void mine_add_count (Mine*);
/*左键点击*/
MStatus mine_click_l (Mine*, mint* map_opened);
/*右键点击*/
void mine_click_r (Mine*, mint* map_flaged);
/*为地雷方格上锁*/
void mine_lock (Mine*);
/*为地雷方格解锁*/
void mine_unlock (Mine*);
/*设置上级图形界面 GUI 组件及方法*/
void mine_set_gui_object_func (Mine* m, void* gui_object, void* gui_func);

/*地雷周围附近的8个雷*/
#define MNEAR(m, site) ((m)->site)
#define MNEARN(m, site, nsite) \
        (MNEAR (m, site) == NULL ? NULL : MNEAR (m, site)->nsite)
#define MINE1(m) (MNEARN (m, left, up))
#define MINE2(m) (MNEAR (m, up))
#define MINE3(m) (MNEARN (m, up, right))
#define MINE4(m) (MNEAR (m, left))
#define MINE5(m) (MNEAR (m, right))
#define MINE6(m) (MNEARN (m, down, left))
#define MINE7(m) (MNEAR (m, down))
#define MINE8(m) (MNEARN (m, down, right))
/*点击雷 MINE_N，若触及到雷则返回 MFAILED*/
#define MIF_FAILED_RETURN(m, map_opened, MINE_N)                \
        if (mine_click_l (MINE_N (m), map_opened) == MFAILED) { \
                return MFAILED;                                 \
        }
/*如果 MINE_N 未被点开，则点开 MINE_N*/
#define MOPEN_IS_UNOPEN(m, map_opened, MINE_N)            \
        if (MINE_N (m) && !MINE_N (m)->is_opened) {       \
                MIF_FAILED_RETURN (m, map_opened, MINE_N) \
        }
/*点开周围未被打开的雷*/
#define MOPEN_AROUND_IS_UNOPEN(m, map_opened)   \
        MOPEN_IS_UNOPEN (m, map_opened, MINE1); \
        MOPEN_IS_UNOPEN (m, map_opened, MINE2); \
        MOPEN_IS_UNOPEN (m, map_opened, MINE3); \
        MOPEN_IS_UNOPEN (m, map_opened, MINE4); \
        MOPEN_IS_UNOPEN (m, map_opened, MINE5); \
        MOPEN_IS_UNOPEN (m, map_opened, MINE6); \
        MOPEN_IS_UNOPEN (m, map_opened, MINE7); \
        MOPEN_IS_UNOPEN (m, map_opened, MINE8);
/*更改 MINE_N 方格的旗标计数*/
#define MCHANGE_FLAGD(m, MINE_N, operate)         \
        if (MINE_N (m)) {                         \
                MINE_N (m)->aflags operate## = 1; \
        }
/*更改周围方格的旗标计数*/
#define MCHANGE_AROUND_FLAGD(m, operate)   \
        MCHANGE_FLAGD (m, MINE1, operate); \
        MCHANGE_FLAGD (m, MINE2, operate); \
        MCHANGE_FLAGD (m, MINE3, operate); \
        MCHANGE_FLAGD (m, MINE4, operate); \
        MCHANGE_FLAGD (m, MINE5, operate); \
        MCHANGE_FLAGD (m, MINE6, operate); \
        MCHANGE_FLAGD (m, MINE7, operate); \
        MCHANGE_FLAGD (m, MINE8, operate);
/*执行单个地雷方格的 GUI 方法*/
#define MRUN_GUI_FUNC(_m)                         \
        if ((_m)->gui_func && (_m)->gui_object) { \
                (_m)->gui_func (_m);              \
        }

/*地图对象*/
typedef struct _MineMap MineMap;
struct _MineMap
{
        /*地图长和宽*/
        mint _x;
        mint _y;
        /*地雷数量*/
        mint m;
        /*旗标数量*/
        mint flag;
        /*已打开的雷区数量*/
        mint opened;
        /*游戏是否结束？*/
        mbool is_over;
        /*游戏开始的时间戳*/
        MTimeVal tm_start;
        /*游戏结束的时间戳*/
        MTimeVal tm_end;
        /*雷池*/
        _mine_ptr** field;
};

/*构造器*/
MineMap* mine_map_new ();
void     mine_map_destory (MineMap* map);

/*游戏初始化*/
void mine_map_init (MineMap** map, mint _x, mint _y, mint m);
/*首次点击*/
void mine_map_first_click (MineMap* map, mint clkx, mint clky);
/*对地图清零*/
void mine_map_reset (MineMap* map);
/*仅用于设置基本信息*/
void mine_map_init_set (MineMap* map, mint x, mint y, mint m);
/*仅用于根据信息分配雷区内存*/
void mine_map_alloc_map (MineMap* map);
/*仅用于关联雷区(雷之间的连接)*/
void mine_map_related_mines (MineMap* map);
/*仅用于安放地雷：在首次点击之后*/
void mine_map_put_mines (MineMap* map, mint clkx, mint clky);
/*仅用于计算全局地雷方格雷数*/
void mine_map_count_quantity (MineMap* map);
/*开始计时*/
void mine_map_start_clock (MineMap* map);
/*结束计时*/
mlf_clock mine_map_end_clock (MineMap* map);

/*获取地雷方格对象*/
Mine* mine_map_get_mine (MineMap* map, mint x, mint y);

/*点击某个地雷方格，并获知游戏状态（游戏成功或失败，或者是进行当中）*/
MStatus mine_map_click (MineMap* map, mint clkx, mint clky, MClick clk);

/*判断游戏是否胜利*/
MStatus mine_map_game_is_win (MineMap* map);

#endif