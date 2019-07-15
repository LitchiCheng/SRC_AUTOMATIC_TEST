#include "DIO_TEST.h"

void DIO_TEST::open()
{
    #if defined(USE_STM32_F4) 
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOG, ENABLE);
    #else

    #endif
}

void DIO_TEST::ioctl(int ioctl_cmd)
{
    if (ioctl_cmd == 0x01)
    {
        #if defined(USE_STM32_F4) 
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
			GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9| GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 |GPIO_Pin_12; 
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 |GPIO_Pin_15 ;
            GPIO_Init(GPIOB, &GPIO_InitStructure);
//            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
//            GPIO_Init(GPIOG, &GPIO_InitStructure);
        #else

        #endif
    }
    else if (ioctl_cmd == 0x02)
    {
        #if defined(USE_STM32_F4) 
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;  
            GPIO_Init(GPIOD, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6| GPIO_Pin_7| GPIO_Pin_8| GPIO_Pin_9;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7| GPIO_Pin_8 ;
            GPIO_Init(GPIOG, &GPIO_InitStructure);
        #else

        #endif
    }
    #
    
}

void DIO_TEST::write(bool is_high, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    if (is_high == true)
        GPIO_SetBits(GPIOx, GPIO_Pin);
    else
        GPIO_ResetBits(GPIOx, GPIO_Pin);
}

unsigned int DIO_TEST::read(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    unsigned int value = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
    return value;
}
