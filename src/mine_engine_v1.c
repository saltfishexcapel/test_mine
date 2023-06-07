#include "mine_engine_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*初始化一张地图，分配内存*/
int
mines_engine_init_map (MineMap* map_obj,
                       int      mnum, /*地雷数量*/
                       int      length,
                       int      width)
{
        if (map_obj == NULL)
                return 1;

        /*拷贝数据*/
        map_obj->mnum   = mnum;
        map_obj->ymine  = mnum;
        map_obj->length = length;
        map_obj->width  = width;
        map_obj->opnum  = 0;
        /*分配纵向空间*/
        map_obj->mstate = (char**)malloc (width * sizeof (char*));
        /*循环分配横向空间*/
        for (int i = 0; i < width; ++i) {
                map_obj->mstate[i] = (char*)malloc (length * sizeof (char));
                for (int j = 0; j < length; ++j)
                        map_obj->mstate[i][j] = MNONE;
        }
        return 0;
}

/*计算雷附近非雷方格的雷数*/
static int
mines_engine_near_mines (char** map, int mapw, int mapl, int w, int l)
{
        for (int i = (w - 1 < 0 ? 0 : w - 1);
             i <= (w + 1 >= mapw ? mapw - 1 : w + 1);
             ++i) {
                for (int j = (l - 1 < 0 ? 0 : l - 1);
                     j <= (l + 1 >= mapl ? mapl - 1 : l + 1);
                     ++j) {
                        if (map[i][j] != MMINE)
                                map[i][j] += 1;
                }
        }
        return 0;
}

/*根据第一次点击建立地雷图*/
int
mines_engine_init_mines (MineMap* map_obj, int click_l, int click_w)
{
        int awmines = 0, /*总是雷开关，默认关闭*/
                mrans,   /*随机大数*/
                umnum,   /*剩余的区域数量*/
                ymnum;   /*未安放的雷数量*/
        if (map_obj == NULL)
                return 1;

        /*区域总数：长乘以宽*/
        umnum = (map_obj->length) * (map_obj->width);
        ymnum = map_obj->mnum;
        /*设定随机种子*/
        srand ((unsigned int)time (NULL));
        /*计算随机大数*/
        mrans = umnum / (map_obj->mnum);
        mrans = (mrans < 1 ? 1 : mrans);

        /*循环安置地雷*/
        for (int w = 0; w < map_obj->width; ++w) {
                for (int l = 0; l < map_obj->length; ++l) {
                        /*无雷可安放就跳出*/
                        if (ymnum == 0)
                                break;

                        /*总是雷开关，当雷数等于剩余格数时剩下格数总是雷*/
                        if (awmines == 1) {
                                map_obj->mstate[w][l] = MMINE;
                                /*首次点击的格附近八个不生成雷*/
                        } else if ((w >= click_w - 1 && w <= click_w + 1) &&
                                   (l >= click_l - 1 && l <= click_l)) {
                                map_obj->mstate[w][l] = MNONE;
                                /*每个区域有 1 / mrans 的几率生成雷*/
                        } else if (rand () % mrans == 0) {
                                map_obj->mstate[w][l] = MMINE;
                                ymnum -= 1;
                                /*不生成雷的地方置 MNONE*/
                        } else {
                                map_obj->mstate[w][l] = MNONE;
                        }
                        umnum -= 1;

                        /*检查剩余雷数与剩余区域数的情况*/
                        if (umnum <= ymnum)
                                awmines = 1;
                }
        }

        /*循环计算格中数字*/
        for (int w = 0; w < map_obj->width; ++w) {
                for (int l = 0; l < map_obj->length; ++l) {
                        /*若是雷，则增加周围非雷区域的雷计数*/
                        if (map_obj->mstate[w][l] == MMINE) {
                                mines_engine_near_mines (map_obj->mstate,
                                                         map_obj->width,
                                                         map_obj->length,
                                                         w,
                                                         l);
                        }
                }
        }
        /*打开第一次点击的区域*/
        mines_engine_click (map_obj, MCLICKL, click_l, click_w);
        return 0;
}

/*释放内存*/
int
mines_engine_destory_map (MineMap* map_obj)
{
        if ((map_obj == NULL) || (map_obj->mstate == NULL))
                return 1;

        for (int i = 0; i < map_obj->width; ++i)
                free (map_obj->mstate[i]);

        free (map_obj->mstate);
        map_obj->mstate = NULL;
        return 0;
}

/*判断点击区域附近有无未打开且未标旗的雷，返回 1 则提示调用者可以打开周围的雷*/
static int
mines_engine_can_open (char** map, int mapw, int mapl, int w, int l)
{
        for (int i = (w - 1 < 0 ? 0 : w - 1);
             i <= (w + 1 >= mapw ? mapw - 1 : w + 1);
             ++i) {
                for (int j = (l - 1 < 0 ? 0 : l - 1);
                     j <= (l + 1 >= mapl ? mapl - 1 : l + 1);
                     ++j) {
                        if (map[i][j] == MMINE)
                                return 0;
                }
        }
        return 1;
}

/*开启非雷的递归方式*/
static int
mines_engine_open (char** map, int mapw, int mapl, int w, int l)
{
        int num = 0, ret;
        /*若当前方格未开启*/
        if (map[w][l] <= MMINE)
                map[w][l] += 10;

        ret = mines_engine_can_open (map, mapw, mapl, w, l);
        for (int i = (w - 1 < 0 ? 0 : w - 1);
             i <= (w + 1 >= mapw ? mapw - 1 : w + 1);
             ++i) {
                for (int j = (l - 1 < 0 ? 0 : l - 1);
                     j <= (l + 1 >= mapl ? mapl - 1 : l + 1);
                     ++j) {
                        /*空白非雷区，判断附近是否有未打开的雷*/
                        if (map[i][j] == MNONE && ret) {
                                num += mines_engine_open (map,
                                                          mapw,
                                                          mapl,
                                                          i,
                                                          j);
                                num += 1;
                        } else if (map[i][j] == MNONE && !ret) {
                                printf ("在 map[%d][%d] "
                                        "遇到了不可开雷的情况.\n",
                                        i,
                                        j);
                                map[i][j] += 10;
                                num += 1;
                        } else if (map[i][j] < MMINE) {
                                /*开启数字区*/
                                map[i][j] += 10;
                                num += 1;
                        }
                }
        }
        return num;
}

/*点击或插旗*/
mstatus
mines_engine_click (MineMap* map_obj, mactive active, int cl, int cw)
{
        if (map_obj == NULL) {
                printf ("[ERROR] 错误！地图对象为空。事件坐标<%d, "
                        "%d>，%s键点击\n",
                        cw,
                        cl,
                        (active == MCLICKL ? "左" : "右"));
                return 3;
        }
        char** ms = map_obj->mstate;

        if (map_obj == NULL)
                return MFAIL;

        /*左键相关操作*/
        if (active == MCLICKL) {
                /*左键点到插旗的地方，或已开启的无雷区*/
                if ((ms[cw][cl] >= MFLNONE) || (ms[cw][cl] == MOPNONE)) {
                        return MTRUE; /*无响应*/
                        /*左键点到未开启的雷*/
                } else if (ms[cw][cl] == MMINE) {
                        ms[cw][cl] = MOPMINE; /*雷爆炸了*/
                        return MOVERFAIL;     /*游戏失败*/
                        /*左键点到未开启的非雷区*/
                } else if (ms[cw][cl] < MMINE) {
                        /*opnum 加上开启数（包括递归的部分），然后交给下一级
                        逻辑判断是否扫雷完成*/
                        map_obj->opnum += mines_engine_open (ms,
                                                             map_obj->width,
                                                             map_obj->length,
                                                             cw,
                                                             cl);
                        map_obj->opnum += 1; /*加上点到的格*/
                        /*左键点到开启的数字区，若周围旗标数小于本数则无反应，若旗标数大于等
                        于本数则开启周围未开启方格*/
                } else {
                        map_obj->opnum += mines_engine_open (ms,
                                                             map_obj->width,
                                                             map_obj->length,
                                                             cw,
                                                             cl);
                }
                /*右键相关操作*/
        } else {
                /*右键点到未开启的地方*/
                if (ms[cw][cl] <= MMINE) {
                        ms[cw][cl] += 20; /*该方格转为插旗态*/
                        return MTRUE;
                        /*右键点到插旗的地方*/
                } else if (ms[cw][cl] >= MFLNONE) {
                        ms[cw][cl] -= 20;    /*该方格转为未开启态*/
                        map_obj->ymine -= 1; /*剩余地雷数量*/
                        return MTRUE;
                        /*右键点到已开启的地方*/
                } else {
                        return MTRUE; /*无反应*/
                }
        }

        /*判断雷是否开完*/
        if (map_obj->mnum + map_obj->opnum == map_obj->length * map_obj->width)
                return MOVERTRUE; /*游戏胜利*/
        else
                return MTRUE;
}
