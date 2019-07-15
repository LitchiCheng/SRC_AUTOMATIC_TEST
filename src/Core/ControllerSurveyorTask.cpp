#include "ControllerSurveyorTask.h"
#include "GlobalData.h"
#include "Console.h"
#include "stdlib.h"
#include "string.h"
#include "DIO_TEST.h"

static void ledGpioInit();
static void ledRun();
static void diTest2000Do();
static void doTest2000Di();
static void canTest2000();
static void uartRun();
static void canTest2000();

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
	CCanRxMailbox::MsgData rxMailboxBuf[SURVEYOR_RX_MAILBOX_QUE_SIZE];
	CCanRxMailbox can_mailbox_test_2000(rxMailboxBuf, SURVEYOR_RX_MAILBOX_QUE_SIZE);
	/**********************************************************************************/
	
	/*****************************************************************/
	const uint8_t SURVEYOR_RX_DMA_BUF_LEN = 5;
	uint8_t buffer[SURVEYOR_RX_DMA_BUF_LEN];	
	CUsart usart3_test_2000(USART3, buffer, SURVEYOR_RX_DMA_BUF_LEN);
	const uint8_t tx_data = 0x5A;
	/***************************************************************/
	
	DIO_TEST di_test_2000_do;
	DIO_TEST do_test_2000_di;

}

uint8_t rs232_data[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
uint8_t rs485_data[5] = {0x05, 0x04, 0x03, 0x02, 0x01};

CControllerSurveyorTask::CControllerSurveyorTask(): CTask(NAMECODE_ControllerSurveyorTask),
                                                    _t(2000,2000),
                                                    _comTxTimer(100,100),
                                                    _canTxTimer(100,100),
                                                    _canRouter_1(CanRouter1)

{	

}

CControllerSurveyorTask::~CControllerSurveyorTask()
{
    // no need
}

void CControllerSurveyorTask::initial()
{
	ledGpioInit();		//心跳灯初始化
	
	usart3_test_2000.setBaudrate(115200);	//串口初始化，默认485
	usart3_test_2000.InitSciGpio();
	usart3_test_2000.InitSci();
	
    _t.reset();
    _comTxTimer.reset();
	_canTxTimer.reset();

    can_mailbox_test_2000.setStdId(1);
    can_mailbox_test_2000.attachToRouter(_canRouter_1);

}

int CControllerSurveyorTask::doRun()
{
	ledRun();		//心跳灯循环
	udpRecieveRun();
	canTest2000();
    
	if(_t.isAbsoluteTimeUp())	Console::Instance()->printf("XSAFDASD\r\n");
	return 0;
}

void CControllerSurveyorTask::udpRecieveRun()
{
	int32_t len;
	if(socket_n < 0)//init process
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
	if( len > 0) // check the size of received data
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
		GPIO_SetBits(HEART_LED_GPIO, HEART_LED_PIN);
	else
		GPIO_ResetBits(HEART_LED_GPIO, HEART_LED_PIN);
}

static void diTest2000Do()
{
	di_test_2000_do.ioctl(0x02);		//切换成DI模式
	
	do_test_2000_di.write(0, GPIOD, GPIO_Pin_9);			//拍下急停
	Console::Instance()->printf("EMERGENCE_OUT_2  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_13));
	Console::Instance()->printf("EMERGENCE_OUT_1  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_14));
	Console::Instance()->printf("EMERGENCE_LIGHT  IS %d\r\n",di_test_2000_do.read(GPIOD, GPIO_Pin_15));

	do_test_2000_di.write(1, GPIOD, GPIO_Pin_11);			//拍下抱闸
	Console::Instance()->printf("BRAKE  IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_9));
	
	Console::Instance()->printf("bootlight  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_2));
	
	Console::Instance()->printf("WARNING LIGHT  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_4));
	
	Console::Instance()->printf("DO0  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_3));
	Console::Instance()->printf("DO1  IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_8));
	Console::Instance()->printf("DO2  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_7));
	Console::Instance()->printf("DO3  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_8));
	Console::Instance()->printf("DO4  IS %d\r\n",di_test_2000_do.read(GPIOC, GPIO_Pin_7));
	Console::Instance()->printf("DO5  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_5));
	Console::Instance()->printf("DO6  IS %d\r\n",di_test_2000_do.read(GPIOG, GPIO_Pin_6));
}

static void doTest2000Di()
{
	static bool DIO_on = true;
	if(1)
	{
		DIO_on = !DIO_on;
		do_test_2000_di.write(DIO_on, GPIOD, GPIO_Pin_11);
		do_test_2000_di.write(DIO_on, GPIOB, GPIO_Pin_14);
		do_test_2000_di.write(DIO_on, GPIOD, GPIO_Pin_8);
		do_test_2000_di.write(DIO_on, GPIOD, GPIO_Pin_10);
	}
}

static void canTest2000()
{
	static Timer can_tx_freq(50,50);
    const uint8_t dat_1to2[8] = {0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC};
    const uint8_t dat_2to1[8] = {0x5A,0x59,0x58,0x57,0x56,0x55,0x54,0x53};
    if (can_tx_freq.isAbsoluteTimeUp())
    {
        CanTxMsg msg;
        msg.IDE = CAN_Id_Standard;
		msg.RTR = CAN_RTR_Data;
		msg.DLC = 8;
		msg.StdId = 2;
        memcpy(msg.Data, dat_1to2, 8);
        CanRouter1.putMsg(msg);  
    }

    if (can_mailbox_test_2000.msgsInQue() > 0)
    {
        CanRxMsg rx_msg;        
        size_t i = 0;
        can_mailbox_test_2000.getMsg(&rx_msg);
        for (; i < 8; ++i)
        {
            if (rx_msg.Data[i] != dat_2to1[i])
            {
                //Message::Instance()->postMsg("Checking failed: CAN 2 to 1 transfer wrong data.(TE35 32 OR 33)");
                break;
            }
        }
    }
}

static void uartRun()
{
	static Timer uart_tx_freq(100,100);
	if (uart_tx_freq.isAbsoluteTimeUp())
    {	
		while(0 != usart3_test_2000.get_BytesInTxFifo()){}
		usart3_test_2000.send_Array(rs485_data, 5);
		Console::Instance()->printf("XSAFDASD\r\n");
	}
	if (usart3_test_2000.get_BytesInRxFifo() >= 1)
	{
		uint8_t temp[SURVEYOR_RX_DMA_BUF_LEN];
		uint8_t bytesInRxFifo = usart3_test_2000.read_RxFifo(temp);
		for(int j = 0; j < bytesInRxFifo; j++)
		{
//			if (temp[j] == 0xBB)
//			{
//				while(0 != _pcom[3]->get_BytesInTxFifo()){}
//				_pcom[3]->send_Array(rs232_data, 5);
//			}
		}
	}			
}
	
