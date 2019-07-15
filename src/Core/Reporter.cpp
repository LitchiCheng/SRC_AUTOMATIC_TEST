#include "GlobalData.h"
#include "Reporter.h"
#include "socket.h"
#include "CommandDispatchTask.h"

//#include <assert.h>

namespace{
	int socket_n = -1;
	const uint16_t RPT_DESTPORT = 5002;
	sockaddr_in destaddr;
	sockaddr_in localaddr;
}

uint8_t CReporter_base::transBuff_[CReporter_base::TRANS_BUFF_LEN];
bool CReporter_base::isInitialized_ = false;


int CReporter_base::upload(uint32_t len, uint8_t* ip, uint16_t port)
{

	int ret = 0;
	if(socket_n < 0)
	{
		localaddr.sin_family = AF_INET;
		localaddr.sin_port = htons(56789);
		localaddr.sin_addr.s_addr = INADDR_ANY;
		
		socket_n = net::socket(AF_INET, SOCK_DGRAM, 0);
		net::bind(socket_n, (sockaddr *)&localaddr,  sizeof(localaddr));
		Console::Instance()->printf("Init report socket: [%d]\r\n", socket_n);
		
		if(socket_n < 0)
		{
			Console::Instance()->printf("Open reporter socket faild, ret = %d\r\n", socket_n);
			return -1;
		}

	}
	
	destaddr.sin_family =  AF_INET;
	destaddr.sin_port = htons(port);
	destaddr.sin_addr.s_addr = byteston(ip);
	ret = net::sendto(socket_n, transBuff_, len, MSG_DONTWAIT, (sockaddr *)&destaddr, sizeof(destaddr));

	if(ret < 0)
	{
		isInitialized_ = false;
	}
	return ret;
}



int CReporter_base::uploadRbk(uint32_t len)
{
	
	return upload(len, (uint8_t*)CmdSocket::Instance()->rbkIP(), RPT_DESTPORT);
}

