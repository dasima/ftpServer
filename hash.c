#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static Hash_node_t ** hash_get_bucket(Hash_t *hash, void *key);
static Hash_node_t *hash_get_node_by_key(Hash_t *hash, void *key, unsigned int key_size);
static void hash_clear_bucket(Hash_t *hash, unsigned int index);

//建立一个hash表，然后找回指针返回
Hash_t *hash_alloc(unsigned int buckets, hashfunc_t hash_func)
{
    Hash_t *hash = (Hash_t *)malloc(sizeof(Hash_t));
    hash->buckets = buckets; //桶的个数
    hash->hash_func = hash_func; //hash函数
    int len = sizeof(Hash_node_t *) * buckets; //数组占用字节数
    hash->nodes = (Hash_node_t **)malloc(len);
    memset(hash->nodes, 0, len);
    return hash;
}

//根据key查找value
void *hash_lookup_value_by_key(Hash_t *hash, 
        void *key, 
        unsigned int key_size)
{
    //先查找结点
    Hash_node_t *node = hash_get_node_by_key(hash, key, key_size);
    if(node == NULL)
    {
        return NULL;
    }
    else
    {
        return node->value;
    }
}

//向hash中添加结点
void hash_add_entry(Hash_t *hash, 
        void *key, 
        unsigned int key_size, 
        void *value,
        unsigned int value_size)
{
    //1. 查找bucket
    Hash_node_t **buck = hash_get_bucket(hash, key);
    assert(buck != NULL);

    //2.生成新的结点
    Hash_node_t *node = (Hash_node_t *)malloc(sizeof(Hash_node_t));
    memset(node, 0, sizeof(Hash_node_t));
    node->key = malloc(key_size);
    node->value = malloc(value_size);
    memcpy(node->key, key, key_size);
    memcpy(node->value, value, value_size); 

    //3.插入相应的链表， 头插法
    if(*buck != NULL)
    {
        Hash_node_t *head = *buck; //head是链表第一个结点

        node->next = head;
        head->pre = node;
        *buck = node; // 这里要用指针操作
    }
    else
    {
        *buck = node;
    }
}

//hash中释放结点
void hash_free_entry(Hash_t *hash, 
        void *key, 
        unsigned int key_size)
{
    //查找结点
    Hash_node_t *node = hash_get_node_by_key(hash, key, key_size);
    assert(node != NULL);

    free(node->key);
    free(node->value);

    //删除结点
    if(node->pre != NULL) //不是第一个结点
    {
        node->pre->next = node->next;
    }
    else
    {
        Hash_node_t **buck = hash_get_bucket(hash, key);
        *buck = node->next;
    }
    if(node->next != NULL) //还有下一个结点
        node->next->pre = node->pre;
    free(node);
}

//清空hash表
void hash_clear_entry(Hash_t *hash)
{
    unsigned int i;
    for(i = 0; i < hash->buckets; ++i)
    {
        hash_clear_bucket(hash, i);
        hash->nodes[i] = NULL;
    }
}

//销毁hash表
void hash_destroy(Hash_t *hash)
{
    hash_clear_entry(hash);
    free(hash->nodes);
    free(hash);
}

//根据key获取bucket
static Hash_node_t **hash_get_bucket(Hash_t *hash, void *key)
{
    unsigned int pos = hash->hash_func(hash->buckets, key); //根据key获取key所在buckets的位置
    assert(pos < hash->buckets);

    return &(hash->nodes[pos]); //返回key所在链表地址
}

//根据key获取node结点
static Hash_node_t *hash_get_node_by_key(Hash_t *hash, 
        void *key, 
        unsigned int key_size)
{
    //获取bucket
    Hash_node_t **buck = hash_get_bucket(hash, key);
    assert(buck != NULL);

    //查找元素
    Hash_node_t *node = *buck; //指向(key所在链表的）第一个元素
    while(node != NULL && memcmp(node->key, key, key_size) != 0)
    {
        node = node->next;
    }

    return node; //包含三种情况 NULL、???
}

//清空bucket
static void hash_clear_bucket(Hash_t *hash, unsigned int index)
{
    assert(index < hash->buckets); //防止越界
    Hash_node_t *node = hash->nodes[index]; //获得key所在桶第一个元素
    Hash_node_t *tmp = node;
    while(node)
    {
        tmp = node->next;
        free(node->key);
        free(node->value);
        free(node);
        node = tmp;
    }
    hash->nodes[index] = NULL;
}
