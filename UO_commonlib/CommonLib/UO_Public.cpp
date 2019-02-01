
#include "UO_Public.h"

#define crush_hashmix(a, b, c) do {			\
	a = a-b;  a = a-c;  a = a^(c>>13);	\
	b = b-c;  b = b-a;  b = b^(a<<8);	\
	c = c-a;  c = c-b;  c = c^(b>>13);	\
	a = a-b;  a = a-c;  a = a^(c>>12);	\
	b = b-c;  b = b-a;  b = b^(a<<16);	\
	c = c-a;  c = c-b;  c = c^(b>>5);	\
	a = a-b;  a = a-c;  a = a^(c>>3);	\
	b = b-c;  b = b-a;  b = b^(a<<10);	\
	c = c-a;  c = c-b;  c = c^(b>>15);	\
} while (0)

//from Ceph
uint32_t UO_HashFun1(uint32_t a)
{
	uint32_t hash = 0x4E67C6A7  ^ a;//	uint32_t crush_hash_seed = 0x4E67C6A7;
	uint32_t b = a;
	uint32_t x = 231232;
	uint32_t y = 1232;
	crush_hashmix(b, x, hash);
	crush_hashmix(y, a, hash);
	return hash;
}

//from Jenkins hash function
uint32_t HashFun2(uint8_t *key,size_t length)
{
	size_t i = 0;
	uint32_t hash = 0;
	while (i != length) 
	{
		hash += key[i++];
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}