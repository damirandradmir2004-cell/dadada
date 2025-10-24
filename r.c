#include <stm32f4xx.h>

char array[8] = {};
void RCC_Init(void);
uint32_t i = 0;
int main(void)
{
  RCC->AHB1ENR|=RCC_AHB1ENR_GPIOEEN;
  RCC_Init();
  GPIOE->MODER&=~GPIO_MODER_MODER10;
  GPIOE->MODER&=~GPIO_MODER_MODER11;
  GPIOE->MODER&=~GPIO_MODER_MODER12;       
  GPIOE->PUPDR|= GPIO_PUPDR_PUPD10_0;
  GPIOE->PUPDR|= GPIO_PUPDR_PUPD11_0;
  GPIOE->PUPDR|= GPIO_PUPDR_PUPD12_0;
  GPIOE->MODER|=GPIO_MODER_MODER13_0;
  GPIOE->MODER|=GPIO_MODER_MODER14_0;
  GPIOE->MODER|=GPIO_MODER_MODER15_0;

while(1)
{
for (i=0; i<7; i++)
{
if ((GPIOE->IDR & GPIO_IDR_ID10)==0)
 array[i] = 1;
else if ((GPIOE->IDR & GPIO_IDR_ID11)==0)
 array[i] = 2;
else if ((GPIOE->IDR & GPIO_IDR_ID12)==0)
 array[i] = 3;
}
}
}