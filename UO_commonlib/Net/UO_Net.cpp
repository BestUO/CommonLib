#include "UO_Net.h"
#include "asio.hpp"

void NetTest::test()
{
	asio::io_context io;
	asio::steady_timer t(io, asio::chrono::seconds(5));
	t.wait();
}