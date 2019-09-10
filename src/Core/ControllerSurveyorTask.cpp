#include "ControllerSurveyorTask.h"
#include "GlobalData.h"
#include "Console.h"
#include "stdlib.h"
#include "string.h"
#include "DIO_TEST.h"

static void ledGpioInit();
static void ledRun();
static void diTest2000Do();
static void canTest2000();
static void uart485Run();
static void canTest2000();
static void openPCRun();
static void closePCRun();
static void chargeAndEmergencyRun();
static void brakeOrDIRun();
static void detectPDORun();
static void detectPCLight();
static void detectEmergency();
static void detectBrake();
static void uartChangeRun();
static void uart232Run();
static void CanDispatchRun();
static void detectDelay();

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
	const uint8_t SURVEYOR_RX_DMA_BUF_LEN = 5;
	uint8_t buffer_485[SURVEYOR_RX_DMA_BUF_LEN];	
	CUsart usart3_test_2000(USART3, buffer_485, SURVEYOR_RX_DMA_BUF_LEN);
	/***************************************************************/
	uint8_t buffer_232[SURVEYOR_RX_DMA_BUF_LEN];	
	CUsart usart2_test_2000(USART2, buffer_232, SURVEYOR_RX_DMA_BUF_LEN);
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
	
	uint8_t rs485_transfer_data[5] = {0x00, 0xAA, 0xAA, 0xAA, 0xAA};
	
	uint8_t rs485_recieve_0_data[5] = {0x01, 0xCC, 0xCC, 0xCC, 0xCC};
	uint8_t rs485_recieve_1_data[5] = {0x02, 0xCC, 0xCC, 0xCC, 0xCC};
	uint8_t rs485_recieve_2_data[5] = {0x03, 0xCC, 0xCC, 0xCC, 0xCC};
	uint8_t rs485_recieve_3_data[5] = {0x04, 0xCC, 0xCC, 0xCC, 0xCC};
	uint8_t rs232_recieve_3_data[5] = {0x04, 0xDD, 0xDD, 0xDD, 0xDD};
	
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
	static bool is_brake12_ok = false;
	static bool is_brake24_ok = false;
	static bool is_delay_ok = false;
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
	is_232_or_485_mode = false;
	can_test_switch = false;
	rs485_test_switch = false;
	rs232_test_switch = false;
	detect_delay = false;
}


CControllerSurveyorTask::CControllerSurveyorTask(): CTask(NAMECODE_ControllerSurveyorTask),_t(2000,2000),_comTxTimer(100,100),_canTxTimer(100,100),_canRouter_1(CanRouter1)
{
}

CControllerSurveyorTask::~CControllerSurveyorTask(){}

void CControllerSurveyorTask::initial()
{
	_t.reset();
    _comTxTimer.reset();
	_canTxTimer.reset();
	
	ledGpioInit();		//�����Ƴ�ʼ��
	
	usart3_test_2000.setBaudrate(115200);	//���ڳ�ʼ����Ĭ��485
	usart3_test_2000.InitSciGpio();
	usart3_test_2000.InitSci();
	
	usart2_test_2000.setBaudrate(115200);	//232���ڳ�ʼ��
	usart2_test_2000.InitSciGpio();
	usart2_test_2000.InitSci();
	
    can_mailbox_test_2000.setStdId(1);
    can_mailbox_test_2000.attachToRouter(_canRouter_1);
	
	can_dispatch.setStdId(0xD1);
	can_dispatch.attachToRouter(_canRouter_1);
	
	di_test_2000_do.open();
	do_test_2000_di.open();

	open_pc_switch = false;
	close_pc_switch = false;
	charge_and_emergency_switch = false;
	is_brake_or_di_switch = false;
	detect_pdo_switch = false;
	detect_pc_light = false;
	detect_emergency = false;
	detect_brake = false;
	is_232_or_485_mode = false;
	can_test_switch = false;
	rs485_test_switch = false;
	rs232_test_switch = false;
	detect_delay = false;
}

int CControllerSurveyorTask::doRun()
{
	/******************������*************************/
	ledRun();
	/**************************************************/
	/******************���ػ�����**********************/
	openPCRun();
	closePCRun();
	/*************************************************/
	/****************���Զ����ͼ�ͣ����*********************/
	chargeAndEmergencyRun();
	/**************************************************/
	/*****************�������*************************/
	udpRecieveRun();
	/**************************************************/
	/******************��բ��DI����*********************/
	brakeOrDIRun();
	/**************************************************/
	/******************��DO����*************************/
	detectPDORun();
	/***************************************************/
	/*****************��⿪����*************************/
	detectPCLight();
	/****************************************************/
	/*****************��⼱ͣ****************************/
	detectEmergency();
	/**************************************************/
	/*****************232��485ģʽ�л�******************/
	uartChangeRun();
	/****************************************************/
	/******************��բ���************************/
	detectBrake();
	/***************************************************/
	/******************Can���ͽ���**********************/
	canTest2000();
	/***************************************************/
	/********************485����*************************/
	uart485Run();
	/*****************************************************/
	/********************232����***********************/
	uart232Run();
	/***************************************************/
	/*********************�������************************/
	CanDispatchRun();
	/****************************************************/
	/***********************�̵�������*********************/
	detectDelay();
	/****************************************************/
	//if(_t.isAbsoluteTimeUp())
		 
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
		Console::Instance()->printf("recieve data: %s\r\n", udp_rx_buff);
		len = len > BUFF_SIZE ? BUFF_SIZE : len;
		memcpy(destip, &remoteaddr.sin_addr.s_addr, 4);
		destport = ntohs(remoteaddr.sin_port);	
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

static void diTest2000Do()
{
//	do_test_2000_di.write(0, GPIOD, GPIO_Pin_9);			//���¼�ͣ

//	do_test_2000_di.write(1, GPIOD, GPIO_Pin_11);			//���±�բ

//	Console::Instance()->printf("WARNING LIGHT  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_4));
}

static void detectBrake()
{
	static Timer test_brake(100,100);
	if(detect_brake)
	{
		di_test_2000_do.ioctl(0x02);		//�л���DIģʽ
		if(di_test_2000_do.read(GPIOC, GPIO_Pin_7)){
			is_brake12_ok = true;
		}else{
			is_brake12_ok = false;
		}
		if(di_test_2000_do.read(GPIOC, GPIO_Pin_9)){
			is_brake24_ok = true;
		}
//		if(test_brake.isAbsoluteTimeUp()){
////			Console::Instance()->printf("BRAKE 12 IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_7));
////			Console::Instance()->printf("BRAKE 24 IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_9));
//			Console::Instance()->printf("is_brake12_ok %d\r\n",is_brake12_ok);
//			Console::Instance()->printf("is_brake24_ok %d\r\n",is_brake24_ok);

//		}
		
	}
}

static void detectDelay()
{
	static Timer test_Delay(100,100);
	if(detect_delay)
	{
		di_test_2000_do.ioctl(0x02);		//�л���DIģʽ
		if(di_test_2000_do.read(GPIOA, GPIO_Pin_8) && di_test_2000_do.read(GPIOA, GPIO_Pin_9)){
			is_delay_ok = true;
		}else{
			is_delay_ok = false;
		}
		if(test_Delay.isAbsoluteTimeUp()){
			Console::Instance()->printf("is_delay_ok %d\r\n",is_delay_ok);
		}
		
	}
}

static void detectEmergency()
{
	if(detect_emergency)
	{
		di_test_2000_do.ioctl(0x02);		//�л���DIģʽ
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
//		Console::Instance()->printf("\r\n");
//		Console::Instance()->printf("EMERGENCE_OUT_2  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_13));
//		Console::Instance()->printf("EMERGENCE_OUT_1  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_14));
//		Console::Instance()->printf("EMERGENCE_LIGHT  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_15));
	}
}

static void detectPDORun()
{
	static Timer testdo(100,100);
	if(detect_pdo_switch)
	{
		di_test_2000_do.ioctl(0x02);		//�л���DIģʽ
		if(di_test_2000_do.read(GPIOG, GPIO_Pin_3) && di_test_2000_do.read(GPIOC, GPIO_Pin_8) &&
		di_test_2000_do.read(GPIOG, GPIO_Pin_7) && di_test_2000_do.read(GPIOG, GPIO_Pin_8) &&
		di_test_2000_do.read(GPIOC, GPIO_Pin_6) && di_test_2000_do.read(GPIOG, GPIO_Pin_5) &&
		di_test_2000_do.read(GPIOG, GPIO_Pin_6)){
			is_all_pdo_on = true;
		}
		if(!di_test_2000_do.read(GPIOG, GPIO_Pin_3) || !di_test_2000_do.read(GPIOC, GPIO_Pin_8) ||
		!di_test_2000_do.read(GPIOG, GPIO_Pin_7) || !di_test_2000_do.read(GPIOG, GPIO_Pin_8) ||
		!di_test_2000_do.read(GPIOC, GPIO_Pin_6) || !di_test_2000_do.read(GPIOG, GPIO_Pin_5) ||
		!di_test_2000_do.read(GPIOG, GPIO_Pin_6)){
			is_all_pdo_off = true;
		}
		
//		if(testdo.isAbsoluteTimeUp())
//		{
//			Console::Instance()->printf("\r\n");
//			Console::Instance()->printf("DO0  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_3));
//			Console::Instance()->printf("DO1  IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_8));
//			Console::Instance()->printf("DO2  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_7));
//			Console::Instance()->printf("DO3  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_8));
//			Console::Instance()->printf("DO4  IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_6));
//			Console::Instance()->printf("DO5  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_5));
//			Console::Instance()->printf("DO6  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_6));
//		}	
	}
}

static void  detectPCLight()
{
	if(detect_pc_light)
	{
		di_test_2000_do.ioctl(0x02);		//�л���DIģʽ
		if(di_test_2000_do.read(GPIOG, GPIO_Pin_2)){
			is_boot_light_on = true;
		}else{
			is_boot_light_on = false;
		}
		//Console::Instance()->printf("bootlight IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_2));
	}	
}

static void uartChangeRun()
{
	if(is_232_or_485_mode)
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(0, GPIOD, GPIO_Pin_8);			//232ģʽ
		//Console::Instance()->printf("232 test started \r\n");
	}
	else
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(1, GPIOD, GPIO_Pin_8);			//485ģʽ
		//Console::Instance()->printf("485 test started \r\n");
	}
}

static void brakeOrDIRun()
{
	if(is_brake_or_di_switch)
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(0, GPIOD, GPIO_Pin_11);			//�ɿ�DI����բ�����
		//Console::Instance()->printf("brake test started \r\n");
	}
	else
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(1, GPIOD, GPIO_Pin_11);			//DI�̽ӣ���բ���24��12
		//Console::Instance()->printf("DI test started \r\n");
	}
	
}

static void chargeAndEmergencyRun()
{
	if(charge_and_emergency_switch)
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(0, GPIOB, GPIO_Pin_14);			//DO 0	 //���Զ����Ͽ���
	}
	else
	{
		do_test_2000_di.ioctl(0x01);
		do_test_2000_di.write(1, GPIOB, GPIO_Pin_14);			//DO 0		//��ͣ�Ͽ�������͵ƶ��У��Զ����̵���
	}
}

static void openPCRun()
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

static void closePCRun()
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

static void canTest2000()
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
			//Console::Instance()->printf("CAN1 OK\r\n");
		}
		if(memcmp(rx_msg.Data, it_is_can2, 8) == 0){
			is_can2_test_ok = true;
			//Console::Instance()->printf("CAN2 OK\r\n");
		}
		if(is_can1_test_ok && is_can2_test_ok)
		{
			can_tx_freq.stop();
		}
	}	
}

static void uart485Run()
{
	static Timer transfer_frq(500);
	if(rs485_test_switch)
	{
		rs485_test_switch = false;
		is_232_or_485_mode = false;
		transfer_frq.start();
	}
	if (transfer_frq.isAbsoluteTimeUp())
	{
		while(0 != usart3_test_2000.get_BytesInTxFifo()){}
		usart3_test_2000.send_Array(rs485_transfer_data, 5);	
	}
	if (usart3_test_2000.get_BytesInRxFifo() >= 5)
	{
		uint8_t temp[SURVEYOR_RX_DMA_BUF_LEN];
		uint8_t bytesInRxFifo = usart3_test_2000.read_RxFifo(temp);
		if (memcmp(temp, rs485_recieve_0_data, 5) == 0)
		{
			is_485_test_ok[0] = true;
		}
		if (memcmp(temp, rs485_recieve_1_data, 5) == 0)
		{
			is_485_test_ok[1] = true;
		}
		if (memcmp(temp, rs485_recieve_2_data, 5) == 0)
		{
			is_485_test_ok[2] = true;
		}
		if (memcmp(temp, rs485_recieve_3_data, 5) == 0)
		{
			is_485_test_ok[3] = true;
			Console::Instance()->printf("485 TEST IS OK \r\n");
			transfer_frq.stop();	
		}
	}			
}

static void uart232Run()
{
	uint8_t rs232_transfer_data[5] = {0x00, 0xBB, 0xBB, 0xBB, 0xBB};
	static Timer transfer_frq(500);
	if(rs232_test_switch)
	{
		rs232_test_switch = false;
		is_232_or_485_mode = true;
		transfer_frq.start();
	}
	if (transfer_frq.isAbsoluteTimeUp())
	{
		while(0 != usart3_test_2000.get_BytesInTxFifo()){}
		usart3_test_2000.send_Array(rs232_transfer_data, 5);
		Console::Instance()->printf("232 TEST IS run \r\n");
		transfer_frq.stop();			
	}
	if (usart2_test_2000.get_BytesInRxFifo() >= 5)
	{
		uint8_t temp[SURVEYOR_RX_DMA_BUF_LEN];
		uint8_t bytesInRxFifo = usart2_test_2000.read_RxFifo(temp);
		if (memcmp(temp, rs232_recieve_3_data, 5) == 0)
		{
			is_232_test_ok = true;
			Console::Instance()->printf("232 TEST IS OK \r\n");
			transfer_frq.stop();
		}
	}	
}

#include "string.h"
static void CanDispatchRun()
{
	static Timer ii(100,100);
	static bool dd = true;
	static uint32_t ss = 0;
	//if(ii.isAbsoluteTimeUp())
		//Console::Instance()->printf("232 TEST IS %d \r\n", is_232_test_ok);
	if (ii.isAbsoluteTimeUp() && dd)
    {
		resetAllSwitch();
        CanTxMsg msg;
        msg.IDE = CAN_Id_Standard;
		msg.RTR = CAN_RTR_Data;
		msg.DLC = 8;
		msg.StdId = 0xD2;
		uint8_t test_case = 0x01;
		switch(test_case)
		{
			case 0x01:	//����232
				//is_232_or_485_mode = true;
				rs232_test_switch = true;
				dd = false;
				break;
			case 0x02:	//����485
				is_232_or_485_mode = false;
				rs485_test_switch = true;
				break;
			case 0x03:	//����Can
				can_test_switch = false;
				break;
			case 0x04:	//����DO��
				detect_pdo_switch = true;
				break;
			case 0x05:	//����DO�ر�
				detect_pdo_switch = true;
				break;
			case 0x06:	//���Կ�����
				detect_pc_light = true;
				break;
			case 0x07:	//���Լ�ͣ���
				charge_and_emergency_switch = false;
				detect_emergency = true;
				break;
			case 0x08:	//�������Զ����
				charge_and_emergency_switch = false;
				break;
			case 0x09:	//���Ա�բ
				is_brake_or_di_switch = false;
				detect_brake = true;
				break;
			case 0x0A:	//����DI�ӵ�
				is_brake_or_di_switch = false;
				break;
			case 0x0B:	//���Լ̵���
				detect_delay = true;
				break;
			case 0x0C:	//����
				open_pc_switch = true;
				dd = false;
				break;
			case 0x0D:	//�ػ�
				close_pc_switch = true;
				dd = false;
				break;
			case 0xFF:
				break;
			default:
				break;
		}	
		memcpy(msg.Data, &test_case, 1);
        CanRouter1.putMsg(msg);      
    }
    if (can_dispatch.msgsInQue() > 0)
    {
        CanRxMsg rx_msg;        
        can_dispatch.getMsg(&rx_msg);
		switch(rx_msg.Data[1])
		{
			case 0x08:
				if(rx_msg.Data[2] == 0xFF){
					is_manual_charge_ok = true;
					//Console::Instance()->printf("manual is ok\r\n");
				}
				if(rx_msg.Data[3] == 0xFF){
					is_auto_charge_ok = true;
					//Console::Instance()->printf("auto is ok\r\n");
				}
				break;
			case 0x0A:
				if(rx_msg.Data[2] == 0xFF){
					is_2000_di_on = true;
					//Console::Instance()->printf("di is all on\r\n");
				}
			default:
				break;
		}
	//	dd = false;
		if(ss++ > 100)
			dd = false;
    }
}
	