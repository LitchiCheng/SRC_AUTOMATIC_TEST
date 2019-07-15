#ifndef _GLOBAL_DATA_H_
#define _GLOBAL_DATA_H_

#include "Communication.h"
#include <stdint.h>
#include "Console.h"
namespace GlobalData{ //以下数据的具体存放位置待考虑，是放在具体模块中还是这里？

	extern CChassisControl ChasCtrl;
	
	extern float ChasCtrl_steerAngle;

	extern float LiftRotationCtrl_spinSpeed;
	
	extern CReportData RptDat;

	extern CDeviceDemandConfig DevDemCfg;	

	extern CChassisStructConfig ChasStruCfg;

	extern CChassisStructConfig4cp ChasStruCfg4cp;

	extern bool HostStatus;	//何时置为false

	extern uint8_t destip[4];
//	extern uint8_t destReplyIp[4];
//	extern uint16_t destReplyPort;

	namespace TaskCom{
		//extern bool b_ChassisTask_Wait;//尚未启用
		extern bool b_CommandTask_ChassisTask_newCmd;
		extern bool b_Gyro_gyroCalibration;
		extern bool b_Gyro_autoCalibrationSwitch;
		extern bool b_Chassis_emergencyButton;
		extern bool b_Chassis_brakeSwitch;
		extern bool b_Chassis_brakeState;
	}
}

namespace GlobalControl
{
	void newSpeedSignal();

	bool isNewSpeedCatched();

	void setGyroCaliFlagOn();

	void setGyroCaliFlagOff();

	bool getGyroCaliFlag();

	void setGyroAutoCaliSwitchOn();

	void setGyroAutoCaliSwitchOff();

	bool getGyroAutoCaliState();
	
	/// software emergency from driver protocol !!!
	void setChassisEmergencyButtonOn();

	void setChassisEmergencyButtonOff();

	bool getChassisEmergencyButton();
	/// software emergency from driver protocol !!!

	void setChassisBrakeSwitchOn();

	void setChassisBrakeSwitchOff();

	bool getChassisBrakeSwitchState();

	void setChassisBrakeStateOn();

	void setChassisBrakeStateOff();

	bool getChassisBrakeState();

}

#endif
