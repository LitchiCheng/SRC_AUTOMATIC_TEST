#ifndef _DIO_TEST_H
#define _DIO_TEST_H

#define USE_STM32_F4 

#if defined(USE_STM32_F4)
    #include "stm32f4xx.h"
#else

#endif

class DIO_TEST
{
private:
    /* data */
    GPIO_InitTypeDef GPIO_InitStructure;
public:
  
    DIO_TEST(/* args */){};
    void open();
    void write(bool is_high, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
    unsigned int read(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
    void ioctl(int ioctl_cmd);
    ~DIO_TEST(){};
};



#endif