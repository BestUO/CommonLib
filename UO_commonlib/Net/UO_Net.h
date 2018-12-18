#ifndef NET_H_UO
#define NET_H_UO

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include "asio.hpp"
#include "thread"
#include "../CommonLib/UO_Queue.h"
#define MTUSIZE 1460

enum SERVERTYPE{TCP,UDP};

struct SocketPacket
{
	uint8_t buf[MTUSIZE];
	asio::ip::udp::endpoint remoteinfo;
	SocketPacket() {memset(buf,0,MTUSIZE);}
};

class UO_Epoll
{
public:
	UO_Epoll(UO_RingQueue *pringqueue,SERVERTYPE type=UDP,std::string ip="127.0.0.1",int port=12345);
	~UO_Epoll();
private:
	UO_RingQueue *m_pringqueue;
	void startudp_server(std::string ip,int port);
	void starttcp_server(std::string ip,int port);
};

class UO_Asio_UDP
{
public:
	UO_Asio_UDP(UO_RingQueue *pringqueue,MemoryPool<SocketPacket> *mempool=nullptr);
	~UO_Asio_UDP();
	void startudp_server(std::string &ip,int port);
private:
	asio::ip::udp::endpoint m_remote_endpoint;
	asio::io_context m_io_context;
	asio::ip::udp::socket m_udpsock;
	MemoryPool<SocketPacket> *m_pmempool;
	std::function<void (const std::error_code &error,std::size_t len)> m_fun;
	UO_RingQueue *m_pringqueue;
	SocketPacket *m_ppack;
	std::thread m_thread;
	void _RecvMSG();
	void _Handle_Receive(const std::error_code &error,std::size_t len);
};
/*not complete
class UO_RingBuf:public UO_SpinLock
{
public:
	UO_RingBuf(uint32_t size=0xff):m_size(size) 
	{
		m_pbuf = new char[size];
		m_phead = m_ptail = m_pbuf;
		m_ptail++;
	}
	~UO_RingBuf() 
	{
		if(m_pbuf)
			delete []m_pbuf;
	}
	uint32_t readringbuf(char *str,int len = 0)
	{
		//assume
		//|0~15|16~31|~~~
		//0x87f1|len|date
		uint8_t sign[2] = {0x87,0xf1};
		int readlen = 0;
		int pos = 0;
		Lock();
		pos = m_tools.KmpSearch(m_phead,m_pbuf + m_size - m_phead,sign,2);
		if(pos > -1)
		{
			readlen = *(uint16_t*)(m_phead+2);
			if(m_phead + readlen <= m_ptail)
			{
				memcpy(str,m_phead,readlen);
				memset(m_phead,0,readlen);
				m_phead+=readlen;
			}
		}
		else
		{
			pos = m_tools.KmpSearch(m_pbuf, m_size,sign,2);
			if(pos > -1)
			{
				m_phead = m_pbuf;
				readlen = *(uint16_t*)(m_phead+2);
				if(m_phead + readlen <= m_ptail)
				{
					memcpy(str,m_phead,readlen);
					memset(m_phead,0,readlen);
					m_phead+=readlen;
				}
			}
		}
		UnLock();
		return readlen;
	}
	uint32_t writeringbuf(char *sstr,uint32_t len)
	{
		uint32_t rv = 0;
		Lock();
		char *dstr = getpos(len);
		if(dstr)
		{
			memcpy(dstr,sstr,len);
			rv = len;
		}
		tailposchange(len);
		UnLock();
		return rv;
	}

	char *getpos(uint32_t len)
	{
		if((m_ptail > m_phead) && (m_ptail + len <= m_pbuf + m_size))
			m_ptail = m_ptail;
		else if((m_ptail > m_phead) && (m_ptail + len > m_pbuf + m_size))
		{
			if(m_pbuf + len >= m_phead)
				return NULL;
			else
				m_ptail = m_pbuf;
		}
		else if((m_ptail < m_phead) && (m_ptail + len >= m_phead))
			return NULL;
		return m_ptail;
	}

	void tailposchange(uint32_t len)
	{
		m_ptail += len;
	}
private:
	UO_Tools m_tools;
	uint32_t m_size;
	char *m_pbuf;
	char *m_phead;
	char *m_ptail;
};
*/
#endif