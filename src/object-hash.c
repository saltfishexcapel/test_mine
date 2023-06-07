#include "object-hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ObjectHash* hash = NULL;

static uint old_size;

/*比较两对值是否相等，若相等为 1，不相等为 0*/
static uint object_hash_compare (void* p_key1, void* p_key2);

kvptr
object_kv_new ()
{
        kvptr kv  = (kvptr)malloc (sizeof (ObjectKV));
        kv->p_key = NULL;
        kv->next  = NULL;
        kv->v_x   = 0;
        kv->v_y   = 0;
        return kv;
}

/**
 * 请注意！！！本函数获得了 ckey 的释放权限！
 */
void
object_kv_set_data (kvptr kv, void* p_key, const uint x, const uint y)
{
        if (kv == NULL)
                return;
        kv->p_key = p_key;
        kv->v_x   = x;
        kv->v_y   = y;
}

/**
 * 请注意！！本函数被移交 ckey 的释放权限！
 */
kvptr
object_kv_new_with (void* p_key, const uint x, const uint y)
{
        kvptr kv = object_kv_new ();
        /**
         * ckey 的释放权限移交到了下级函数
         */
        object_kv_set_data (kv, p_key, x, y);
        return kv;
}

void
object_kv_destory (ObjectKV* kv)
{
        free (kv);
}

static void
object_kv_destory_chain (ObjectKV* kv)
{
        if (kv == NULL)
                return;
        object_kv_destory_chain (kv->next);
        object_kv_destory (kv);
}

ObjectHash*
object_hash_new ()
{
        ObjectHash* hash;
        hash = (ObjectHash*)malloc (sizeof (ObjectHash));
        if (hash == NULL)
                return NULL;
        hash->table_size  = 0;
        hash->residue     = 0;
        hash->main_table  = NULL;
        hash->cache_table = NULL;
        hash->rehash_lock = 0;
        hash->hangs       = 0;
        /*为哈希表分配初始大小*/
        object_hash_add_size (hash, HASH_SIZE);
        return hash;
}

void
object_hash_destory (ObjectHash* hash)
{
        if (hash == NULL)
                return;
        if (hash->main_table == NULL) {
                free (hash);
                return;
        }
        for (uint i = 0; i < hash->table_size; ++i)
                if (hash->main_table[i] != NULL)
                        object_kv_destory_chain (hash->main_table[i]);
        free (hash->main_table);
        free (hash);
}

static void
object_hash_is_lock (ObjectHash* hash)
{
        if (hash == NULL)
                return;
        /*当 rehash_lock 上锁时进行循环*/
        while (hash->rehash_lock == 1) {
        }
}

void
object_hash_add_size (ObjectHash* hash, unsigned int size)
{
        if (hash == NULL)
                return;
        object_hash_is_lock (hash);
        old_size = hash->table_size;
        hash->table_size += size;
        hash->residue += size;
        hash->cache_table =
                (kvptr*)malloc (sizeof (kvptr) * (hash->table_size));
        /*新空间全置零*/
        memset (hash->cache_table, 0, (hash->table_size) * sizeof (kvptr));
        /*进行重哈希操作*/
        object_hash_rehash (hash);

        hash->cache_table = NULL;
}

uint
object_hash (void* p_key, const unsigned int table_size)
{
        return (ulong)p_key % table_size;
}

/*将 kv 及其链放置于 table 中，新的总表大小为 ts*/
static unsigned int
object_hash_rehash_once (kvptr kv, kvptr* table, unsigned int ts)
{
        unsigned int pst, hangs = 0; /*hash位*/
        kvptr        tmp = kv, tn, tc;
        /*tmp 是需要被分配的键值对（链）*/
        while (tmp != NULL) {
                /*根据当前键值对分配一个哈希位*/
                pst = object_hash (tmp->p_key, ts);
                /*tn 为新表上该哈希位对应的键值*/
                tn = table[pst];
                /*查看新表该哈希位上是否有键值对，有则获取到键值链的对后一位*/
                while (tn != NULL && tn->next != NULL)
                        tn = tn->next;
                /*新表该哈希位无键值，旧哈希链该位置为链尾*/
                if (tn == NULL && tmp->next == NULL) {
                        table[pst] = tmp; /*拷贝值*/
                        /*新表该哈希位无键值，旧哈希链该位置不是链尾*/
                } else if (tn == NULL && tmp->next != NULL) {
                        table[pst] = tmp;
                        /*新表该哈希位有键值，旧哈希链该位置为链尾*/
                } else if (tn != NULL && tmp->next == NULL) {
                        tn->next = tmp;
                        hangs += 1;
                        /*新表该哈希位有键值，旧哈希链该位置不是链尾*/
                } else {
                        tn->next = tmp;
                        hangs += 1;
                }
                /*搜寻旧表该哈希位链的下一位数据*/
                tc  = tmp;
                tmp = tmp->next;
                /*表哈希位为最后一个节点，下一节点置零*/
                if (tmp != NULL)
                        tc->next = NULL;
        }
        return hangs;
}

void
object_hash_rehash (ObjectHash* hash)
{
        hash->rehash_lock = 1;
        /*第一次操作直接移动表*/
        if (hash->main_table == NULL) {
                hash->main_table  = hash->cache_table;
                hash->rehash_lock = 0;
                return;
        }
        /*重置链式挂载数据*/
        hash->hangs = 0;
        /*复制表数据*/
        for (unsigned int i = 0; i < old_size; ++i) {
                if (hash->main_table[i] != NULL) {
                        hash->hangs +=
                                object_hash_rehash_once (hash->main_table[i],
                                                         hash->cache_table,
                                                         hash->table_size);
                }
        }
        /*释放原表*/
        free (hash->main_table);
        /*移动表*/
        hash->main_table  = hash->cache_table;
        hash->cache_table = NULL;
        hash->rehash_lock = 0;
}

/**
 * 本函数获得了 ckey 的释放权限！
 */
void
object_hash_set_data (ObjectHash* hash, void* p_key, const uint x, const uint y)
{
        unsigned int pst, compare = 0;
        kvptr        kv;

        if (hash == NULL)
                return;
        /*判断哈希表是否处于上锁状态*/
        object_hash_is_lock (hash);
        /*获取当前数据哈希位*/
        pst = object_hash (p_key, hash->table_size);
        kv  = hash->main_table[pst];
        /*在当前哈希位寻找有无相同的键，有则 compare 为 1，且 kv 锁定到相同键*/
        while (kv != NULL && kv->next != NULL) {
                if (object_hash_compare (kv->p_key, p_key)) {
                        compare = 1;
                        break;
                }
                kv = kv->next;
        }
        if (!compare && kv != NULL)
                if (object_hash_compare (kv->p_key, p_key))
                        compare = 1;
        /*有相同的键，只需修改数据*/
        if (compare) {
                /*先释放当前的数值*/
                kv->v_x = x;
                kv->v_y = y;
                /*修改完成即返回*/
                return;
        } else {
                /*无相同的键，即增加数据，剩余可用键减少1*/
                hash->residue -= 1;
        }
        /**无相同的键，且有下挂链
         *
         * ckey 的释放权限移交到下级函数
         */
        if (kv != NULL) {
                kv->next = object_kv_new_with (p_key, x, y);
                hash->hangs += 1;
        } else {
                hash->main_table[pst] = object_kv_new_with (p_key, x, y);
        }
        // object_hash_print_table (hash);
        /*判断是否需要增加容量*/
        if (HASH_IS_RESIZE (hash->residue, hash->table_size))
                object_hash_add_size (hash, HASH_SIZE);
}

/*注意！！！本函数获得了 ckey 的消除责任！！！是否执行消除取决于上层函数*/
void
object_hash_unset_data (ObjectHash* hash, void* p_key)
{
        kvptr        kv, kvtmp;
        unsigned int pst;

        if (hash == NULL)
                return;
        /*判断哈希表当前是否上锁*/
        object_hash_is_lock (hash);
        pst = object_hash (p_key, hash->table_size);
        kv  = hash->main_table[pst];
        if (kv == NULL)
                return;
        /*判断需要消除的是否是哈希链的第一个元素*/
        if (object_hash_compare (kv->p_key, p_key)) {
                hash->main_table[pst] = kv->next;
                /*重链接首节点后释放节点*/
                object_kv_destory (kv);
                /*可用节点数加一*/
                hash->residue += 1;
                return;
        }
        /*循环判断第二个及以后的元素是否是需要消除的元素*/
        while (kv->next != NULL) {
                /*若 kv->next 是需要消除的元素*/
                if (object_hash_compare (kv->p_key, p_key)) {
                        /*将前置节点连接到后置节点*/
                        kvtmp    = kv->next;
                        kv->next = kv->next->next;
                        object_kv_destory (kvtmp);
                        /*可用节点加一，下挂数减一*/
                        hash->residue += 1;
                        hash->hangs -= 1;
                        return;
                }
                kv = kv->next;
        }
}

/*比较两对值是否相等，若相等为 1，不相等为 0*/
static uint
object_hash_compare (void* p_key1, void* p_key2)
{
        return p_key1 == p_key2;
}

/*注意！！！本函数负有消除 ckey 的责任！！！*/
uint
object_hash_get_xy (ObjectHash* hash, void* p_key, int is_x)
{
        kvptr        kv = NULL;
        unsigned int pst;

        if (hash == NULL)
                goto free_and_return;
        object_hash_is_lock (hash);
        /*根据键和表大小获取键值位*/
        pst = object_hash (p_key, hash->table_size);
        /*取哈希位*/
        kv = hash->main_table[pst];
        /*循环寻找该哈希位上的所有数据*/
        while (kv != NULL) {
                if (object_hash_compare (kv->p_key, p_key))
                        goto free_and_return;
                kv = kv->next;
        }
free_and_return:
        return kv == NULL ? 0 : (is_x == 1 ? kv->v_x : kv->v_y);
}

void
object_hash_print_status (ObjectHash* hash)
{
        if (hash == NULL)
                return;
        printf ("当前：\n总数 <%u>, 可用 <%u>, 重叠的 <%u>\n",
                hash->table_size,
                hash->residue,
                hash->hangs);
}
