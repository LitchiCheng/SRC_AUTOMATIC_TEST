#include "CommandDispatchTask.h"
#include "socket.h"
#include "Timer.h"
#include "NetworkAdapter.h"
#include "TaskManager.h"
#include "stmflash.h"
#include "version.h"
#include "powerupOption.h"
#include "callbackhandler.h"
#include "Console.h"

typedef struct
{
	uint32_t ECommand;
	CCallbackHandler_base* pHandler;
}Handler_t;

static int chipResetHandlerFunc(uint8_t* msg, uint16_t len);
static int iapHandlerFunc(uint8_t* msg, uint16_t len);
uint8_t CCommandDispatchTask::_commandBuffer[CCommandDispatchTask::INSTRUCT_BUFF_LEN];

namespace {
	bool firstIn = true;

	uint8_t destip[4];
	uint16_t destport;
	int socket_n = -1;

	const uint8_t MAX_HANDLER_NUM = 128;


	CCallbackHandler<void> chipResetCommandHandler(chipResetHandlerFunc);
	CCallbackHandler<void> iapCommandHandler(iapHandlerFunc);
	CCallbackHandler<CTaskManager> addTaskCommandHandler(TaskManager::Instance(), &CTaskManager::addTaskCommandHandler);
	CCallbackHandler<CVersion> versionQueryCmdHandler(Version::Instance(), &CVersion::queryVersionHandler);


	Handler_t CommandHandlerTab[MAX_HANDLER_NUM] =
	{
		{0x00000001, NULL},
		{0x00000002, NULL},
		{0x00001032, &versionQueryCmdHandler},
		{0x0000101D, &iapCommandHandler},	//EInApplicationProgram
		{0x0000101E, &chipResetCommandHandler},	//EResetDIOBoard
	};
}

uint8_t CCommandDispatchTask::CCommandPortSender::transBuff_[TRANS_BUFF_LEN];

CCommandDispatchTask::CCommandDispatchTask():CTask(NAMECODE_CommandDispatchTask)
{
	memset(_commandBuffer,0,INSTRUCT_BUFF_LEN);
}

int CCommandDispatchTask::doRun()
{
	if ((true == firstIn) && (true == NetworkAdapter::Instance()->isOpen()))
	{
		firstIn = false;
	}

	if (false == NetworkAdapter::Instance()->isOpen())
	{
		return 2;
	}

	udp_cfg(LOCAL_PORT, 0);

	return 0;
}


void CCommandDispatchTask::udp_cfg(uint16_t port, uint16_t mode)
{
	int32_t len;
	sockaddr_in remoteaddr;
	socklen_t remoteaddrlen; 
	
	if(socket_n < 0)//init process
	{
		sockaddr_in localaddr;
	
		localaddr.sin_family = AF_INET;
		localaddr.sin_port = htons(CCommandDispatchTask::LOCAL_PORT);
		localaddr.sin_addr.s_addr = INADDR_ANY;
		
		socket_n = net::socket(AF_INET, SOCK_DGRAM, 0);
		net::bind(socket_n, (sockaddr *)&localaddr,  sizeof(localaddr));
		Console::Instance()->printf("Init CCommandDispatchTask socket: [%d]\r\n", socket_n);
	}
	
	len = net::recvfrom(socket_n, _commandBuffer, INSTRUCT_BUFF_LEN, MSG_DONTWAIT, (sockaddr *)&remoteaddr, &remoteaddrlen);
	if( len > 0) // check the size of received data
	{
		len = len > INSTRUCT_BUFF_LEN ? INSTRUCT_BUFF_LEN : len;

		// get command type
		int32_t int_ct;
		memcpy(&int_ct, _commandBuffer, sizeof(int_ct));
		//Console::Instance()->printf("config type [%d]\r\n", int_ct);
		
		memcpy(destip, &remoteaddr.sin_addr.s_addr, 4);
		destport = ntohs(remoteaddr.sin_port);

		bool findHandler = false;
		int ret;

		for(int i = 0; i < MAX_HANDLER_NUM; i++)
		{
			if(int_ct == CommandHandlerTab[i].ECommand && CommandHandlerTab[i].pHandler != NULL)
			{
				if(int_ct > 0x1000)
				{
					memcpy(CmdSocket::Instance()->cmderIP_, destip, 4);
					CmdSocket::Instance()->cmderPort_ = destport;
				}
				else
				{
					memcpy(CmdSocket::Instance()->rbkIP_, destip, 4);
					CmdSocket::Instance()->rbkPort_ = destport;
				}
				CmdSocket::Instance()->lastCmdTyp_ = int_ct;
			
				ret = CommandHandlerTab[i].pHandler->handle(_commandBuffer + sizeof(uint32_t), len - sizeof(uint32_t));
				findHandler = true;
				break;
			}
		}
		
		int32_t* retBuff = (int32_t*)_commandBuffer;
		
		if(findHandler)
		{
			retBuff[0] = 0;
		}
		else
		{
			retBuff[0] = -1;
		}
		
		if(int_ct > 0xA000 || !findHandler)
		{
			uint16_t retMsgLen;
			if(!findHandler) // return 0
				retMsgLen = sizeof(uint32_t);
			else if (ret < 0) // return 0, -1
				retMsgLen = 2 * sizeof(uint32_t);
			else
				retMsgLen = ret + sizeof(uint32_t);
		
			ret = net::sendto(socket_n, _commandBuffer, retMsgLen, MSG_DONTWAIT, (sockaddr *)&remoteaddr, remoteaddrlen);
		}
	}

}

/*******************************************************************************
****      this part is for CCommandDispatchTask::CCommandPortSender        *****
********************************************************************************/
int CCommandDispatchTask::CCommandPortSender::sendto(uint32_t len, uint8_t* ip, uint16_t port)
{
	
	int32_t ret = 0;
	
	sockaddr_in remoteaddr;
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_port = htons(port);
	remoteaddr.sin_addr.s_addr = byteston(ip);
	
//	ret = net::sendto(socket_n, transBuff_, len, ip, port);
	ret = net::sendto(socket_n, transBuff_, len, MSG_DONTWAIT, (sockaddr *)&remoteaddr, sizeof(remoteaddr));
	
	if(ret < 0)
	{
//		isInitialized_ = false;
	}
	return ret;
}

CCommandDispatchTask::CCommandPortSender::CCommandPortSender()
//	:isInitialized_(false)
{
//	uint8_t broadcast_ip[4] = {0, 0, 0, 0};
	uint8_t broadcast_ip[4] = {192, 168, 192, 4};
	uint8_t* localIP = NetworkAdapter::Instance()->IP();
	uint8_t* localNetMask = NetworkAdapter::Instance()->NetMask();
	memcpy(cmderIP_, broadcast_ip, 4);
	memcpy(rbkIP_, broadcast_ip, 4);
}

int CCommandDispatchTask::CCommandPortSender::sock_n()
{
	return socket_n;
}

/*******************************************************************************
****      this part is for some single command        *****
********************************************************************************/
int chipResetHandlerFunc(uint8_t* msg, uint16_t len)
{
	if(len != sizeof(uint32_t))
		return -1;

	CmdSocket::Instance()->transParam(0) = CmdSocket::Instance()->lastCmdTyp();
	CmdSocket::Instance()->transParam(1) = *(uint32_t*)msg;
	CmdSocket::Instance()->sendto(len, CmdSocket::Instance()->cmderIP(), CmdSocket::Instance()->cmderPort());

	Console::Instance()->printf("Get reset command!\r\n");
	while(!Console::Instance()->isIdle());
	NVIC_SystemReset();
	return 0;
}

int iapHandlerFunc(uint8_t* msg, uint16_t len)
{
	pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_LOADING);
	Console::Instance()->printf("Jump to bootloader...\r\n");
	CmdSocket::Instance()->transParam(0) = CmdSocket::Instance()->lastCmdTyp();
	CmdSocket::Instance()->transParam(1) = 0;
	CmdSocket::Instance()->sendto(2 * sizeof(uint32_t), CmdSocket::Instance()->cmderIP(), CmdSocket::Instance()->cmderPort());
	
	Console::Instance()->printf("Jump to bootloader...\r\n");
	while(!Console::Instance()->isIdle());
	NVIC_SystemReset();
	return 0;
}

//end of file
