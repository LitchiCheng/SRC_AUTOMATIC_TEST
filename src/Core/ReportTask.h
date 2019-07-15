#ifndef _FW_REPORT_TASK_H_
#define _FW_REPORT_TASK_H_

#include "Task.h"
#include <stdint.h>
#include "Communication.h"
#include "Timer.h"
#include "socket.h"

#define REPORT_DATA_LENGTH	120	//注意这个值要够用
class CReportTask : public CTask
{
public:
	CReportTask():CTask(NAMECODE_ReportTask),_t(1,30)//33ms
	{}

	virtual int doRun();

private:
	void udp_rpt(SOCKET s, uint16_t port, uint16_t mode);
private:
	Timer _t;
};

#endif
