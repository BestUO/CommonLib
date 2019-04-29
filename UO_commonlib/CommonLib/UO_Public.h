#ifndef PUBLIC_H_UO
#define PUBLIC_H_UO
#include <iostream>
#include <string.h>
#include <unistd.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define CACHE_LINE_SIZE 64 
#define cache_aligned __attribute__((__aligned__(CACHE_LINE_SIZE)))

//from Ceph
extern uint32_t UO_HashFun1(uint32_t a);
//from Jenkins hash function
extern uint32_t HashFun2(uint8_t *key,size_t length);
extern uint32_t public_align32pow2(uint32_t x);
extern inline int set_cpu(int i);

class UO_SpinLock
{
public:
	UO_SpinLock():m_spinlock(0) {}
	void Lock() {while(__sync_lock_test_and_set(&m_spinlock, 1)) {};}
	void UnLock() {__sync_lock_release(&m_spinlock);}
private:
	volatile int m_spinlock;
};

class UO_Tools
{
public:
	int Get_Cpu_num()
	{
		return sysconf( _SC_NPROCESSORS_ONLN);
	}

	int KmpSearch(uint8_t *s,int slen, uint8_t *p,int plen)
	{
		int i = 0;
		int j = 0;
		int sLen = slen;
		int pLen = plen;
		int next[256] = {0};
		GetNextval(p,plen,next);
		while (i < sLen && j < pLen)
		{
			//①如果j = -1，或者当前字符匹配成功（即S[i] == P[j]），都令i++，j++    
			if (j == -1 || s[i] == p[j])
			{
				++i;
				++j;
			}
			else   
				j = next[j];
		}
		if (j == pLen)
			return i - j;
		else
			return -1;
	}
private:
	void GetNextval(uint8_t* p, int plen, int *next)
	{
		int pLen = plen;
		next[0] = -1;
		int k = -1;
		int j = 0;
		while (j < pLen - 1)
		{
			//p[k]表示前缀，p[j]表示后缀  
			if (k == -1 || p[j] == p[k])
			{
				++j;
				++k;
				//较之前next数组求法，改动在下面4行
				if (p[j] != p[k])
					next[j] = k;   //之前只有这一行
				else
					//因为不能出现p[j] = p[ next[j ]]，所以当出现时需要继续递归，k = next[k] = next[next[k]]
					next[j] = next[k];
			}
			else
				k = next[k];
		}
	}
};
#endif