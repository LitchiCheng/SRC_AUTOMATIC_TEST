#include "CanRouterTask.h"
#include "CCan.h"
#include "F4Kxx_BSPmacro.h"
#include "Console.h"
namespace
{
	const uint8_t TX_QUEBUF_SIZE = 35;
	CanTxMsg router1_txQueBuf[TX_QUEBUF_SIZE];
	CanTxMsg router2_txQueBuf[TX_QUEBUF_SIZE];
};
CCanRouter CanRouter1(CAN1, router1_txQueBuf, TX_QUEBUF_SIZE, 250000);
CCanRouter CanRouter2(CAN2, router2_txQueBuf, TX_QUEBUF_SIZE, 250000);


int CCanRouterTask::doRun()
{
	if(firstIn_)
	{
		CanRouter2.InitCan();
		CanRouter2.InitCanGpio(F4KXX_CAN2_IOGROUP);
		CanRouter1.InitCan();
		CanRouter1.InitCanGpio(F4KXX_CAN1_IOGROUP);		
		firstIn_ = false;
	}
	CanRouter1.runTransmitter();
	CanRouter1.runReceiver();	
	CanRouter2.runTransmitter();
	CanRouter2.runReceiver();	
	return 0;
}

int queryErrorRateCallback(uint8_t* argv, uint16_t argc)
{
	Console::Instance()->printf(
		"CAN1 TEC = %d, REC = %d, prvErr = 0x%02X, \
		CAN2 TEC = %d, REC = %d, prvErr = 0x%02X\r\n",
		CAN_GetLSBTransmitErrorCounter(CAN1), 
		CAN_GetReceiveErrorCounter(CAN1), 
		CAN_GetLastErrorCode(CAN1),
		CAN_GetLSBTransmitErrorCounter(CAN2), 
		CAN_GetReceiveErrorCounter(CAN2), 
		CAN_GetLastErrorCode(CAN2));
	return 0;
}
//end of file
