#ifndef _REPLIER_H_
#define _REPLIER_H_
#include <stdint.h>

namespace Replier
{
//	typedef enum 
//	{
//		NULL_DEV = 0,
//		UART_DEV,
//		CAN_DEV,
//		RTT_DEV,
//		UDP_DEV,
//		NUM_OF_DEV
//	}DevType_t;
//	
//	typedef struct
//	{
//		DevType_t type;
//		uint32_t idx;
//		uint32_t addr[3];
//	}HostAddr_t;
	
	uint32_t append(void* addr, void* src, uint16_t len);
	uint32_t append(void* addr, int32_t val);
	uint32_t append(void* addr, float val);
	
	void init(void* addr);
	int32_t except(void* addr, int32_t val);
}
#endif
//end of file
