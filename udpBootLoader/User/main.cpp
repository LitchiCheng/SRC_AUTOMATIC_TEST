/********************************************************************************
* @file    udpBootloader
* @author  Chenxx
* @version V1.0
* @date    2016-04-21
* @brief   
* release information: 
*	V1.0: Add bootloader update function.
* V1.1: Add flash read function. 
* V1.2: Disable interrupt before jump to bootloader.
********************************************************************************/
#include "stm32f4xx.h"
#include "Console.h"
#include "CommonConfig.h"

//#include "w5500.h"
#include "socket.h"
#include "udpDevice.h"
#include "iap.h"
#include "Timer.h"
#include "stmflash.h"
#include "CCan.h"
#include "NetworkAdapter.h"
#include "network_conf.h"
#include "powerupOption.h"
#include "HeartLed.h"
#include "DIO_TEST.h"
#include "CUsart.h"

uint8_t tx_mem_conf[8] = {8,8,8,8,8,8,8,8};	// for setting TMSR regsiter
uint8_t rx_mem_conf[8] = {8,8,8,8,8,8,8,8};   // for setting RMSR regsiter
uint8_t w5500_ip[4] = IPV4_ADDR;            // for setting SIP register
uint8_t gw[4] = IPV4_GATEWAY;              // for setting GAR register
uint8_t sn[4] = IPV4_SUBNETMASK;              // for setting SUBR register
uint8_t mac[6] = MAC_ADDR;   // for setting SHAR register
uint8_t test_buf[10] = {0,0,0,0,0,0,0,0,0,0};

namespace
{
	const uint8_t TX_QUEBUF_SIZE = 64;
	CanTxMsg router1_txQueBuf[TX_QUEBUF_SIZE];
	CanTxMsg router2_txQueBuf[TX_QUEBUF_SIZE];
};
CCanRouter CanRouter1(CAN1, router1_txQueBuf, TX_QUEBUF_SIZE, 250000);
CCanRouter CanRouter2(CAN2, router2_txQueBuf, TX_QUEBUF_SIZE, 250000);


extern const uint8_t FIRMWARE_VERSION = 0x16;
extern CUdpDevice udpConsole;
// <<< Use Configuration Wizard in Context Menu >>>
// <e> RUN_IN_APP
// 	<i>Default: 0
#define RUN_IN_APP 0
// <<< end of configuration section >>>
int main(void)
{ 
#if RUN_IN_APP
	SCB->VTOR = FLASH_APP1_ADDR;
#else
	SCB->VTOR = 0x08000000;
#endif
	NVIC_CONFIG();
	BaseTimer::Instance()->initialize();
	Initial_HeartLED();
	/**************************************************************/
	DIO_TEST dio_test;
	DIO_TEST do_4;
	do_4.open();
	do_4.ioctl(0x01);
	dio_test.open();
	dio_test.ioctl(0x02);
	dio_test.write(0, GPIOB, GPIO_Pin_13);			//0245
//	dio_test.write(1, GPIOD, GPIO_Pin_11);
	
	do_4.write(1, GPIOD, GPIO_Pin_10);
	BaseTimer::Instance()->delay_ms(100);
	do_4.write(0, GPIOD, GPIO_Pin_10);
	
	
	Timer dio_time(500, 500);	
	bool DIO_on = true;
	/******************************************************************/
	#define BUFF_SIZE 10
	uint8_t rxbuff[BUFF_SIZE] = {};
	uint8_t txbuff_uart3[5] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
	uint8_t txbuff_uart6[5] = {0x05, 0x04, 0x03, 0x02, 0x01};
	CUsart uart3_test(USART3, rxbuff, BUFF_SIZE);
	uart3_test.InitSciGpio();
	uart3_test.InitSci();
	Timer test_for_usart(100,100);
//	CUsart uart6_test(USART6, rxbuff, BUFF_SIZE);
//	uart6_test.setBaudrate(9600);
//	uart6_test.InitSciGpio();
//	uart6_test.InitSci();
	/***********************************************/
//	const uint8_t TX_QUEBUF_SIZE = 35;
//	CanTxMsg router1_txQueBuf[TX_QUEBUF_SIZE];

//	CCanRouter CanRouter1(CAN1, router1_txQueBuf, TX_QUEBUF_SIZE, 250000);

	CanRouter1.InitCan();
	CanRouter1.InitCanGpio(F4KXX_CAN1_IOGROUP);	
	
//	CCanRxMailbox::MsgData can1_rx_buff[100];
//	CCanRxMailbox can1_mailbox(can1_rx_buff, 100);
//	can1_mailbox.setStdId(1);
//	can1_mailbox.attachToRouter(CanRouter1);
//		CanRouter1.InitCan();
//		CanRouter1.InitCanGpio(F4KXX_CAN1_IOGROUP);		
	
	const uint8_t dat_1to2[8] = {0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC};
  const uint8_t dat_2to1[8] = {0x5A,0x59,0x58,0x57,0x56,0x55,0x54,0x53};
	Timer _canTxTimer(50, 50);
	
	/***************************************************/
	GPIO_SetBits(HEART_LED_GPIO, HEART_LED_PIN);
	
	if(BOOT_PARAM_BL != pvf::read(pvf::VAR_BOOT_OPTI) && BOOT_PARAM_APP != pvf::read(pvf::VAR_BOOT_OPTI) && BOOT_PARAM_LOADING != pvf::read(pvf::VAR_BOOT_OPTI))
		pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_BL);
	
	BaseTimer::Instance()->delay_ms(2000);
	
	NetworkAdapter::Instance()->doInit();
	iapUdpDevice.open();
	Timer rcvTimer = Timer(1000,1000);
	while(!rcvTimer.isAbsoluteTimeUp())
	{
		iapUdpDevice.run();
		if(iapUdpDevice.data_in_read_buf() > 1)
			Console::Instance()->printf("In bootloader Receive data %d\r\n",iapUdpDevice.data_in_read_buf());
		Console::Instance()->runTransmitter();
	}
#if !RUN_IN_APP
	if(iapUdpDevice.data_in_read_buf() > 1)
	{
		iapUdpDevice.runReceiver();
		iapUdpDevice.read(test_buf, 2);
		Console::Instance()->printf("Receive data 0x%x 0x%x\r\n",test_buf[0],test_buf[1]);
		if(test_buf[0] == 0x7F && test_buf[1] == 0x7F)
		{
			Console::Instance()->printf("Get into bootloader by emergency mode.\r\n");
//			write_boot_parameter(BOOT_PARAM_IAP);
			pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_BL);
		}else if(BOOT_PARAM_APP == pvf::read(pvf::VAR_BOOT_OPTI))
		{
			Console::Instance()->printf("in BootLoader 2\r\n");
			iap_load_app(FLASH_APP1_ADDR);
		}
	}else if(BOOT_PARAM_APP == pvf::read(pvf::VAR_BOOT_OPTI))
	{
		iap_load_app(FLASH_APP1_ADDR);
		//pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_BL);
	}
	
#endif

#if RUN_IN_APP
	BaseTimer::Instance()->delay_ms(1000);
	Console::Instance()->printf("\r\n Bootloader flasher start!\r\n");
#else	
	if(BOOT_PARAM_BL == pvf::read(pvf::VAR_BOOT_OPTI) || BOOT_PARAM_LOADING == pvf::read(pvf::VAR_BOOT_OPTI))
	{
		Console::Instance()->printf("\r\n Bootloader start!\r\n");
	}else
	{
		Console::Instance()->printf("\r\n Boot paramter error: 0x%X, keep at bootloader\r\n",
			pvf::read(pvf::VAR_BOOT_OPTI));
	}
#endif
	Console::Instance()->printf("Firmware virsion: V%d.%d\r\n", FIRMWARE_VERSION>>4, FIRMWARE_VERSION&0xF);
	while(1)
	{	
	
		if(rcvTimer.isAbsoluteTimeUp())
		{
//			//do_4.write(0, GPIOD, GPIO_Pin_9);
//			Console::Instance()->printf("EMERGENCE_OUT_2  IS %d\r\n",dio_test.read(GPIOD, GPIO_Pin_13));
//			Console::Instance()->printf("EMERGENCE_OUT_1  IS %d\r\n",dio_test.read(GPIOD, GPIO_Pin_14));
//			Console::Instance()->printf("EMERGENCE_LIGHT  IS %d\r\n",dio_test.read(GPIOD, GPIO_Pin_15));
			
			do_4.write(1, GPIOD, GPIO_Pin_11);
			Console::Instance()->printf("BRAKE  IS %d\r\n",dio_test.read(GPIOC, GPIO_Pin_9));	//
			
//			Console::Instance()->printf("bootlight  IS %d\r\n",dio_test.read(GPIOG, GPIO_Pin_2));
//			
//			Console::Instance()->printf("WARNING LIGHT  IS %d\r\n",dio_test.read(GPIOG, GPIO_Pin_4));
			
			
//			Console::Instance()->printf("DO0  IS %d\r\n",dio_test.read(GPIOG, GPIO_Pin_3));
//			Console::Instance()->printf("DO1  IS %d\r\n",dio_test.read(GPIOC, GPIO_Pin_8));
//			Console::Instance()->printf("DO2  IS %d\r\n",dio_test.read(GPIOG, GPIO_Pin_7));
//			Console::Instance()->printf("DO3  IS %d\r\n",dio_test.read(GPIOG, GPIO_Pin_8));
//			Console::Instance()->printf("DO4  IS %d\r\n",dio_test.read(GPIOC, GPIO_Pin_7));
//			Console::Instance()->printf("DO5  IS %d\r\n",dio_test.read(GPIOG, GPIO_Pin_5));
//			Console::Instance()->printf("DO6  IS %d\r\n",dio_test.read(GPIOG, GPIO_Pin_6));
			
			
		}
		
		
//		if(dio_time.isAbsoluteTimeUp())
//		{
//			DIO_on = !DIO_on;
//			if(DIO_on)
//			{
//				dio_test.write(0, GPIOD, GPIO_Pin_11);
//				dio_test.write(0, GPIOB, GPIO_Pin_14);
//				dio_test.write(0, GPIOD, GPIO_Pin_8);
//				dio_test.write(0, GPIOD, GPIO_Pin_10);
//			}	
//			else
//			{
//				dio_test.write(1, GPIOD, GPIO_Pin_11);
//				dio_test.write(1, GPIOB, GPIO_Pin_14);
//				dio_test.write(1, GPIOD, GPIO_Pin_8);
//				dio_test.write(1, GPIOD, GPIO_Pin_10);
//			}
//		}
		
//		if (_canTxTimer.isAbsoluteTimeUp())
//		{
//			CanTxMsg msg;
//			msg.IDE = CAN_Id_Standard;
//			msg.RTR = CAN_RTR_Data;
//			msg.DLC = 8;

//			msg.StdId = 2;
//			memcpy(msg.Data, dat_1to2, 8);
//			CanRouter1.putMsg(msg);
//      
//		}
//		
//		if(test_for_usart.isAbsoluteTimeUp()){
//			if(uart3_test.get_BytesInTxFifo() == 0)
//			{
//				uart3_test.send_Array(txbuff_uart3,5);
//			}
//			Console::Instance()->printf("xcwdfwef\r\n");
//		}
//		CanRouter1.runTransmitter();
//		CanRouter1.runReceiver();
		
		iapUdpDevice.run();
		iap_run();
		Console::Instance()->runTransmitter();
		HeartLed_Run();
	}
}

#if  1

void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	Timer loopTimer(1000,1000);
	
  /* Infinite loop */
  while (1)
  {
		Console::Instance()->runTransmitter();
		if(loopTimer.isAbsoluteTimeUp())
		{
			Console::Instance()->printf("Wrong parameters value: file %s on line %d\r\n", file, line);
		}
  }
}
#endif



