#include "UO_Net.h"
#include <iostream>

UO_Epoll::UO_Epoll(UO_RingQueue *pringqueue,MemoryPool<SocketPacket> *mempool,uint8_t recvnum)
:m_pmempool(mempool),m_pringqueue(pringqueue),m_recvnum(recvnum < MAXRECVNUM? recvnum:MAXRECVNUM),m_bEXIT(false)
{
	memset(m_sockets,0,MAXRECVNUM * sizeof(int));
}

UO_Epoll::~UO_Epoll()
{
	for(auto &t : m_vthreads)
		t.join();
}

void UO_Epoll::_epoll_udp_run(int fd)
{
	int epfd = epoll_create(MAXSOCKETS);
	struct epoll_event ev,events[MAXSOCKETS];
	ev.events=EPOLLIN|EPOLLET;
	ev.data.ptr = nullptr;
	ev.data.fd = fd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
	int nfds;
	while(!m_bEXIT)
	{
		 nfds = epoll_wait(epfd, events, MAXSOCKETS, 1);
		if (nfds <= 0) 
			continue;

		for (int n = 0; n < nfds; ++n) 
		{
			if(!m_pmempool)
				m_ppack = new SocketPacket;
			else
				m_ppack = m_pmempool->newElement();
			if(!m_ppack)
				continue;
			m_ppack->fd = events[n].data.fd;
			socklen_t len = sizeof(struct sockaddr_in);
			recvfrom(m_ppack->fd, m_ppack->buf, MTUSIZE, 0,(struct sockaddr*)&m_ppack->clientAddr, &len);
			if(strstr((char*)m_ppack->buf,"EXIT"))
				m_bEXIT = true;
			else
			{
				m_pringqueue->pushMSG(m_ppack);
				std::cout << m_ppack->buf;
			}
			m_ppack = (SocketPacket*)m_pringqueue->popMSG();
			m_pmempool->deleteElement(m_ppack);
		}
		std::cout << pthread_self() << std::endl;
	}
}

bool UO_Epoll::_inisockets(int fd,std::string ip,int port)
{
	int opt_val = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(opt_val));

	fcntl(fd, F_SETFL, O_NONBLOCK);

	int socketBuf=32*1024;//设置为32K
	setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(const char*)&socketBuf,sizeof(int));
	setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(const char*)&socketBuf,sizeof(int));

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip.data());  //设置地址
    servaddr.sin_port = htons(port);//设置的端口为DEFAULT_PORT

	if(-1 == bind(fd, (struct sockaddr*)(&servaddr), sizeof(sockaddr_in)))
	{
		std::cout << "bind udp v4 server socket address error:"<< strerror(errno) << std::endl;
		return false;
	}
	return true;	
}

void UO_Epoll::startudp_server(std::string ip,int port)
{
	for(int i=0;i<m_recvnum;i++)
	{
		m_sockets[i] = socket(AF_INET, SOCK_DGRAM, 0);
		if(_inisockets(m_sockets[i],ip,port))
			m_vthreads.push_back(std::thread(&UO_Epoll::_epoll_udp_run,this,m_sockets[i]));
	}
}

void UO_Epoll::starttcp_server(std::string ip,int port)
{}

UO_Asio_UDP::UO_Asio_UDP(UO_RingQueue *pringqueue,MemoryPool<SocketPacket> *mempool,uint8_t recvnum):
m_pmempool(mempool),m_pringqueue(pringqueue),m_recvnum(recvnum < MAXRECVNUM? recvnum:MAXRECVNUM),m_bEXIT(false)
{
	//m_fun = std::bind(&UO_Asio_UDP::_Handle_Receive, this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4);
}

UO_Asio_UDP::~UO_Asio_UDP()
{
	for(auto &t : m_vthreads)
		t.join();

	for(int i = 0;i < m_recvnum; i++)
	{
		asio::ip::udp::socket *p = m_vudpsock.back();
		m_vudpsock.pop_back();
		delete p;
	}
}

void UO_Asio_UDP::startudp_server(std::string ip,int port)
{
	for(int i=0;i<m_recvnum;i++)
	{
		m_vudpsock.push_back(new asio::ip::udp::socket(m_io_context));
		m_vthreads.push_back(std::thread(&UO_Asio_UDP::_asio_udp_run,this,m_vudpsock.at(i), ip,port));
	}
}

void UO_Asio_UDP::_asio_udp_run(asio::ip::udp::socket *udpsocket, std::string ip,int port)
{
	asio::ip::udp::endpoint ep(asio::ip::address::from_string(ip), port);
	udpsocket->open(ep.protocol());
	udpsocket->set_option(asio::ip::udp::socket::reuse_address(true));
	udpsocket->set_option(asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT>(true));
	
    udpsocket->set_option(asio::socket_base::receive_buffer_size(1024 * 1024));
    udpsocket->set_option(asio::socket_base::send_buffer_size(1024 * 1024));
	udpsocket->bind(ep);
	_RecvMSG(udpsocket);
	m_io_context.run();
}

void UO_Asio_UDP::_RecvMSG(asio::ip::udp::socket *udpsocket)
{	
	SocketPacket *ppack;
	if(!m_pmempool)
		ppack = new SocketPacket;
	else
		ppack = m_pmempool->newElement();
	
	//udpsocket->async_receive_from(asio::buffer(ppack->buf,MTUSIZE), ppack->remoteinfo,m_fun);
	udpsocket->async_receive_from(asio::buffer(ppack->buf,MTUSIZE), ppack->remoteinfo,
		std::bind(&UO_Asio_UDP::_Handle_Receive, this,std::placeholders::_1,std::placeholders::_2,udpsocket,ppack));
}

void UO_Asio_UDP::_Handle_Receive(const std::error_code &error,std::size_t len,asio::ip::udp::socket *udpsocket,SocketPacket *ppack)
{
	std::cout << "from :" << udpsocket << std::endl;
	std::cout << "from :" << pthread_self() << std::endl;
	if (!error)
	{
		//std::cout << "received from<" << m_remote_endpoint.address().to_string() << ":" << m_remote_endpoint.port() << ">" << std::endl;
		if(strstr((char*)ppack->buf,"EXIT"))
			m_io_context.stop();
		else
		{
			m_pringqueue->pushMSG(ppack);
			std::cout << ppack->buf;
			_RecvMSG(udpsocket);
		}
		ppack = (SocketPacket*)m_pringqueue->popMSG();
		m_pmempool->deleteElement(ppack);
	}
}