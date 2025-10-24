/*#include <stm32f407xx.h>
#include "log.h"
#define LED1_ON GPIOE->BSRR|=GPIO_BSRR_BR_13;
#define LED1_OFF GPIOE->BSRR|=GPIO_BSRR_BS_13;
void TIM2_Init(void);
void RCC_Init(void);
void ADC1_Init(void);
void ADC_IRQHandler(void);
void Debug_ADC_Values(void);
int main(void) {
  volatile uint32_t adc_value = ADC1->JDR1;
  RCC->AHB1ENR|=RCC_AHB1ENR_GPIOEEN;
  GPIOE->MODER|=GPIO_MODER_MODER13_0;
  GPIOE->BSRR|=GPIO_BSRR_BS_13;
  RCC_Init();
  TIM2_Init();
  ADC1_Init();
  volatile uint32_t current_value=ADC1->JDR1;
  volatile uint32_t cnt_value=TIM2->CNT;
  while(1)
{
}
}
void TIM2_Init(void)
{

  RCC->APB1ENR|= RCC_APB1ENR_TIM2EN;
  //RCC->AHB1ENR|= RCC_AHB1ENR_GPIOEEN;
  TIM2->PSC = 41;  // установка 1 МГц с помощью предделителя
  TIM2->ARR = 999; // частота 1 кгц
  TIM2->CR2 |= TIM_CR2_MMS_1;  //переполнение счетчика является выходным тригером TRGO EVENT
  TIM2->CR1|= TIM_CR1_CEN; //разрешение счета
 
}

void ADC1_Init(void)
{
  RCC->APB2ENR|=RCC_APB2ENR_ADC1EN;
  RCC->AHB1ENR|= RCC_AHB1ENR_GPIOAEN;

  GPIOA->MODER|=GPIO_MODER_MODE5; //аналоговый вход
  ADC1->CR1|=ADC_CR1_RES_0; //10 битное разрешение

  ADC1->CR2|=ADC_CR2_JEXTEN_0; //тактирование по восходящему фронту UPDATE EVENT
  ADC1->CR2 |= ADC_CR2_JEXTSEL_1|ADC_CR2_JEXTSEL_0;  //тактирование по TRGO event от таймера 2

  ADC1->SMPR2|= ADC_SMPR2_SMP5_0; //15 циклов тактирования ацп
  
  
  ADC1->JSQR&=~(ADC_JSQR_JL); //длина последовательности 1
  ADC1->JSQR|=(5<<ADC_JSQR_JSQ4_Pos); //первая конвертация 5 канал
  ADC1->CR1&=~(ADC_CR1_AWDCH_Msk);
  ADC1->CR1|=ADC_CR1_AWDCH_0|ADC_CR1_AWDCH_2; //watchdog на 5 канал
  
  ADC1->CR1|=ADC_CR1_JAWDEN; //разрешение watchdog на инжектированных каналах
  //ADC1->CR1 |= (5 << ADC_CR1_AWDCH_Pos); // Канал 5 для сторожа
  //ADC1->CR1|=ADC_CR1_AWDCH_0|ADC_CR1_AWDCH_2; //watchdog на 5 канал


  ADC1->CR1|=ADC_CR1_AWDIE; //разрешение прерывания по cторожевому таймеру
  ADC1->CR1 |= ADC_CR1_JEOCIE;

   

  NVIC_EnableIRQ(ADC_IRQn); // разрешение прерывания в NVIC
  ADC1->CR2|=ADC_CR2_ADON;   //включение ацп 
  ADC1->CR2|=ADC_CR2_JSWSTART;  //запуск измерения

  ADC1->HTR = 818; //верхний порог 80 процентов;
  ADC1->LTR = 102; //нижний порог 10 процентов
  

}

void ADC_IRQHandler(void)
{

  if (ADC1->SR & ADC_SR_AWD)
  {
  LED1_ON
  for (uint32_t i = 1; i<10000; i++)
  ADC1->SR&=~ADC_SR_AWD;
  }
  else LED1_OFF
 NVIC_ClearPendingIRQ(ADC_IRQn);
 } */