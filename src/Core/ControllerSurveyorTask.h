#ifndef _CONTROLLER_SURVEYOR_TASK_H_
#define _CONTROLLER_SURVEYOR_TASK_H_

#include "Task.h"
#include "Timer.h"
#include "CUsart.h"
#include "CCan.h"
#include "fixed_vector.h"
#include "socket.h"

#define CONTROLLER_SURVEYOR_TEST_COM_NUM	4

class CControllerSurveyorTask : public CTask
{
public:
	CControllerSurveyorTask();
	virtual ~CControllerSurveyorTask();
	virtual int doRun();
	virtual void initial();
	void udpRecieveRun();
	void udpTx(uint8_t* tx_buff, uint16_t tx_len);

private:
	Timer _t;
	// for serial ports
	Timer _comTxTimer;
	// for can
	Timer _canTxTimer;
	CCanRouter& _canRouter_1;
	sockaddr_in remoteaddr;
	socklen_t remoteaddrlen; 
};

#endif
