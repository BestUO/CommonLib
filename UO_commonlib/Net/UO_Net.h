#ifndef NET_H_UO
#define NET_H_UO

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include "asio.hpp"
#include <thread>
#include <functional>
#include "../CommonLib/UO_Queue.h"
#define MTUSIZE 1460
#define MAXRECVNUM 8
#define MAXSOCKETS 256

struct SocketPacket
{
	uint8_t buf[MTUSIZE];
	asio::ip::udp::endpoint remoteinfo;//just for asio
	int fd;
	struct sockaddr_in clientAddr;
	SocketPacket() {memset(buf,0,MTUSIZE);}
};

class UO_Epoll
{
public:
	UO_Epoll(UO_RingQueue *pringqueue,MemoryPool<SocketPacket> *mempool=nullptr,uint8_t recvnum = 1);
	~UO_Epoll();
	void startudp_server(std::string ip,int port);
	void starttcp_server(std::string ip,int port);
private:
	MemoryPool<SocketPacket> *m_pmempool;
	UO_RingQueue *m_pringqueue;
	SocketPacket *m_ppack = nullptr;
	int m_sockets[MAXRECVNUM];
	uint8_t m_recvnum;
	std::vector<std::thread> m_vthreads;
	bool m_bEXIT;
	void _epoll_udp_run(int fd);
	bool _inisockets(int fd,std::string ip,int port);

};

class UO_Asio_UDP
{
public:
	UO_Asio_UDP(UO_RingQueue *pringqueue,MemoryPool<SocketPacket> *mempool=nullptr,uint8_t recvnum = 1);
	~UO_Asio_UDP();
	void startudp_server(std::string ip,int port);
private:
	asio::io_context m_io_context;
	std::vector<asio::ip::udp::socket *> m_vudpsock;
	MemoryPool<SocketPacket> *m_pmempool;
	//std::function<void (const std::error_code &error,std::size_t len,asio::ip::udp::socket *udpsocket,SocketPacket *ppack)> m_fun;
	UO_RingQueue *m_pringqueue;
	uint16_t m_recvnum;
	bool m_bEXIT;
	std::vector<std::thread> m_vthreads;
	void _RecvMSG(asio::ip::udp::socket *udpsocket);
	void _Handle_Receive(const std::error_code &error,std::size_t len,asio::ip::udp::socket *udpsocket,SocketPacket *ppack);
	void _asio_udp_run(asio::ip::udp::socket *udpsocket,std::string ip,int port);
};
#endif