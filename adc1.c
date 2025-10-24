
//ИНИЦИАЛИЗАЦИЯ АЦП В РЕЖИМЕ РЕГУЛЯРНОГО КАНАЛА//
#include <stm32f407xx.h>
void ADC5_Init(void)
{
  RCC->APB2ENR|=RCC_APB2ENR_ADC1EN;
  RCC->AHB1ENR|= RCC_AHB1ENR_GPIOAEN;

  GPIOA->MODER|=GPIO_MODER_MODE5; //аналоговый режим работы
  ADC1->SMPR2|=ADC_SMPR2_SMP5_0; //15 + 12    27 циклов тактирования ацп
  ADC1->SQR1|=~(ADC_SQR1_L); //длина последовательности 1
  ADC1->SQR3|=5<<ADC_SQR3_SQ1_Pos;  //первая конвертация 5 канал
  ADC1->CR1|=ADC_CR1_EOCIE; //разрешение прерывания по завершению преобразования
  ADC1->CR1|=ADC_CR1_AWDIE;
  ADC1->CR1|=ADC_CR1_AWDEN;
  NVIC_EnableIRQ(ADC_IRQn); // разрешение прерывания в NVIC
  ADC1->HTR = (8*4096)/10; //верхний порог 80 процентов;
  ADC1->LTR = (1*4096)/10; //нижний порог 10 процентов
  ADC1->CR2|=ADC_CR2_CONT|ADC_CR2_ADON;   //непрерывный режим/включение ацп 
  
  ADC1->CR2|=ADC_CR2_SWSTART;  //запуск измерения

}