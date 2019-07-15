#include "GlobalData.h"

namespace GlobalData{

	float PI = 3.1415926;

	CChassisControl ChasCtrl;
	float ChasCtrl_steerAngle;

	CReportData RptDat;

	float LiftRotationCtrl_spinSpeed;
	
	CDeviceDemandConfig DevDemCfg;

	CChassisStructConfig ChasStruCfg;

	CChassisStructConfig4cp ChasStruCfg4cp;

	bool HostStatus = false;

	
	namespace TaskCom{
		bool b_CommandTask_ChassisTask_newCmd = false;
		bool b_Gyro_gyroCalibration = false;
		bool b_Gyro_autoCalibrationSwitch = false;
		bool b_Chassis_emergencyButton = false;
		bool b_Chassis_brakeSwitch = false;
		bool b_Chassis_brakeState = false;
	}
}

namespace GlobalControl
{
	using namespace GlobalData;

	void newSpeedSignal()
	{
		TaskCom::b_CommandTask_ChassisTask_newCmd = true;
	}

	bool isNewSpeedCatched()
	{
		bool ret = TaskCom::b_CommandTask_ChassisTask_newCmd;

		//clear flag
		if (ret) TaskCom::b_CommandTask_ChassisTask_newCmd = false;

		return ret;
	}

	void setGyroCaliFlagOn()
	{
		TaskCom::b_Gyro_gyroCalibration = true;
	}

	void setGyroCaliFlagOff()
	{
		TaskCom::b_Gyro_gyroCalibration = false;
	}

	bool getGyroCaliFlag()
	{
		return TaskCom::b_Gyro_gyroCalibration;
	}

	void setGyroAutoCaliSwitchOn()
	{
		TaskCom::b_Gyro_autoCalibrationSwitch = true;
	}

	void setGyroAutoCaliSwitchOff()
	{
		TaskCom::b_Gyro_autoCalibrationSwitch = false;
	}

	bool getGyroAutoCaliState()
	{
		return TaskCom::b_Gyro_autoCalibrationSwitch;
	}

	void setChassisEmergencyButtonOn()
	{
		TaskCom::b_Chassis_emergencyButton = true;
	}

	void setChassisEmergencyButtonOff()
	{
		TaskCom::b_Chassis_emergencyButton = false;
	}

	bool getChassisEmergencyButton()
	{
		return TaskCom::b_Chassis_emergencyButton;
	}

	void setChassisBrakeSwitchOn()
	{
		TaskCom::b_Chassis_brakeSwitch = true;
	}

	void setChassisBrakeSwitchOff()
	{
		TaskCom::b_Chassis_brakeSwitch = false;
	}

	bool getChassisBrakeSwitchState()
	{
		return TaskCom::b_Chassis_brakeSwitch;
	}

	void setChassisBrakeStateOn()
	{
		TaskCom::b_Chassis_brakeState = true;
	}

	void setChassisBrakeStateOff()
	{
		TaskCom::b_Chassis_brakeState = false;
	}

	bool getChassisBrakeState()
	{
		return TaskCom::b_Chassis_brakeState;
	}
}

