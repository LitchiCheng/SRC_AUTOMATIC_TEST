#include "NetworkAdapter.h"
#include "lan8720.h"
#include "Console.h"
#include "lwip_comm.h"
CNetworkAdapter::CNetworkAdapter():_periodTimer(this, &CNetworkAdapter::periodHandler, 1, true), _isOpen(false)
{
	
}

int CNetworkAdapter::doInit()
{
	int netret = initComponents();
	while(netret < 0)
	{
		for(int i = 0;i > netret;--i)
		{
			GPIO_SetBits(GPIOG, GPIO_Pin_10);
			GPIO_SetBits(GPIOE, GPIO_Pin_2);
			BaseTimer::Instance()->delay_ms(500);
			GPIO_ResetBits(GPIOG, GPIO_Pin_10);
			GPIO_ResetBits(GPIOE, GPIO_Pin_2);
			BaseTimer::Instance()->delay_ms(500);
		}
		
		GPIO_ResetBits(GPIOE, GPIO_Pin_2);
		GPIO_ResetBits(GPIOG, GPIO_Pin_10);
		BaseTimer::Instance()->delay_ms(2500);
	}
	
	uint8_t speed = LAN8720_Get_Speed();
	if(speed&1<<1)Console::Instance()->printf("Ethernet Speed:100M\r\n");
	else Console::Instance()->printf("Ethernet Speed:10M\r\n");
	
	setOpen(true);
	return 0;
}

int CNetworkAdapter::initComponents()
{
	_periodTimer.enable();
	
	int ret;
	ret = LAN8720_Init();
	if(ret <0 )//初始化LAN8720失败
	{
		Console::Instance()->printf("LAN8720 Init failed, code %d\r\n", ret);
		return ret;
	}			
	ret = lwip_comm_init();
	if(ret <0 )
	{
		Console::Instance()->printf("lwIP Init failed!\r\n");
		return ret;
	}
	
	return 0;
}

int CNetworkAdapter::doRun()
{
	return 0;
}

int CNetworkAdapter::queryIpInfoHandler(uint8_t* msg, uint16_t len)
{
	return 0;
}

int CNetworkAdapter::setIpHandler(uint8_t* pbData, uint16_t len)
{
	return 0;
}

int CNetworkAdapter::RstCommandHandler(uint8_t* argv, uint16_t argc)
{
	return 0;
}

uint8_t* CNetworkAdapter::IP()
{
	return (uint8_t*)"\xC0\xA8\xC0\x04";
}

uint8_t* CNetworkAdapter::NetMask()
{
	return (uint8_t*)"\xFF\xFF\xFF\x00";
}

void CNetworkAdapter::setOpen(bool val)
{
	_isOpen = val;
}

bool CNetworkAdapter::isOpen()
{
	return _isOpen;
}

void CNetworkAdapter::periodHandler()
{
	lwip_periodic_handle();
}

//end of file
