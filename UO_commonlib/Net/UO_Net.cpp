#include "UO_Net.h"
#include <iostream>

UO_Epoll::UO_Epoll(UO_RingQueue *pringqueue,SERVERTYPE type,std::string ip,int port)
{

}

UO_Epoll::~UO_Epoll()
{

}

void UO_Epoll::startudp_server(std::string ip,int port)
{}

void UO_Epoll::starttcp_server(std::string ip,int port)
{}

UO_Asio_UDP::UO_Asio_UDP(UO_RingQueue *pringqueue,MemoryPool<SocketPacket> *mempool):
m_udpsock(m_io_context),m_pmempool(mempool),m_pringqueue(pringqueue)
{
}

UO_Asio_UDP::~UO_Asio_UDP()
{
	m_thread.join();
}

void UO_Asio_UDP::startudp_server(std::string &ip,int port)
{
	m_fun = std::bind(&UO_Asio_UDP::_Handle_Receive, this,std::placeholders::_1,std::placeholders::_2);
	asio::ip::udp::endpoint ep(asio::ip::address::from_string(ip), port);
	m_udpsock.open(ep.protocol());
	m_udpsock.set_option(asio::ip::udp::socket::reuse_address(true));
	m_udpsock.set_option(asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT>(true));
	
    m_udpsock.set_option(asio::socket_base::receive_buffer_size(1024 * 1024));
    m_udpsock.set_option(asio::socket_base::send_buffer_size(1024 * 1024));
	m_udpsock.bind(ep);
	_RecvMSG();
	m_io_context.run();
	m_thread = std::thread([&](){m_io_context.run();});
}

void UO_Asio_UDP::_RecvMSG()
{	
	if(!m_pmempool)
		m_ppack = new SocketPacket;
	else
		m_ppack = m_pmempool->newElement();
	m_udpsock.async_receive_from(asio::buffer(m_ppack->buf,MTUSIZE), m_remote_endpoint,m_fun);
}

void UO_Asio_UDP::_Handle_Receive(const std::error_code &error,std::size_t len)
{
	if (!error)
	{
		//std::cout << "received from<" << m_remote_endpoint.address().to_string() << ":" << m_remote_endpoint.port() << ">" << std::endl;
		m_ppack->remoteinfo = m_remote_endpoint;
		m_pringqueue->pushMSG(m_ppack);
		if(strstr((char*)m_ppack->buf,"EXIT"))
			return;
		std::cout << m_ppack->buf;
	}
	_RecvMSG();
}