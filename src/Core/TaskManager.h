#ifndef _FW_TASK_MANAGER_H_
#define _FW_TASK_MANAGER_H_

#include "Task.h"
#include "Singleton.h"
#include <stdint.h>
#include "Timer.h"

#define MAX_TASK_NUM 32

class CTaskManager
{
public:
	CTaskManager();
//	~CTaskManager(){};

	void addTask(int taskID);	// only add, not delete
	void runTasks();

	void resetRunPool();
	void printRunPool();

	int addTaskCommandHandler(uint8_t* argv, uint16_t argc);
	int taskStateCommandHandler(uint8_t* argv, uint16_t argc);

private:
	void addInTaskPool(int taskID);
	void addInRunPool(int taskID);

private:
	CTask* _taskPool[MAX_TASK_NUM];
	CTask* _runPool[MAX_TASK_NUM];

	static uint32_t _taskTimeArray[MAX_TASK_NUM];
	static int32_t _taskTimeTickArray[MAX_TASK_NUM];
	uint16_t _taskLoopCount;

	CAsyncTimer<CTaskManager> _statisticTimer;
	void statisticTimerCallback();

	int _taskNum;
	int _runNum;

	uint8_t _showTaskState;
};
typedef NormalSingleton<CTaskManager> TaskManager;

#endif
