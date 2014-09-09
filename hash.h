#ifndef _HASH_H_
#define _HASH_H_

//hash函数
typedef unsigned int (*hashfunc_t)(unsigned int, void*);

//hash结点定义
typedef struct Hash_node
{
    void *key; //关键字
    void *value; //要的存储信息
    struct Hash_node *pre;
    struct Hash_node *next;
}Hash_node_t;   

//hash表定义
typedef struct Hash
{
    unsigned int buckets; //bucket个数
    hashfunc_t hash_func; //hash函数
    Hash_node_t **nodes; //链表数组
}Hash_t;

//
Hash_t *hash_alloc(unsigned int buckets, hashfunc_t hash_func);
void *hash_lookup_value_by_key(Hash_t *hash, void *key, unsigned int key_size);
void hash_add_entry(Hash_t *hash, void *key, unsigned int key_size, void *value, unsigned int value_size);
void hash_free_entry(Hash_t *hash, void *key, unsigned int key_size);
void hash_clear_entry(Hash_t *hash);
void hash_destroy(Hash_t *hash);

#endif  /*_HASH_H_*/
