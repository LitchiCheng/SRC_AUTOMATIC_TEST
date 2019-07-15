#ifndef _CANROUTER_TASK_H_
#define _CANROUTER_TASK_H_
#include "Task.h"
#include "Timer.h"

class CCanRouterTask : public CTask
{
public:
	CCanRouterTask():CTask(NAMECODE_CanRouterTask),firstIn_(true){};
	virtual int doRun();
	virtual void initial(){}
	
		
private:
	bool firstIn_;
};

int queryErrorRateCallback(uint8_t* argv, uint16_t argc);

#endif
//end of file
