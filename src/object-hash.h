/**
 * Object 哈希键值表系统
 * 辅助扫雷按钮坐标存储
 */
#ifndef OBJECT_HASH_H
#define OBJECT_HASH_H

#include "object-type.h"


/*每次可增加的哈希表大小*/
#define HASH_SIZE 32
/*进行重哈希的比率系数，即剩余位置系数*/
#define HASH_REPERSENT 0.4
/*判断是否需要重哈希，需要则返回真*/
#define HASH_IS_RESIZE(now, total) \
        (((double)(now)) / ((double)(total)) < HASH_REPERSENT ? 1 : 0)

/*声明哈希对象的构造器和清除器*/
OBJECT_SET_TYPE (ObjectHash, object_hash)

/*键值对结构*/
OBJECT_SET_TYPE (ObjectKV, object_kv)
typedef ObjectKV* kvptr;
struct _ObjectKVClass
{
        void*        p_key;
        unsigned int v_x;
        unsigned int v_y;
        kvptr        next;
};
/*设置键值对数据*/
void object_kv_set_data (kvptr kv, void* p_key, const uint x, const uint y);
/*从数据新建键值对*/
kvptr object_kv_new_with (void* p_key, const uint x, const uint y);

/*哈希对象主结构，存储结构和重哈希结构*/
struct _ObjectHashClass
{
        unsigned int table_size;  /*总表大小*/
        unsigned int residue;     /*剩余可用数量*/
        unsigned int hangs;       /*链挂载数量*/
        unsigned int rehash_lock; /*重哈希时上锁*/
        kvptr*       main_table;  /*主存储表*/
        kvptr*       cache_table; /*临时挂载表*/
};

/*哈希函数*/
uint object_hash (void* p_key, const unsigned int table_size);

/*增加哈希表的大小*/
void object_hash_add_size (ObjectHash* hash, unsigned int size);

/*重哈希*/
void object_hash_rehash (ObjectHash* hash);

/*添加数据到表，支持的键为 ckey 或 rkey */
void object_hash_set_data (ObjectHash* hash,
                           void*       p_key,
                           const uint  x,
                           const uint  y);

/*消除数据*/
void object_hash_unset_data (ObjectHash* hash, void* p_key);

/*搜寻数据*/
uint object_hash_get_xy (ObjectHash* hash, void* p_key, int is_x);

/*打印哈希表当前存储状态*/
void object_hash_print_status (ObjectHash* hash);

extern ObjectHash* hash;

#endif