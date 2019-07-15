#include "stm32f4xx.h"
#include "CommonConfig.h"
#include <stdint.h>
#include "TaskManager.h"
#include "Timer.h"
#include "NetworkAdapter.h"

#include "GlobalData.h"

#include "stmflash.h"
#include "F4Kxx_BSPmacro.h"
#include "powerupOption.h"
#include "CommandDispatchTask.h"



int main(void)
{
	SCB->VTOR = FLASH_APP1_ADDR;
	NVIC_CONFIG();
	
	BaseTimer::Instance()->initialize();
	pvf::initInApp();
	
	NetworkAdapter::Instance()->doInit();
	
	TaskManager::Instance()->addTask(NAMECODE_CommandDispatchTask);
	TaskManager::Instance()->addTask(NAMECODE_CanRouterTask);
	TaskManager::Instance()->addTask(NAMECODE_ControllerSurveyorTask);

	InitWatchDog(2000);
	Timer tt(1000,1000);

	while(1)
	{
		TaskManager::Instance()->runTasks();
		ServiceDog();
		BaseTimer::Instance()->doRun();
	}
}


void assert_failed(uint8_t* file, uint32_t line)
{

}


