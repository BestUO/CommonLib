#include "UO_Thread.h"

UO_Thread::UO_Thread():m_runthread(true)
{
	m_threadid = std::thread(&UO_Thread::Start_thread,this);
}
UO_Thread::~UO_Thread()
{
	m_runthread = false;
	m_threadid.join();
}
void UO_Thread::set_thread_run(bool flag)
{
	m_runthread = flag;
}

void UO_Thread::Start_thread()
{
	while(m_runthread)
		start();
}
