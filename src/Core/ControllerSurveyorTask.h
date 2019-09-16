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
	virtual ~CControllerSurveyorTask(){}
	virtual int doRun();
	virtual void initial();
	void udpRecieveRun();
	void udpTx(uint8_t* tx_buff, uint16_t tx_len);
	void detectWarningLight();
	void detectBrake();
	void detectDelay();
	void detectEmergency();
	void detectPDORun();
	void detectPCLight();
	void uartChangeRun();
	void brakeOrDIRun();
	void chargeAndEmergencyRun();
	void openPCRun();
	void closePCRun();
	void canTest2000();
	void uart485Run();
	void uart232Run();
	void CanDispatchRun();
		
private:
	Timer _t;
	CCanRouter& _canRouter_1;
	sockaddr_in remoteaddr;
	socklen_t remoteaddrlen; 
};
#endif
