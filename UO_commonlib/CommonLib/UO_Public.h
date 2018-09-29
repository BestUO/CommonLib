#ifndef PUBLIC_H
#define PUBLIC_H
#include <iostream>
#include "unistd.h"

class UO_SpinLock
{
public:
	UO_SpinLock():m_spinlock(0) {}
	void Lock() {while(__sync_lock_test_and_set(&m_spinlock, 1)) {};}
	void Unlock() {__sync_lock_release(&m_spinlock);}
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
};

#endif