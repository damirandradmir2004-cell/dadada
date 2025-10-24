#include <stm32f407xx.h>
void ADC1_DMA_Init(void)
{
  RCC->APB2ENR|=RCC_APB2ENR_ADC1EN;
  RCC->AHB1ENR|= RCC_AHB1ENR_GPIOAEN;

  GPIOA->MODER|=GPIO_MODER_MODE5; //аналоговый режим работы
  ADC->CCR|=ADC_CCR_ADCPRE; //PCLK / 8
  ADC1->SMPR2|=ADC_SMPR2_SMP5_2; //15 + 12    27 циклов тактирования ацп
  ADC1->SQR1|=~(ADC_SQR1_L); //длина последовательности 1
  ADC1->SQR3|=5<<ADC_SQR3_SQ1_Pos;  //первая конвертация 5 канал
  ADC1->CR2|=ADC_CR2_DMA | ADC_CR2_DDS; //включение непрерывных запросов dma, запрос dma будет генерироваться после каждой конвертации

  ADC1->CR2|=ADC_CR2_CONT|ADC_CR2_ADON;   //непрерывный режим/включение ацп 
  
  ADC1->CR2|=ADC_CR2_SWSTART;  //запуск измерения

}