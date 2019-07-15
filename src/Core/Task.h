#ifndef _FW_TASK_H_
#define _FW_TASK_H_

// system task

#define NAMECODE_ReportTask						2

#define NAMECODE_CanRouterTask					6

#define NAMECODE_CommandDispatchTask			9



#define NAMECODE_IS_SYS_TASK(x)					(x < 100)

// user task
#define NAMECODE_ControllerSurveyorTask			112


class CTask
{
public:	
	CTask(int id):idx(id){}
	virtual ~CTask(){}
	int getIdx(){return idx;}
	virtual int doRun() = 0;		//if run normally, return 0
	virtual void initial() {}		
	virtual void closeTask() {}		//这不是一个完整实现的closeTask函数，只是在重启任务池的时候回收资源
private:
	int idx;
};


#endif // !_FW_TASK_H_
