/**
  ******************************************************************************
  * @file    EEPROM_Emulation/src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    10-October-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 


#ifdef __cplusplus
#include "Timer.h"
#include "Console.h"
#include "CUsart.h"

extern "C" {
#endif 
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */

  while(1)
	{
		Console::Instance()->runTransmitter();	
	}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**********************************************************************************************************
*��������SysTick_Handler()
*������void
*����ֵ��void
*���ܣ�ϵͳ���Ķ�ʱ���жϺ���
**********************************************************************************************************/

void SysTick_Handler(void)
{
	CPUTIMER0_ISR();
}
/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

//
//void DMA2_Stream7_IRQHandler(void)
//
void DMA2_Stream7_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
}

//uart2_tx
void DMA1_Stream6_IRQHandler(void)
{
	USART_ITConfig(USART2, USART_IT_TC, ENABLE);
	DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
}

//uart3_tx
void DMA1_Stream3_IRQHandler(void)
{
	USART_ITConfig(USART3, USART_IT_TC, ENABLE);
	DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
}

//uart4_Tx
void DMA1_Stream4_IRQHandler(void)
{
	USART_ITConfig(UART4, USART_IT_TC, ENABLE);
	DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
}

//uart5_tx
void DMA1_Stream7_IRQHandler(void)
{
	USART_ITConfig(UART5, USART_IT_TC, ENABLE);
	DMA_ClearITPendingBit(DMA1_Stream7, DMA_IT_TCIF7);
}

//uart6_tx
void DMA2_Stream6_IRQHandler(void)
{
	USART_ITConfig(USART6, USART_IT_TC, ENABLE);
	DMA_ClearITPendingBit(DMA2_Stream6, DMA_IT_TCIF6);
}

void USART2_IRQHandler(void)
{
	if (USART_GetFlagStatus(USART2, USART_FLAG_TXE))
	{
		GPIO_ResetBits(UART2_Dir_Port, UART2_Dir_Pin);
		USART_ITConfig(USART2, USART_IT_TC, DISABLE);	
	} else {}
	
	USART_ClearITPendingBit(USART2, USART_IT_TC);

}

void USART3_IRQHandler(void)
{
	if (USART_GetFlagStatus(USART3, USART_FLAG_TXE))
	{
		GPIO_ResetBits(UART3_Dir_Port, UART3_Dir_Pin);
		USART_ITConfig(USART3, USART_IT_TC, DISABLE);
	} else {}
	
	USART_ClearITPendingBit(USART3, USART_IT_TC);
}

void UART4_IRQHandler(void)
{
	if (USART_GetFlagStatus(UART4, USART_FLAG_TXE))
	{
		GPIO_ResetBits(UART4_Dir_Port, UART4_Dir_Pin);
		USART_ITConfig(UART4, USART_IT_TC, DISABLE);
	} else {}
	
	USART_ClearITPendingBit(UART4, USART_IT_TC);
}

void UART5_IRQHandler(void)
{
	if (USART_GetFlagStatus(UART5, USART_FLAG_TXE))
	{
		GPIO_ResetBits(UART5_Dir_Port, UART5_Dir_Pin);
		USART_ITConfig(UART5, USART_IT_TC, DISABLE);
	} else {}
	
	USART_ClearITPendingBit(UART5, USART_IT_TC);
}
	
void USART6_IRQHandler(void)
{
	if (USART_GetFlagStatus(USART6, USART_FLAG_TXE))
	{
		GPIO_ResetBits(UART6_Dir_Port, UART6_Dir_Pin);
		USART_ITConfig(USART6, USART_IT_TC, DISABLE);
	} else {}
	
	USART_ClearITPendingBit(USART6, USART_IT_TC);
}

#ifdef __cplusplus
}
#endif 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
