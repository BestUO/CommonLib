#ifndef ALARM_H_UO
#define ALARM_H_UO

#include <condition_variable>
#include "UO_Thread.h"
#include "UO_Tree.h"

class UO_Alarm:public UO_Thread
{
public:
	UO_Alarm(std::function<void(void*)> const &p);
	~UO_Alarm();
	bool setalarmtime(int interval,void *param);
private:
	struct NODE
	{
		unsigned long interval;
		void *param;
		NODE(int i,void *p):interval(i),param(p){}
		~NODE(){}
	};
	UO_AVLTree *m_avltree;
	std::function<void(void*)> m_callback;
	int alarmcompare(void *node1,void *node2);
	void start();
};

#endif