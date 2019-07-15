#include "TaskManager.h"


#include "CanRouterTask.h"

#include "CommandDispatchTask.h"

#include "ControllerSurveyorTask.h"

uint32_t CTaskManager::_taskTimeArray[MAX_TASK_NUM];
int32_t CTaskManager::_taskTimeTickArray[MAX_TASK_NUM];

CTaskManager::CTaskManager()
	:_statisticTimer(this, &CTaskManager::statisticTimerCallback, 1000, true),
	_showTaskState(0)
{
	_taskNum = 0;
	_runNum = 0;
	for (int i = 0;i < MAX_TASK_NUM;++i)
	{
		_taskPool[i] = 0;//NULL
	}
	_statisticTimer.enable();
}

void CTaskManager::statisticTimerCallback()
{
	int16_t mostCostTaskId = -1;
	uint32_t mostCostTaskTimeNs = 0;
	uint32_t systickperiod = BaseTimer::Instance()->systickPeriod();
	for(int i = 0; i < _runNum; i++)
	{
		_taskTimeArray[i] *= 1000000ul;
		_taskTimeArray[i] += (_taskTimeTickArray[i] * 1000000ull / systickperiod);
		if(_taskTimeArray[i] > mostCostTaskTimeNs)
		{
			mostCostTaskTimeNs = _taskTimeArray[i];
			mostCostTaskId = _runPool[i]->getIdx();
		}
	}
	const uint16_t TASK_LOOP_LOW_LIMIT_ERROR = 6000;
	const uint16_t TASK_LOOP_LOW_LIMIT_WARNING = TASK_LOOP_LOW_LIMIT_ERROR + 2000;
	if(_taskLoopCount < TASK_LOOP_LOW_LIMIT_WARNING || _showTaskState)
	{
//		Console::Instance()->printf("low task frequence: %d ticks/s, most cost task: %d time: %fms\r\n",
//			_taskLoopCount, mostCostTaskId, mostCostTaskTimeNs / 1000000.0f);
		if(_taskLoopCount < TASK_LOOP_LOW_LIMIT_ERROR)
		{
			
		}
		else if (_taskLoopCount < TASK_LOOP_LOW_LIMIT_WARNING)
		{
			
		}


		
		uint64_t sum = 0;
		for(int i = 0; i < _runNum; i++)
		{
			sum += _taskTimeArray[i];
			Console::Instance()->printf("task: %d: %f ms\r\n", _runPool[i]->getIdx(), _taskTimeArray[i]/1000000.0f);
		}
		Console::Instance()->printf("Total : %fms\r\n", sum/1000000.0f);
	} else {

	}


	_taskLoopCount  = 0;
	if(_showTaskState) _showTaskState--;
	memset(_taskTimeArray, 0, MAX_TASK_NUM * sizeof(uint32_t));
	memset(_taskTimeTickArray, 0, MAX_TASK_NUM * sizeof(uint32_t));
}

void CTaskManager::addInTaskPool(int taskID)
{
	for (int i = 0;i < _taskNum;++i)
	{
		if(taskID == _taskPool[i]->getIdx())
		{
			_taskPool[i]->initial();
			return;
		}
	}
	
	if (taskID == NAMECODE_ControllerSurveyorTask)_taskPool[_taskNum] = new CControllerSurveyorTask();
	else if (taskID == NAMECODE_CommandDispatchTask)_taskPool[_taskNum] = new CCommandDispatchTask();
	else if (taskID == NAMECODE_CanRouterTask) _taskPool[_taskNum] = new CCanRouterTask();

	else
	{
		Console::Instance()->printf("Cannot find task with ID %d\r\n", taskID);
		return ;
	}
	_taskPool[_taskNum++]->initial();
}

void CTaskManager::addInRunPool(int taskID)
{
	for(int i = 0; i < _runNum; i++)
	{
		if(taskID == _runPool[i]->getIdx())
		{
			Console::Instance()->printf("Task with ID %d alredy in runPool\r\n", taskID);
			return;
		}
	}
	
	
	for (int i = 0;i < _taskNum;++i)
	{
		if(taskID == _taskPool[i]->getIdx())
		{
			_runPool[_runNum++] = _taskPool[i];
			Console::Instance()->printf("Add task[%d] with ID: %d\r\n", _runNum - 1, taskID);
			return;
		}
	}
	
	// output an error!!
	Console::Instance()->printf("Cannot find task with ID: %d\r\n", taskID);

	return;
}

void CTaskManager::addTask(int taskID)
{
	addInTaskPool(taskID);
	addInRunPool(taskID);
}

void CTaskManager::resetRunPool()
{
	for (int i = 0;i < _runNum;++i)
	{
		_runPool[i]->closeTask();
	}
	
	// reset run pool number
	_runNum = 0;

	// add sys tasks to run pool.
	for (int i = 0;i < _taskNum;++i)
	{
		if(NAMECODE_IS_SYS_TASK(_taskPool[i]->getIdx()))
		{
			_runPool[_runNum++] = _taskPool[i];
		}
	}
}

void CTaskManager::runTasks()
{
	static int32_t prevTick = SysTick->VAL;
	static uint32_t prevTime = BaseTimer::Instance()->getTime();

	int16_t delta_ms;
	int32_t delta_tick;

	uint32_t currTime0 = BaseTimer::Instance()->getTime();
	int32_t currTick = SysTick->VAL;
	uint32_t currTime1 = BaseTimer::Instance()->getTime();

	if(currTime0 != currTime1 && currTick > (BaseTimer::Instance()->systickPeriod() >> 1))
		++currTime0;

	prevTick = currTick;
	prevTime = currTime0;

	for (int i = 0;i < _runNum;++i)
	{
		int ret = _runPool[i]->doRun();
		if (0 != ret)
		{
			//do ?
		}

		currTime0 = BaseTimer::Instance()->getTime();
		currTick = SysTick->VAL;
		currTime1 = BaseTimer::Instance()->getTime();

		if(currTime0 != currTime1 && currTick > (BaseTimer::Instance()->systickPeriod() >> 1))
			++currTime0;

		delta_ms = (currTime0 - prevTime);
		delta_tick = (currTick < prevTick) ? (prevTick - currTick) : 					//systick count down
			(--delta_ms, prevTick + BaseTimer::Instance()->systickPeriod() - currTick); //borrow a bit
		_taskTimeArray[i] += delta_ms;
		_taskTimeTickArray[i] += delta_tick;


		prevTick = currTick;
		prevTime = currTime0;
		if(delta_ms > 1)
		{
		}
	}

	_taskLoopCount++;

}

void CTaskManager::printRunPool()
{
	Console::Instance()->printf("========== RunTaskPool ==========\r\n");
	for (int i = 0;i < _runNum;++i)
	{
		Console::Instance()->printf("ID of Task[%d] = %d\r\n", i, _taskPool[i]->getIdx());
	}
}

int CTaskManager::addTaskCommandHandler(uint8_t* argv, uint16_t argc)
{
	if(argc != sizeof(uint32_t))
		return -1;

	uint32_t taskID = *(uint32_t*)argv;

	addTask(taskID);

	return 0;
}

int CTaskManager::taskStateCommandHandler(uint8_t* argv, uint16_t argc)
{
	if(argc != sizeof(uint32_t))
		return -1;

	uint32_t showTime = *(uint32_t*)argv;

	_showTaskState = showTime;

	return 0;
}
