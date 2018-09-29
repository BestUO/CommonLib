#include "UO_Alarm.h"

UO_Alarm::UO_Alarm(std::function<void(void*)> const &p):m_avltree(nullptr),m_callback(p)
{
	m_avltree = new UO_AVLTree(std::bind(&UO_Alarm::alarmcompare,this,std::placeholders::_1,std::placeholders::_2));
}

UO_Alarm::~UO_Alarm()
{
	NODE *temp = static_cast<NODE*>(m_avltree->get_min_item());
	while(temp)
	{
		m_avltree->delete_item_from_avltree(temp);
		delete temp;
		temp = static_cast<NODE*>(m_avltree->get_min_item());
	}
	delete m_avltree;
}

bool UO_Alarm::setalarmtime(int interval,void *param)
{
	unsigned long time1 = (unsigned long)time(NULL) + interval; 
	NODE *node = new NODE(time1,param);
	return m_avltree->insert_item_to_avltree(node);
}

int UO_Alarm::alarmcompare(void *node1,void *node2)
{
	return (static_cast<NODE*>(node1))->interval - (static_cast<NODE*>(node2))->interval;
}

void UO_Alarm::start()
{
	NODE *temp = static_cast<NODE*>(m_avltree->get_min_item());
	if(!temp)
		sleep(1);
	else
	{
		unsigned long time1 = (unsigned long)time(NULL);
		if(time1 >= temp->interval)
		{
			m_callback(temp->param);
			m_avltree->delete_item_from_avltree(temp);
			delete temp;
		}
		else
			sleep(temp->interval - time1);
	}
}