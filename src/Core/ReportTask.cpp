#include "ReportTask.h"
//#include "W5500Device.h"
#include "GlobalData.h"
#include "socket.h"

//#include <assert.h>

namespace{
	uint32_t len = sizeof(CReportData);
	uint16_t destport = 5002;
}

int CReportTask::doRun()
{
//	if (false == W5500Device::Instance()->isOpen())
//	{
//		return 1;
//	}
//	if (false == GlobalData::HostStatus)
//	{
//		return 2;
//	}
	udp_rpt(CW5500Device::REPORT_SKT, 5002, 0);
	return 0;
}

void CReportTask::udp_rpt(SOCKET s, uint16_t port, uint16_t mode)
{
	switch(getSn_SR(s))
	{
	case SOCK_UDP:
		if(0 == (getPHYCFGR() & 0x1))// 断线
			close(s);
		if (true == _t.isAbsoluteTimeUp())
		{
			GlobalData::RptDat.packetCount++;
			GlobalData::RptDat.packetTimestamp = BaseTimer::Instance()->getTime();
			//set _rptBuffer
			//assert(len < REPORT_DATA_LENGTH);

			//md5
			//			md5_ctx context;
			//			uint8_t  digest[16];//以下几行代码中的16是md5用
			//
			//			md5_init(&context);
			//			md5_update(&context, _rptBuffer, len);
			//			md5_final(digest, &context);
			//
			//			MemCpy(_rptBuffer+len,digest,16);
			
			int32_t ret = 0;
			if ((ret = sendto(s,(uint8_t*)&GlobalData::RptDat,len,GlobalData::destip,destport)) != len)//send failed!!!sendLen是byte值
			{
				Console::Instance()->printf("buflen = %d, actually send = %d\r\n", len, ret);
			}
		}
		break;
		// -----------------
	case SOCK_CLOSED:                                  // CLOSED
		close(s);                                       // close the SOCKET
		if(0 != (getPHYCFGR() & 0x1))// 没有断线
			socket(s,Sn_MR_UDP,port,mode);                  // open the SOCKET with UDP mode
		break;
	default:
		break;
	}
}
