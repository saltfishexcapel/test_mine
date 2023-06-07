#ifndef MINE_ENGINE_V2_H
#define MINE_ENGINE_V2_H

/*扫雷二代引擎*/

/*若为 Windows 系统，则注释掉本行即可*/
#define _MINE_ENGINE_USE_IN_UNIX

typedef unsigned int muint;
typedef int          mint;
typedef unsigned int mbool;
typedef double       mlf_clock;

/*关于时间戳的定义，不同系统下有不同方法*/
#ifndef _MINE_ENGINE_USE_IN_UNIX
        #define _MINE_ENGINE_USE_IN_WIN
#endif
#ifdef _MINE_ENGINE_USE_IN_UNIX
        #include <sys/time.h>
/*微秒计时结构*/
typedef struct timeval MTimeVal;
        /**
         * 获取当前微秒时到 _tm
         * @_tm: 精确计时结构
         */
        #define MINE_GET_TIME(_tm) gettimeofday (&(_tm), NULL);
        /*两个微秒时相减，返回单位：秒*/
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

/**
 * 布尔量
 */
#define MTRUE  1
#define MFALSE 0

/**
 * 由于访问网格横纵坐标时，可能会把 x, y 顺序搞混淆，固定使用 GOBJXY ()
 * 来访问一切二维结构
 * @obj: 二维对象
 * @x: 二维对象的 X 坐标
 * @y: 二维对象的 Y 坐标
 *
 */
#define GOBJXY(obj, X, Y) (obj)[Y][X]
/**
 * GOBXYN () 是个方向宏，与 struct _Mine
 * 联合使用，表示一个地雷方格连续两个方向的方格对象，如果其不存在则返回 NULL
 */
#define GOBJXYN(obj, X, Y, site, nsite)          \
        (GOBJXY (obj, X, Y)->site == NULL ? NULL \
                                          : GOBJXY (obj, X, Y)->site->nsite)

/**
 * 程序返回状态
 * @MSUCCESS: 游戏成功
 * @MFAILED: 游戏失败
 * @GAMING: 游戏中
 * @MPROGERR: 程序错误
 */
typedef enum
{
        MSUCCESS,
        MFAILED,
        MGAMING,
        MPROGERR,
} MStatus;

/**
 * @MCLKL: 左键点击
 * @MCLKR: 右键点击
 */
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

/**
 * 地雷方格对象的构造器
 * @return: 返回一个新的地雷方格对象
 */
Mine* mine_new ();

/**
 * 地雷方格对象的销毁器
 * @m: 需要销毁的地雷方格对象
 */
void mine_destory (Mine* m);

/**
 * 设置地雷方格对象为地雷
 * @m: 需要设置为雷的地雷方格
 */
void mine_set_mine (Mine* m);

/**
 * 打开本地雷方格
 * @m: 需要打开的地雷方格
 */
void mine_open (Mine* m);

/**
 * 旗标（调用后旗标状态相反）
 * @m: 需要设置为旗标方格的方格
 */
void mine_flag (Mine* m);

/**
 * 增加一个附近地雷计数：当某个地雷方格设置为地雷的时候，其将为周围的地雷方格使用该方法。
 * m->around 就是本地雷方格周围的地雷数量
 * @m: 需要增加一个周围雷计数的地雷方格
 */
void mine_add_count (Mine* m);

/**
 * 左键点击一个地雷方格
 * @m: 被左键点击的地雷方格
 * @map_opened: MineMap* 对象的参数 opened，将更新已打开的地雷方格数量
 * @return: 将返回点击后的状态（如是否点到雷，是否游戏成功）
 */
MStatus mine_click_l (Mine* m, mint* map_opened);

/**
 * 右键点击一个地雷方格
 * @m: 被右键点击的地雷方格
 * @map_flaged: MineMap* 对象的参数 map_flaged，将更新已标记的旗标数量
 */
void mine_click_r (Mine* m, mint* map_flaged);

/**
 * 为地雷方格上锁：当某地雷方格调用左点击方法时，通常会递归地使周围方格
 * 也调用左点击方法。为了防止两个相邻地雷方格互相调用对方的左点击方法，
 * （会造成死循环导致栈溢出），在地雷方格对象被左点击时将会为自身上一把
 * “锁”，从而防止死循环的产生。
 * @m: 需要上锁的地雷方格对象
 */
void mine_lock (Mine* m);

/**
 * 为地雷方格解锁：某方格对象递归调用完周围的方格对象后，死锁现象解除，
 * 即可解锁。
 * @m: 需要解锁的地雷方格对象
 */
void mine_unlock (Mine*);

/**
 * 设置上级图形界面 GUI 组件及方法。地雷方格产生变化时，GUI 也应该产生
 * 对应的变化。本引擎将每个地雷方格单独抽象成为了独立的对象，产生这些变化
 * 时，可由地雷方格自身来改变上层对应 GUI 部件的状态，从而避免对上层 GUI
 * 整体性刷新（对于 GTK 来说全盘刷新地雷状态效率是不高的）。
 * @m: 需要关联的地雷方格
 * @gui_object: 需要关联的 GUI 组件（如某个按钮）
 * @gui_func: 需要关联的 GUI 方法，这个方法需要自己编写。格式为 void func_name
 * (Mine* m)
 */
void
mine_set_gui_object_func (Mine* m, void* gui_object, void (*gui_func) (Mine*));

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

/*地图对象，包含地雷方格子对象*/
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

/*地图对象的构造器*/
MineMap* mine_map_new ();

/*地图对象的销毁器（程序结束时必须调用此方法来释放内存）*/
void     mine_map_destory (MineMap* map);

/*引擎初始化*/
void mine_map_init (MineMap** map, mint _x, mint _y, mint m);

/*对于某方格的首次点击，将根据首次点击随机生成地雷*/
void mine_map_first_click (MineMap* map, mint clkx, mint clky);

/*对地图清零，会安全释放之前分配的二维雷图*/
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

/*结束计时，返回游戏总时长*/
mlf_clock mine_map_end_clock (MineMap* map);

/*在 x y 坐标获取地雷方格对象*/
Mine* mine_map_get_mine (MineMap* map, mint x, mint y);

/*点击某个地雷方格，并获知游戏状态（游戏成功或失败，或者是进行当中）*/
MStatus mine_map_click (MineMap* map, mint clkx, mint clky, MClick clk);

/*判断游戏是否胜利*/
MStatus mine_map_game_is_win (MineMap* map);

#endif