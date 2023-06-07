#ifndef MINES_ENGINE_H
#define MINES_ENGINE_H

// #define OS_TYPE_WIN
#define OS_TYPE_UNIX

typedef struct _MineMap MineMap;
typedef enum _MapStatus mstatus;
typedef enum _MapActive mactive;

/*地图对象结构*/
struct _MineMap
{
        int    length; /*地图长*/
        int    width;  /*地图宽*/
        int    mnum;   /*地雷数量*/
        int    opnum;  /*开启方格数量*/
        int    ymine;  /*剩余地雷数量*/
        char** mstate; /*地雷在地图上的分布*/
};

/*地图每一格的状态，雷数或雷或无雷*/
enum _MapStatus
{
        MOVERFAIL = 36, /*游戏失败*/
        MOVERTRUE = 37, /*游戏成功*/
        MTRUE     = 39, /*普通返回*/
        MFAIL     = 40, /*输入参数错误*/

        MNONE     = 0,
        M1        = 1,
        M2        = 2,
        M3        = 3,
        M4        = 4,
        M5        = 5,
        M6        = 6,
        M7        = 7,
        M8        = 8,
        MMINE     = 9,

        MOPNONE   = 10,
        MOP1      = 11,
        MOP2      = 12,
        MOP3      = 13,
        MOP4      = 14,
        MOP5      = 15,
        MOP6      = 16,
        MOP7      = 17,
        MOP8      = 18,
        MOPMINE   = 19,

        MFLNONE   = 20,
        MFL1      = 21,
        MFL2      = 22,
        MFL3      = 23,
        MFL4      = 24,
        MFL5      = 25,
        MFL6      = 26,
        MFL7      = 27,
        MFL8      = 28,
        MFLMINE   = 29,
};

/*模拟左键或右键点击*/
enum _MapActive
{
        MCLICKR = 0,
        MCLICKL = 1,
};

/*初始化一张地图，分配内存*/
extern int mines_engine_init_map (MineMap* map_obj,
                                  int      mnum, /*地雷数量*/
                                  int      length,
                                  int      width);

/*根据第一次点击建立地雷图*/
extern int mines_engine_init_mines (MineMap* map_obj, int click_l, int click_w);

/*释放内存*/
extern int mines_engine_destory_map (MineMap* map_obj);

/*点击或插旗*/
extern mstatus
mines_engine_click (MineMap* map_obj, mactive active, int length, int width);

#endif