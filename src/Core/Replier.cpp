#include "Replier.h"
#include <string.h>

// 
// return:
//	> 0: return a value, length is return
//  = 0: return None
//  < 0: exception
//


uint32_t Replier::append(void* addr, int32_t val)
{
	return append(addr, &val, sizeof(int32_t));
}

uint32_t Replier::append(void* addr, float val)
{	
	return append(addr, &val, sizeof(float));
}

void Replier::init(void* addr)
{
		*(uint32_t*)addr = sizeof(uint32_t);
}

int32_t Replier::except(void* addr, int32_t val)
{
	*(int32_t*)addr = val;
	return val;
}

uint32_t Replier::append(void* addr, void* src, uint16_t len)
{
	uint32_t datalen = *(uint32_t*)addr;
	memcpy((uint8_t*)addr + datalen, src, len);
	*(uint32_t*)addr += len;
	
	return *(uint32_t*)addr;
}
//end of file
