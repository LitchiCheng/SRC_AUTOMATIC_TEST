#ifndef _FW_COMMAND_DISPATCH_TASK_H_
#define _FW_COMMAND_DISPATCH_TASK_H_

#include "Timer.h"
#include "Task.h"
#include <stdint.h>
#include "Communication.h"
#include "GlobalData.h"

#include "socket.h"
#include "string.h"
#include "CCan.h"

class CCommandDispatchTask : public CTask
{
public:
	CCommandDispatchTask();

	virtual int doRun();	
	enum{
		INSTRUCT_BUFF_LEN = 300,
//		SOCKET_NUM = 5,
		LOCAL_PORT = 15003
	};
	
	void uartPortRun();
	
	class CCommandPortSender
	{
		friend class CCommandDispatchTask;
		
		public:
			CCommandPortSender();
			enum {TRANS_BUFF_LEN = 300};
			uint16_t getTransBuffSize() {return TRANS_BUFF_LEN;}
			int sendto(uint32_t len, uint8_t* ip, uint16_t port);
			uint8_t* transBuff() {return transBuff_;}
			uint32_t& transParam(uint16_t i) {return ((uint32_t*)transBuff_)[i];}
			
			uint8_t* cmderIP() {return cmderIP_;}
			void getCmderIP(uint8_t* ip) {memcpy(ip, cmderIP_, 4);}
			uint16_t cmderPort() {return cmderPort_;}
			
			uint8_t* rbkIP() {return rbkIP_;}
			void getRbkIp(uint8_t* ip) {memcpy(ip, rbkIP_, 4);}
			uint16_t rbkPort() {return rbkPort_;}
			
			uint32_t lastCmdTyp() {return lastCmdTyp_;}
			
			int sock_n();
			
		private:
			static uint8_t transBuff_[TRANS_BUFF_LEN];
//			bool isInitialized_;
		
		/*********** commander and robokit ***********/
		/* MCU response to the command from the cmder */
			uint8_t cmderIP_[4];
			uint16_t cmderPort_;
		/* MCU upload initiative to robokit */
			uint8_t rbkIP_[4];
			uint16_t rbkPort_;
		
			uint32_t lastCmdTyp_;
	};
	
private:
	void doDispatch();
	void udp_cfg(uint16_t port, uint16_t mode);

private:	
	static uint8_t _commandBuffer[INSTRUCT_BUFF_LEN];
};

class CCommandDispatchCanMailbox : public CCanRxMailbox_base
{
	public:

		CCommandDispatchCanMailbox()
		{
			for(int i = 0; i < MAX_ROUTER_NUM; i++)
			{
				canBaseRouter_[i] = NULL;
			}
			setExtId(0x15003);
		}

		virtual void pushMsg(const CanRxMsg& canMsg);
		bool attachToRouter(CCanRouter& refRouter)
		{
			for(int i = 0; i < MAX_ROUTER_NUM; i++)
			{
				if(canBaseRouter_[i] == NULL)
					canBaseRouter_[i] = &refRouter;
			}
			return CCanRxMailbox_base::attachToRouter(refRouter);
		}
		
		void timeoutCallback() { Console::Instance()->printf("Triowin PDO timeout\r\n"); }

	private:
		enum {MAX_ROUTER_NUM = 2};
		CCanRouter* canBaseRouter_[MAX_ROUTER_NUM];
};


typedef NormalSingleton<CCommandDispatchTask::CCommandPortSender> CmdSocket;
//typedef NormalSingleton<CCommandDispatchCanMailbox> CommandDispatchMailbox;

#endif	//_FW_CONFIG_TASK_H_
