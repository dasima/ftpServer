#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Student
{
	char sno[5];
	char name[32];
	int age;
}Stu_t;

unsigned int hash_str(unsigned int buckets, void *key)
{
	char *sno = (char *)key;
	unsigned int index = 0;

	while(*sno)
	{
		index = *sno + 4 * index;
		++sno;
	}

	return index % buckets;
}

int main(int argc, char* argv[])
{
	Stu_t stu[] =
	{
		{ "1011", "aaaa", 23},
		{ "1012", "bbbb", 25},
		{ "1013", "cccc", 22}
	};

	Hash_t *hash = hash_alloc(20, hash_str);

	//添加元素
	int size = sizeof(stu) / sizeof(stu[0]);
	int i;
	for(i = 0; i < size; ++i)
	{
		hash_add_entry(hash, stu[i].sno, strlen(stu[i].sno), &stu[i], sizeof(stu[i]));		
	}
	printf("------\n");
	for(i = 0; i < size; ++i)
	{
		printf("%s %s %d\n", stu[i].sno, stu[i].name, stu[i].age);
	}
	printf("\nsearch: ----\n");

	//查找元素
	Stu_t *s = (Stu_t *)hash_lookup_value_by_key(hash, "1012", strlen("1012"));
	if(s)
	{
		printf("%s %s %d\n", s->sno, s->name, s->age);
	}
	else
	{
		printf("not found\n");
	}

	//删除结点
	printf("\nsearch after free:\n");
	hash_free_entry(hash, "1012", strlen("1012"));
	s = (Stu_t *)hash_lookup_value_by_key(hash, "1012", strlen("1012"));
	if(s)
	{
		printf("%s %s %d\n", s->sno, s->name, s->age);
	}
	else
	{
		printf("not found\n");
	}

	//清空结点
	hash_clear_entry(hash);

	//销毁hash
	hash_destroy(hash);

	return 0;
}