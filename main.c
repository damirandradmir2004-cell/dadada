

#include <stdio.h>
#include <stm32f407xx.h>
void RCC_Init(void);
void ADC1_Init(void);

void ADC_IRQHandler(void);
void TIM1_Init(void);
void USART1_Init(void);
void USART1_Send(uint8_t* data, uint32_t len);
void DMA2_Stream7_Init(void);

void USART1_SEND_DMA(void);
uint32_t ms_count = 0;
const uint8_t bufferOUT[] = "USART-DMA OK\r";
int main(void) {


RCC_Init();
USART1_Init();
ADC1_Init();
DMA2_Stream7_Init();
SysTick_Config(84000);
  while (1)
  {
   // if ((ms_count % 1000) == 0)
    //{
       
      //USART1_Send((uint8_t*)"Hello sobaki negr\r", 18);
      //float voltage = (float)(3.3 * (float)ADC1->DR / (float)4095);
      //float value = (float)(3*ADC1->DR)/(float)4096;
      //printf("hello hhhhhhh%f\r",voltage);
        if (ms_count == 1000)
        {
            ms_count = 0;
            USART1_SEND_DMA();
        }
    }
  } 




void ADC1_Init(void)
{
  RCC->APB2ENR|=RCC_APB2ENR_ADC1EN;
  RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER|=(3<<GPIO_MODER_MODE4_Pos);
  ADC1->CR1&=~ADC_CR1_RES_Msk;

  ADC1->SMPR1|=ADC_SMPR2_SMP4_0;  //время выборки
  ADC1->SQR1&=~(ADC_SQR1_L); //колво конвертаций = 1
  ADC1->SQR3|=(4 << ADC_SQR3_SQ1_Pos);  //первая конвертация на 4 канале

  ADC1->CR1|=ADC_CR1_EOCIE;
  ADC1->CR2|=ADC_CR2_ADON|ADC_CR2_CONT;
  ADC1->CR2|=ADC_CR2_SWSTART;
}


void USART1_Init(void)
{
  RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  GPIOA->MODER|=(2<<GPIO_MODER_MODE9_Pos);
  GPIOA->MODER|=(2<<GPIO_MODER_MODE10_Pos);
  USART1->BRR = 0x1117;
  GPIOA->AFR[1]|=(7<<GPIO_AFRH_AFSEL9_Pos); //af для PA9 tx
  GPIOA->AFR[1]|=(7<<GPIO_AFRH_AFSEL10_Pos); //af для PA10 rx
  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE ;
  USART1->CR1 |= USART_CR1_UE;
  NVIC_EnableIRQ(USART1_IRQn);
}
void DMA2_Stream7_Init(void) {

 RCC->AHB1ENR|=RCC_AHB1ENR_DMA2EN;                        //тактирование dma
 RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;                       //тактирование gpio
 USART1->CR3|= USART_CR3_DMAT;
 DMA2_Stream7->PAR =  (uint32_t)&(USART1->DR);            //задание адреса регистра данных USART1 через указатель
 DMA2_Stream7->FCR&= ~(DMA_SxFCR_DMDIS);                  //прямой режим без FIFO 
 DMA2_Stream7->CR|=(4<<DMA_SxCR_CHSEL_Pos);               //канал 4 tx
 DMA2_Stream7->CR&= ~(DMA_SxCR_MBURST|DMA_SxCR_PBURST);   //одиночная передача (не пакетная)
 DMA2_Stream7->CR&= ~DMA_SxCR_DBM;                        //режим двойного буфера ВЫКЛ
 DMA2_Stream7->CR|=DMA_SxCR_PL;                           //самый высокий приоритет
 DMA2_Stream7->CR&=~DMA_SxCR_MSIZE_0;                      //размер данных памяти и периферии -байт (8 бит)
 DMA2_Stream7->CR&=~DMA_SxCR_PSIZE_0; 
 DMA2_Stream7->CR|=DMA_SxCR_MINC;                         //инкрементирование адреса памяти
 DMA2_Stream7->CR&=~DMA_SxCR_PINC;                        //выключение инкрементирования адреса перефирии 
 DMA2_Stream7->CR&=~DMA_SxCR_CIRC;                        // отключение циклической передачи данных
 DMA2_Stream7->CR|=DMA_SxCR_DIR_0;                        //передача из памяти в периферию   
 //DMA2_Stream7->CR |= DMA_SxCR_EN;
               
}


void SysTick_Handler(void)
{
 ms_count++;
}

void USART1_Send(uint8_t* data, uint32_t len)
//void USART1_Send(uint8_t* data, uint32_t len)
{
 for (int i = 0; i<len; i++)
 {
  while ((USART1->SR & USART_SR_TXE)==0);
  USART1->DR = data[i];
 }
 //while ((USART1->SR & USART_SR_TC)==0)
 //;
}
void USART1_SEND_DMA(void)
{   
        
        DMA2_Stream7->CR &= ~DMA_SxCR_EN;
        DMA2_Stream7->M0AR = (uint32_t)bufferOUT;
        DMA2_Stream7->NDTR = sizeof(bufferOUT)-1;   
        DMA2_Stream7->CR |= DMA_SxCR_EN;  //Включить DMA Stream для начала передачи
}

int __SEGGER_RTL_X_file_write(__SEGGER_RTL_FILE *__stream, const char *__s, unsigned __len) {
  while (__len > 0) {
    USART1->DR = *__s;
    __s++;
    __len--;
    while (! (USART1->SR & USART_SR_TC) );
  }
  return 0;
}

