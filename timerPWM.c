#include <stm32f407xx.h>
void TIM1_DMA_Init(void)

{

  RCC->APB2ENR|= RCC_APB2ENR_TIM1EN;
  RCC->AHB1ENR|= RCC_AHB1ENR_GPIOEEN;

  GPIOE->MODER|=GPIO_MODER_MODE13_1;
  GPIOE->OSPEEDR|=GPIO_OSPEEDR_OSPEED13_1;
  GPIOE->AFR[1]|=GPIO_AFRH_AFRH5_0;     //af1 для pe13

  TIM1->PSC = 83;  // установка 1 МГц с помощью предделителя
  TIM1->ARR = 999; // частота 1 кгц
  TIM1->CR1|=TIM_CR1_CMS;
  TIM1->CCR3 = 1;                //коэф заполнения
  TIM1->CCMR2|=TIM_CCMR2_OC3M;   //pwm mode 2
  TIM1->CCMR2&=~(TIM_CCMR2_CC3S); //режим работы - выход
  TIM1->CCER|=TIM_CCER_CC3E;  //вкл OC3
  TIM1->BDTR|=TIM_BDTR_MOE;  //вкл выхода в блоке dead time
  TIM1->CR1|= TIM_CR1_CEN; //разрешение счета
  TIM1->EGR|=TIM_EGR_UG;
 
}