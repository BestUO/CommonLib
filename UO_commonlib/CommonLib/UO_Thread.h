#ifndef THREAD_UO_H
#define THREAD_UO_H
#include <iostream>
#include <thread>
class UO_Thread
{
public:
	UO_Thread();
	~UO_Thread();
	void set_thread_run(bool flag);
private:
	std::thread m_threadid;
	bool m_runthread;
	void Start_thread();
	virtual void start()
	{
		std::cout << "int thread base" << std::endl;
	}
};

#endif