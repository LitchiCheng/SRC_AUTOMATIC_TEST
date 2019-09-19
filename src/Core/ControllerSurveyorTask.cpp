#include "ControllerSurveyorTask.h"
#include "GlobalData.h"
#include "Console.h"
#include "stdlib.h"
#include "string.h"
#include "DIO_TEST.h"

static void ledGpioInit();
static void ledRun();

namespace{
	const uint16_t RPT_DESTPORT = 5002;
	sockaddr_in destaddr;
	sockaddr_in localaddr;
	uint8_t destip[4];
	uint16_t destport;
	int socket_n = -1;
	const uint16_t BUFF_SIZE = 300;
	uint8_t udp_tx_buff[BUFF_SIZE];
	uint8_t udp_rx_buff[BUFF_SIZE];
}

namespace
{
	/**********************************************************************************/
    const uint8_t SURVEYOR_RX_MAILBOX_QUE_SIZE = 2;
	CCanRxMailbox::MsgData rxMailboxBuf[2][SURVEYOR_RX_MAILBOX_QUE_SIZE];
	CCanRxMailbox can_mailbox_test_2000(rxMailboxBuf[0], SURVEYOR_RX_MAILBOX_QUE_SIZE);
	CCanRxMailbox can_dispatch(rxMailboxBuf[1], SURVEYOR_RX_MAILBOX_QUE_SIZE);
	/**********************************************************************************/
	
	/*****************************************************************/
	const uint8_t SURVEYOR_RX_DMA_BUF_LEN_232 = 5;
	const uint8_t SURVEYOR_RX_DMA_BUF_LEN_458 = 2;
	uint8_t buffer_485[SURVEYOR_RX_DMA_BUF_LEN_458];	
	CUsart usart3_test_2000(USART3, buffer_485, SURVEYOR_RX_DMA_BUF_LEN_458);
	/***************************************************************/
	uint8_t buffer_232[SURVEYOR_RX_DMA_BUF_LEN_232];	
	CUsart usart2_test_2000(USART2, buffer_232, SURVEYOR_RX_DMA_BUF_LEN_232);
	/****************************************************************/
	
	DIO_TEST di_test_2000_do;
	DIO_TEST do_test_2000_di;

	static bool open_pc_switch = true;
	static bool close_pc_switch = false;
	static bool charge_and_emergency_switch = false;
	static bool is_brake_or_di_switch = false;
	static bool detect_pdo_switch = false;
	static bool detect_pc_light = false;
	static bool detect_emergency = false;
	static bool detect_brake = false;
	static bool is_232_or_485_mode = false;
	static bool can_test_switch = false;
	static bool rs485_test_switch = false;
	static bool rs232_test_switch = false;
	static bool detect_delay = false;
	static bool detect_warninglight = false;
	
	uint8_t rs485_transfer_data[5] = {0x00, 0xAA, 0xAA, 0xAA, 0xAA};
	uint8_t rs232_transfer_data[5] = {0x00, 0xBB, 0xBB, 0xBB, 0xBB};
	uint8_t rs485_recieve_0_data[5] = {0x01, 0xCC, 0xCC, 0xCC, 0xCC};
	uint8_t rs485_recieve_1_data[5] = {0x02, 0xCC, 0xCC, 0xCC, 0xCC};
	uint8_t rs485_recieve_2_data[5] = {0x03, 0xCC, 0xCC, 0xCC, 0xCC};
	uint8_t rs485_recieve_3_data[5] = {0x04, 0xCC, 0xCC, 0xCC, 0xCC};
	uint8_t rs232_recieve_3_data[5] = {0x04, 0xDD, 0xDD, 0xDD, 0xDD};
	
	static bool need_console_printf = true;
	
	static bool is_485_test_ok[4] = {false, false, false, false};
	static bool is_232_test_ok = false;
	static bool is_can1_test_ok = false;
	static bool is_can2_test_ok = false;
	static bool is_all_pdo_on = false;
	static bool is_all_pdo_off = false;
	static bool is_boot_light_on = false;
	static bool is_manual_charge_ok = false;
	static bool is_auto_charge_ok = false;
	static bool is_emc_out1_ok = false;
	static bool is_emc_out2_ok = false;
	static bool is_emc_light_on = false;
	static bool is_2000_di_on = false;
	static bool is_2000_di_off = false;
	static bool is_brake12_ok = false;
	static bool is_brake24_ok = false;
	static bool is_delay_ok = false;
	static bool is_warning_light_on = false;
	
	static bool can_dispatch_switch = true;
	static uint32_t can_dispatch_times_counter = 0;
	uint8_t can_test_case_select = 0xFF;
}

static void resetAllSwitch()
{
	open_pc_switch = false;
	close_pc_switch = false;
	charge_and_emergency_switch = false;
	is_brake_or_di_switch = false;
	detect_pdo_switch = false;
	detect_pc_light = false;
	detect_emergency = false;
	detect_brake = false;
	can_test_switch = false;
	rs485_test_switch = false;
	rs232_test_switch = false;
	detect_delay = false;
	detect_warninglight = false;
}

static void resetAllResult()
{
	for(int i=0;i<4;i++){
		is_485_test_ok[i] = false;
	}
	is_232_test_ok = false;
	is_can1_test_ok = false;
	is_can2_test_ok = false;
	is_all_pdo_on = false;
	is_all_pdo_off = false;
	is_boot_light_on = false;
	is_manual_charge_ok = false;
	is_auto_charge_ok = false;
	is_emc_out1_ok = false;
	is_emc_out2_ok = false;
	is_emc_light_on = false;
	is_2000_di_on = false;
	is_2000_di_off = false;
	is_brake12_ok = false;
	is_brake24_ok = false;
	is_delay_ok = false;
	is_warning_light_on = false;
}

CControllerSurveyorTask::CControllerSurveyorTask(): CTask(NAMECODE_ControllerSurveyorTask),_t(2000,2000),_canRouter_1(CanRouter1)
{
}

void CControllerSurveyorTask::initial()
{
	_t.reset();
	
	ledGpioInit();		//心跳灯初始化
	
	usart3_test_2000.setBaudrate(115200);	//串口初始化，默认485
	usart3_test_2000.InitSciGpio();
	usart3_test_2000.InitSci();
	
	usart2_test_2000.setBaudrate(115200);	//232串口初始化
	usart2_test_2000.InitSciGpio();
	usart2_test_2000.InitSci();
	
    can_mailbox_test_2000.setStdId(1);
    can_mailbox_test_2000.attachToRouter(_canRouter_1);
	
	can_dispatch.setStdId(0xD1);
	can_dispatch.attachToRouter(_canRouter_1);
	
	di_test_2000_do.open();
	do_test_2000_di.open();

	resetAllSwitch();
	resetAllResult();
}

int CControllerSurveyorTask::doRun()
{
	/******************心跳灯*************************/
	ledRun();
	/**************************************************/
	
	/*****************网络接收*************************/
	udpRecieveRun();
	/**************************************************/
	
	/*********************任务调度************************/
	CanDispatchRun();
	/****************************************************/
	
	/******************开关机动作**********************/
	openPCRun();
	closePCRun();
	/*************************************************/
	
	/****************手自动充电和急停测试*************/
	chargeAndEmergencyRun();
	/**************************************************/
	
	/******************抱闸和DI测试*********************/
	brakeOrDIRun();
	/**************************************************/
	
	/******************读DO测试*************************/
	detectPDORun();
	/***************************************************/
	
	/*****************检测开机灯*************************/
	detectPCLight();
	/****************************************************/
	
	/*****************检测急停*************************/
	detectEmergency();
	/**************************************************/
	
	/******************抱闸检测************************/
	detectBrake();
	/***************************************************/
	
	/******************Can发送接收**********************/
	canTest2000();
	/***************************************************/
	
	/********************485测试*************************/
	uart485Run();
	/*****************************************************/
	
	/********************232测试***********************/
	uart232Run();
	/***************************************************/
	
	/*****************232或485模式切换******************/
	uartChangeRun();
	/****************************************************/

	/***********************继电器测试*********************/
	detectDelay();
	/****************************************************/
	
	/********************报警灯测试*********************/
	detectWarningLight();
	/***************************************************/ 
	return 0;
}

void CControllerSurveyorTask::udpRecieveRun()
{
	int32_t len;
	if(socket_n < 0)
	{
		sockaddr_in localaddr;
		localaddr.sin_family = AF_INET;
		localaddr.sin_port = htons(4822);
		localaddr.sin_addr.s_addr = INADDR_ANY;
		socket_n = net::socket(AF_INET, SOCK_DGRAM, 0);
		net::bind(socket_n, (sockaddr *)&localaddr,  sizeof(localaddr));
		Console::Instance()->printf("Init ControllerSurveyorTask socket: [%d]\r\n", socket_n);
	}
	len = net::recvfrom(socket_n, udp_rx_buff, BUFF_SIZE, MSG_DONTWAIT, (sockaddr *)&remoteaddr, &remoteaddrlen);
	if( len > 0)
	{
		//Console::Instance()->printf("recieve data: %s\r\n", udp_rx_buff);
		Console::Instance()->printf("recieve data: \r\n");
		for(int i=0;i<len;i++){
			Console::Instance()->printf("0x%02x,", udp_rx_buff[i]);
		}
		Console::Instance()->printf("recieve data: %s\r\n", udp_rx_buff);
		len = len > BUFF_SIZE ? BUFF_SIZE : len;
		memcpy(destip, &remoteaddr.sin_addr.s_addr, 4);
		destport = ntohs(remoteaddr.sin_port);
		if(udp_rx_buff[0] == 0x12 && udp_rx_buff[1] == 0x34){
			resetAllSwitch();
			resetAllResult();
			can_test_case_select = udp_rx_buff[2];
			Console::Instance()->printf("TEST CASE RECEIVE IS 0x%02x \r\n", can_test_case_select);
			can_dispatch_switch = true;
			can_dispatch_times_counter = 0;
		}
	}
}

void CControllerSurveyorTask::udpTx(uint8_t* tx_buff, uint16_t tx_len)
{
	int ret;
	ret = net::sendto(socket_n, tx_buff, tx_len, MSG_DONTWAIT, (sockaddr *)&remoteaddr, remoteaddrlen);
}

#define HEART_LED_GPIO		GPIOD
#define HEART_LED_PIN		GPIO_Pin_3
static void ledGpioInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = HEART_LED_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(HEART_LED_GPIO, &GPIO_InitStructure);
}

static void ledRun()
{
	static bool LedOn = true;
	static Timer freq(1000,1000);
	if(freq.isAbsoluteTimeUp())
	{
		LedOn = !LedOn;
	}
	if(LedOn)
	{
		GPIO_SetBits(HEART_LED_GPIO, HEART_LED_PIN);
	}
	else
	{
		GPIO_ResetBits(HEART_LED_GPIO, HEART_LED_PIN);
	}
}

void CControllerSurveyorTask::detectWarningLight()
{
	static Timer test_warning_light(100,100);
	if(detect_warninglight)
	{
		di_test_2000_do.ioctl(0x02);		//切换成DI模式
		if(di_test_2000_do.read(GPIOG, GPIO_Pin_4)){
			is_warning_light_on = true;
			
		}else{
			is_warning_light_on = false;
		}
		if(test_warning_light.isAbsoluteTimeUp())
		{
			if(!is_warning_light_on){
				uint8_t result[4] = {0x56,0x78,0x13,0xFF};
				udpTx(result,4);
			}else{
				uint8_t result[4] = {0x56,0x78,0x14,0xFF};
				udpTx(result,4);
			}
			Console::Instance()->printf("WARNING LIGHT  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_4));
		}
	}
}

void CControllerSurveyorTask::detectBrake()
{
	static Timer test_brake(100,100);
	if(detect_brake)
	{
		di_test_2000_do.ioctl(0x02);		//切换成DI模式
		if(di_test_2000_do.read(GPIOC, GPIO_Pin_7)){
			is_brake12_ok = true;
		}else{
			is_brake12_ok = false;
		}
		if(di_test_2000_do.read(GPIOC, GPIO_Pin_9)){
			is_brake24_ok = true;
		}	
		if(test_brake.isAbsoluteTimeUp())
		{
			if(is_brake12_ok && is_brake24_ok && (can_test_case_select == 0x0B)){
				uint8_t result[4] = {0x56,0x78,0x0B,0xFF};
				udpTx(result,4);
			}else if(!is_brake12_ok && (can_test_case_select == 0x0C)){
				uint8_t result[4] = {0x56,0x78,0x0C,0xFF};
				udpTx(result,4);
			}
			Console::Instance()->printf("BRAKE 12 IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_7));
			Console::Instance()->printf("BRAKE 24 IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_9));
			Console::Instance()->printf("is_brake12_ok %d\r\n",is_brake12_ok);
			Console::Instance()->printf("is_brake24_ok %d\r\n",is_brake24_ok);
		}	
	}
}

void CControllerSurveyorTask::detectDelay()
{
	static Timer test_Delay(100,100);
	if(detect_delay)
	{
		di_test_2000_do.ioctl(0x02);		//切换成DI模式
		if(di_test_2000_do.read(GPIOA, GPIO_Pin_8) && di_test_2000_do.read(GPIOA, GPIO_Pin_9)){
			is_delay_ok = true;
		}else{
			is_delay_ok = false;
		}
		if(test_Delay.isAbsoluteTimeUp()){
			if(!is_delay_ok){
				uint8_t result[4] = {0x56,0x78,0x0F,0xFF};
				udpTx(result,4);
			}else{
				uint8_t result[4] = {0x56,0x78,0x10,0xFF};
				udpTx(result,4);
			}
			Console::Instance()->printf("is_delay_ok %d\r\n",is_delay_ok);
		}	
	}
}

void CControllerSurveyorTask::detectEmergency()
{
	static Timer test_emergency(100,100);
	if(detect_emergency)
	{
		di_test_2000_do.ioctl(0x02);		//切换成DI模式
		if(di_test_2000_do.read(GPIOD, GPIO_Pin_13)){
			is_emc_out2_ok = true;
		}else{
			is_emc_out2_ok = false;
		}
		if(di_test_2000_do.read(GPIOD, GPIO_Pin_14)){
			is_emc_out1_ok = true;
		}else{
			is_emc_out1_ok = false;
		}
		if(di_test_2000_do.read(GPIOD, GPIO_Pin_15)){
			is_emc_light_on = true;
		}else{
			is_emc_light_on = false;
		}
		if(test_emergency.isAbsoluteTimeUp())
		{
			if((is_emc_out2_ok && is_emc_out1_ok && is_emc_light_on) && (can_test_case_select == 0x07)){
				uint8_t result[4] = {0x56,0x78,0x07,0xFF};
				udpTx(result,4);
			}else if((!is_emc_out2_ok && !is_emc_out1_ok && !is_emc_light_on) && (can_test_case_select == 0x08)){
				uint8_t result[4] = {0x56,0x78,0x08,0xFF};
				udpTx(result,4);
			}
			Console::Instance()->printf("\r\n");
			Console::Instance()->printf("EMERGENCE_OUT_2  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_13));
			Console::Instance()->printf("EMERGENCE_OUT_1  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_14));
			Console::Instance()->printf("EMERGENCE_LIGHT  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_15));
		}
	}
}

void CControllerSurveyorTask::detectPDORun()
{
	static Timer testdo(100,100);
	if(detect_pdo_switch)
	{
		if(can_test_case_select == 0x04){
			di_test_2000_do.ioctl(0x02);		//切换成DI模式
			if(di_test_2000_do.read(GPIOG, GPIO_Pin_3) && di_test_2000_do.read(GPIOC, GPIO_Pin_8) &&
			di_test_2000_do.read(GPIOG, GPIO_Pin_7) && di_test_2000_do.read(GPIOG, GPIO_Pin_8) &&
			di_test_2000_do.read(GPIOC, GPIO_Pin_6) && di_test_2000_do.read(GPIOG, GPIO_Pin_5) &&
			di_test_2000_do.read(GPIOG, GPIO_Pin_6)){
				is_all_pdo_on = true;
			}else{
				is_all_pdo_on = false;
			}
		}
		if(can_test_case_select == 0x05){
			if(!di_test_2000_do.read(GPIOG, GPIO_Pin_3) && !di_test_2000_do.read(GPIOC, GPIO_Pin_8) &&
			!di_test_2000_do.read(GPIOG, GPIO_Pin_7) && !di_test_2000_do.read(GPIOG, GPIO_Pin_8) &&
			!di_test_2000_do.read(GPIOC, GPIO_Pin_6) && !di_test_2000_do.read(GPIOG, GPIO_Pin_5) &&
			!di_test_2000_do.read(GPIOG, GPIO_Pin_6)){
				is_all_pdo_off = true;
			}else{
				is_all_pdo_off = false;
			}
		}
		
		if(testdo.isAbsoluteTimeUp())
		{
			if(is_all_pdo_on && can_test_case_select == 0x04){
				uint8_t result[4] = {0x56,0x78,0x04,0xFF};
				udpTx(result,4);
			}else if(is_all_pdo_off && can_test_case_select == 0x05){
				uint8_t result[4] = {0x56,0x78,0x05,0xFF};
				udpTx(result,4);
			}
			Console::Instance()->printf("\r\n");
			Console::Instance()->printf("pdo test on status is %d\r\n", is_all_pdo_on);
			Console::Instance()->printf("pdo test off status is %d\r\n", is_all_pdo_off);
			Console::Instance()->printf("DO0  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_3));
			Console::Instance()->printf("DO1  IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_8));
			Console::Instance()->printf("DO2  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_7));
			Console::Instance()->printf("DO3  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_8));
			Console::Instance()->printf("DO4  IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_6));
			Console::Instance()->printf("DO5  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_5));
			Console::Instance()->printf("DO6  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_6));
		}	
	}
}

void CControllerSurveyorTask::detectPCLight()
{
	static Timer test_pc_light(100,100);
	if(detect_pc_light)
	{
		di_test_2000_do.ioctl(0x02);		//切换成DI模式
		if(di_test_2000_do.read(GPIOG, GPIO_Pin_2)){
			is_boot_light_on = true;
		}else{
			is_boot_light_on = false;
		}
		if(test_pc_light.isAbsoluteTimeUp())
		{
			if(di_test_2000_do.read(GPIOG, GPIO_Pin_2)){
				is_boot_light_on = true;
				uint8_t result[4] = {0x56,0x78,0x06,0xFF};
				udpTx(result,4);
			}else{
				is_boot_light_on = false;
				uint8_t result[4] = {0x56,0x78,0x06,0x00};
				udpTx(result,4);
			}
			Console::Instance()->printf("bootlight IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_2));
		}
	}	
}

void CControllerSurveyorTask::uartChangeRun()
{
	if(is_232_or_485_mode)
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(0, GPIOD, GPIO_Pin_8);			//232模式
		//Console::Instance()->printf("232 test started \r\n");
	}
	else
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(1, GPIOD, GPIO_Pin_8);			//485模式
		//Console::Instance()->printf("485 test started \r\n");
	}
}

void CControllerSurveyorTask::brakeOrDIRun()
{
	if(is_brake_or_di_switch)
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(0, GPIOD, GPIO_Pin_11);			//松开DI，抱闸不输出
		//Console::Instance()->printf("brake test started \r\n");
	}
	else
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(1, GPIOD, GPIO_Pin_11);			//DI短接，抱闸输出24到12
		//Console::Instance()->printf("DI test started \r\n");
	}
}

void CControllerSurveyorTask::chargeAndEmergencyRun()
{
	if(charge_and_emergency_switch)
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(0, GPIOB, GPIO_Pin_14);			//DO 0	 //手自动充电断开地
	}
	else
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(1, GPIOB, GPIO_Pin_14);			//DO 0		//急停断开，输出和灯都有，自动充电短到地
	}
}

void CControllerSurveyorTask::openPCRun()
{
	static Timer open_pc_time(500);
	do_test_2000_di.ioctl(0x01);
	if(open_pc_switch)
	{
		open_pc_switch = false;
		open_pc_time.start();
		do_test_2000_di.write(1, GPIOD, GPIO_Pin_10);
		Console::Instance()->printf("in pc open process \r\n");
	}
	if(open_pc_time.isAbsoluteTimeUp())
	{
		do_test_2000_di.write(0, GPIOD, GPIO_Pin_10);
		open_pc_time.stop();
		Console::Instance()->printf("open pc finished....\r\n");
	}
}

void CControllerSurveyorTask::closePCRun()
{
	static Timer close_pc_time(3000);
	do_test_2000_di.ioctl(0x01);
	if(close_pc_switch)
	{
		close_pc_switch = false;
		close_pc_time.start();
		do_test_2000_di.write(1, GPIOD, GPIO_Pin_10);
		Console::Instance()->printf("in pc close process \r\n");
	}
	if(close_pc_time.isAbsoluteTimeUp())
	{
		do_test_2000_di.write(0, GPIOD, GPIO_Pin_10);
		close_pc_time.stop();
		Console::Instance()->printf("close pc finished....\r\n");
	}
}

void CControllerSurveyorTask::canTest2000()
{
	static Timer can_tx_freq(50);
	if(can_test_switch)
	{
		can_test_switch = false;
		can_tx_freq.start();
	}
	const uint8_t it_is_can1[8] = {0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC};
	const uint8_t it_is_can2[8] = {0x5A,0x59,0x58,0x57,0x56,0x55,0x54,0x53};
	const uint8_t i_am_can_test[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
	if (can_tx_freq.isAbsoluteTimeUp())
	{
		CanTxMsg msg;
		msg.IDE = CAN_Id_Standard;
		msg.RTR = CAN_RTR_Data;
		msg.DLC = 8;
		msg.StdId = 2;
		memcpy(msg.Data, i_am_can_test, 8);
		CanRouter1.putMsg(msg);  
	}
	if (can_mailbox_test_2000.msgsInQue() > 0)
	{
		CanRxMsg rx_msg;        
		can_mailbox_test_2000.getMsg(&rx_msg);
		if(memcmp(rx_msg.Data, it_is_can1, 8) == 0){
			is_can1_test_ok = true;
			Console::Instance()->printf("CAN1 OK\r\n");
		}
		if(memcmp(rx_msg.Data, it_is_can2, 8) == 0){
			is_can2_test_ok = true;
			Console::Instance()->printf("CAN2 OK\r\n");
		}
		if(is_can1_test_ok && is_can2_test_ok)
		{
			can_tx_freq.stop();
			uint8_t result[4] = {0x56,0x78,0x03,0xFF};
			udpTx(result,4);
		}
	}	
}

void CControllerSurveyorTask::uart485Run()
{
	static Timer transfer_frq(500);
	if(rs485_test_switch)
	{
		rs485_test_switch = false;
		is_232_or_485_mode = false;
		transfer_frq.start();
	}
	if( 0x02 == can_test_case_select){
		if (transfer_frq.isAbsoluteTimeUp())
		{
			while(0 != usart3_test_2000.get_BytesInTxFifo()){}
			usart3_test_2000.send_Array(rs485_transfer_data, 5);
		}
		if (usart3_test_2000.get_BytesInRxFifo() >= 2)
		{
			uint16_t check_sum = 0;	
			uint8_t temp[SURVEYOR_RX_DMA_BUF_LEN_458] = {0,0};
			uint8_t bytesInRxFifo = usart3_test_2000.read_RxFifo(temp);
			for(int i=0;i < bytesInRxFifo;i++){
				check_sum += temp[i];
			}
			//Console::Instance()->printf("tester:%d 0x%02x,0x%02x \r\n",bytesInRxFifo, temp[0],temp[1]);
			//if (memcmp(temp, rs485_recieve_0_data, 5) == 0)
			if(check_sum == 0xCD)
			{
				is_485_test_ok[0] = true;
			}
			//if (memcmp(temp, rs485_recieve_1_data, 5) == 0)
			if(check_sum == 0xCE)
			{
				is_485_test_ok[1] = true;
			}
			//if (memcmp(temp, rs485_recieve_2_data, 5) == 0)
			if(check_sum == 0xCF)
			{
				is_485_test_ok[2] = true;
			}
			//if (memcmp(temp, rs485_recieve_3_data, 5) == 0)
			if(check_sum == 0xD0)
			{
				is_485_test_ok[3] = true;
				Console::Instance()->printf("485 TEST IS OK \r\n");
				can_test_case_select = 0xFF;
				transfer_frq.stop();
				uint8_t result[4] = {0x56,0x78,0x02,0xFF};
				udpTx(result,4);				
			}
		}
	}
}

void CControllerSurveyorTask::uart232Run()
{	
	static Timer transfer_frq(500);
	if(rs232_test_switch)
	{
		rs232_test_switch = false;
		is_232_or_485_mode = true;
		transfer_frq.start();
	}
	if( 0x01 == can_test_case_select){
		if (transfer_frq.isAbsoluteTimeUp())
		{
			while(0 != usart3_test_2000.get_BytesInTxFifo()){}
			usart3_test_2000.send_Array(rs232_transfer_data, 5);		
		}
		if (usart2_test_2000.get_BytesInRxFifo() >= 5)
		{
			uint16_t check_sum = 0;	
			uint8_t temp[SURVEYOR_RX_DMA_BUF_LEN_232];
			uint8_t bytesInRxFifo = usart2_test_2000.read_RxFifo(temp);
			for(int i=0;i < SURVEYOR_RX_DMA_BUF_LEN_232;i++){
				check_sum += temp[i];
			}
			//Console::Instance()->printf("tester 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \r\n",temp[0],temp[1],temp[2],temp[3],temp[4]);
			//if (memcmp(temp, rs232_recieve_3_data, 5) == 0)
			if(check_sum == 0x378)
			{
				is_232_test_ok = true;
				Console::Instance()->printf("232 TEST IS OK \r\n");
				transfer_frq.stop();
				can_test_case_select = 0xFF;
				uint8_t result[4] = {0x56,0x78,0x01,0xFF};
				udpTx(result,4);
			}
		}	
	}
}

#include "string.h"
void CControllerSurveyorTask::CanDispatchRun()
{
	static Timer can_transfer_freq(100,100);
	if (can_transfer_freq.isAbsoluteTimeUp() && can_dispatch_switch)
    {
		//resetAllSwitch();
		CanTxMsg msg;
		msg.IDE = CAN_Id_Standard;
		msg.RTR = CAN_RTR_Data;
		msg.DLC = 8;
		msg.StdId = 0xD2;

		switch(can_test_case_select)
		{
			case 0x01:	//测试232
				rs232_test_switch = true;
				can_dispatch_switch = false;
				break;
			case 0x02:	//测试485
				rs485_test_switch = true;
				can_dispatch_switch = false;
				break;
			case 0x03:	//测试Can
				can_test_switch = true;
				can_dispatch_switch = false;
				break;
			case 0x04:	//测试DO打开
				detect_pdo_switch = true;
				break;
			case 0x05:	//测试DO关闭
				detect_pdo_switch = true;
				break;
			case 0x06:	//测试开机灯
				detect_pc_light = true;
				break;
			case 0x07:	//测试急停输出
				charge_and_emergency_switch = false;
				detect_emergency = true;
				break;
			case 0x08:	//断开急停输出
				charge_and_emergency_switch = true;
				detect_emergency = true;
				break;
			case 0x09:	//测试手自动充电
				charge_and_emergency_switch = false;
				break;
			case 0x0A:	//断开手自动充电接地
				charge_and_emergency_switch = true;
				break;
			case 0x0B:	//测试抱闸
				is_brake_or_di_switch = false;
				detect_brake = true;
				break;
			case 0x0C:	//断开抱闸输出
				is_brake_or_di_switch = true;
				detect_brake = true;
				break;
			case 0x0D:	//测试DI不接地
				is_brake_or_di_switch = true;
				break;
			case 0x0E:	//DI接地
				is_brake_or_di_switch = false;
				break;
			case 0x0F:	//断开继电器
				detect_delay = true;
				break;
			case 0x10:	//打开继电器
				detect_delay = true;
				break;
			case 0x11:	//开机
				open_pc_switch = true;
				can_dispatch_switch = false;
				break;
			case 0x12:	//关机
				close_pc_switch = true;
				can_dispatch_switch = false;
				break;
			case 0x13:	//打开报警灯
				detect_warninglight = true;
				break;
			case 0x14:	//关闭报警灯
				detect_warninglight = true;
				break;
			case 0x15:	//取UID
				
				break;
			case 0x16:	//取主版本
	
				break;
			case 0x17:	//取陀螺仪
	
				break;
			case 0xFF:
				break;
			default:
				break;
		}	
		memcpy(msg.Data, &can_test_case_select, 1);
        CanRouter1.putMsg(msg);      
    }
    if (can_dispatch.msgsInQue() > 0)
    {
        CanRxMsg rx_msg;        
        can_dispatch.getMsg(&rx_msg);
		switch(rx_msg.Data[1])
		{
			case 0x09:
				if(rx_msg.Data[2] == 0xFF){
					is_manual_charge_ok = true;
					Console::Instance()->printf("manual is ok\r\n");
				}
				if(rx_msg.Data[3] == 0xFF){
					is_auto_charge_ok = true;
					Console::Instance()->printf("auto is ok\r\n");
				}
				if(is_manual_charge_ok && is_auto_charge_ok){
					uint8_t result[4] = {0x56,0x78,0x09,0xFF};
					udpTx(result,4);
				}
				break;
			case 0x0A:
				if(rx_msg.Data[2] == 0xFF){
					is_manual_charge_ok = false;
					Console::Instance()->printf("manual is not tie ground\r\n");
				}
				if(rx_msg.Data[3] == 0xFF){
					is_auto_charge_ok = false;
					Console::Instance()->printf("auto is not tie ground\r\n");
				}
				if(!is_manual_charge_ok && !is_auto_charge_ok){
					uint8_t result[4] = {0x56,0x78,0x0A,0xFF};
					udpTx(result,4);
				}
				break;
			case 0x0D:
				if(rx_msg.Data[2] == 0xFF){
					is_2000_di_off = true;
					Console::Instance()->printf("di is all off\r\n");
				}
				if(is_2000_di_off){
					uint8_t result[4] = {0x56,0x78,0x0D,0xFF};
					udpTx(result,4);
				}
				break;
			case 0x0E:
				if(rx_msg.Data[2] == 0xFF){
					is_2000_di_on = true;
					Console::Instance()->printf("di is all on\r\n");
				}
				if(is_2000_di_on){
					uint8_t result[4] = {0x56,0x78,0x0E,0xFF};
					udpTx(result,4);
				}
				break;
			case 0x15:
				{
					can_dispatch_switch = false;
					uint8_t result[9] = {0x56,0x78,0x15,rx_msg.Data[2],rx_msg.Data[3],rx_msg.Data[4],rx_msg.Data[5],rx_msg.Data[6],rx_msg.Data[7]};
					udpTx(result, 9);
				}
				break;
			case 0x16:
				{
					can_dispatch_switch = false;
					uint8_t result[7] = {0x56,0x78,0x16,rx_msg.Data[2],rx_msg.Data[3],rx_msg.Data[4],rx_msg.Data[5]};
					udpTx(result, 7);
				}
				break;
			case 0x17:
				{
					can_dispatch_switch = false;
					uint8_t result[6] = {0x56,0x78,0x16,rx_msg.Data[2],rx_msg.Data[3],rx_msg.Data[4]};
					udpTx(result, 6);	
				}
				break;
			default:
				break;
		}
		
		if(can_dispatch_times_counter++ > 10)
			can_dispatch_switch = false;
    }
}
	
