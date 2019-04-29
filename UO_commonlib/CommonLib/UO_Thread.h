#ifndef THREAD_UO_H
#define THREAD_UO_H
#include <iostream>
#include <thread>
#include <functional>
#include <vector>

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include "../Net/asio.hpp"


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

 using namespace std;
 typedef std::shared_ptr<std::thread> thread_ptr;
 typedef std::vector<thread_ptr> vecThread;
 
 class UO_ThreadPool {
 public:
     explicit UO_ThreadPool(int num) : stopped_(false), threadNum_(num),  work_(io_) {
         for(int i=0; i<threadNum_; ++i) {
             threads_.push_back(std::make_shared<std::thread>([&](){io_.run();}));
         }
     }   
     ~UO_ThreadPool() {
         _stop();  
     }   
     template<typename F, typename...Args>
     void post(F &&f, Args&&...args) {
         io_.post(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
     }
     void stop()
     {
     	io_.stop();
     }
 
 private:
	void _stop() 
	{
		if(!stopped_) 
		{
			for(auto t : threads_) 
				t->join();
			stopped_ = true;
		}
	}  
	bool             stopped_;
	vecThread        threads_;
	int              threadNum_;
	asio::io_service io_;
	asio::io_service::work work_;
 };

#endif