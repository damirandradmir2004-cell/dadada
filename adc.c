#include <stm32f407xx.h>
  #include <stdio.h>
  #include <stdlib.h>

void RCC_Init(void);
void TIM1_DMA_Init(void);
void ADC1_Init(void);
void ADC1_DMA_Init(void);
void USART1_Init(void);
void DMA2_Stream7_Init(void);  //канал для TX USART1
void DMA2_Stream5_MEM2MEM_Init(void);
void DMA2_Stream7_IRQHandler(void);
void USART1_SEND_DMA(void);
const uint8_t bufferOUT[] = "USART-DMA OK\r" ;
//uint8_t bufferIN[sizeof(bufferOUT)] __attribute__((section(".fast"))); //конкретно задаем адрес в памяти

uint32_t ms_count = 0;
uint32_t seconds = 0;
int main(void)
{
 RCC_Init();
 USART1_Init();
 DMA2_Stream7_Init();
 DMA2_Stream5_MEM2MEM_Init();
 SysTick_Config(84000);
 while(1)
   {
        // Отправляем строку по DMA раз в секунду
        if (ms_count == 1000)
        {
            seconds++;
            //USART1_SEND_DMA();
        } 
    } 
}



void DMA2_Stream7_Init(void) {

 RCC->AHB1ENR|=RCC_AHB1ENR_DMA2EN;                        //тактирование dma
 RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;                       //тактирование gpio
 USART1->CR3|= USART_CR3_DMAT;
 DMA2_Stream7->PAR = (uint32_t)&(USART1->DR);            //задание адреса регистра данных USART1 через указатель
 DMA2_Stream7->FCR&= ~(DMA_SxFCR_DMDIS);                //прямой режим без FIFO 
 DMA2_Stream7->CR|=(4<<DMA_SxCR_CHSEL_Pos);                    //канал 4 tx
 DMA2_Stream7->CR&= ~(DMA_SxCR_MBURST|DMA_SxCR_PBURST);   //одиночная передача (не пакетная)
 DMA2_Stream7->CR&= ~DMA_SxCR_DBM;                     //режим двойного буфера ВЫКЛ
 DMA2_Stream7->CR|=DMA_SxCR_PL;                        //самый высокий приоритет
 DMA2_Stream7->CR&=~DMA_SxCR_MSIZE_0;                   //размер данных памяти и периферии -байт (8 бит)
 DMA2_Stream7->CR&=~DMA_SxCR_PSIZE_0; 
 DMA2_Stream7->CR|=DMA_SxCR_MINC;                      //инкрементирование адреса памяти
 DMA2_Stream7->CR&=~DMA_SxCR_PINC;                     //выключение инкрементирования адреса перефирии 
 DMA2_Stream7->CR&=~DMA_SxCR_CIRC;                     // отключение циклической передачи данных
 DMA2_Stream7->CR|=DMA_SxCR_DIR_0;                      //передача из памяти в периферию 
 DMA2_Stream7->CR|=DMA_SxCR_TCIE;                      //прерывание по завершению передачи данных
 NVIC_EnableIRQ(DMA2_Stream7_IRQn);
 

} 

void DMA2_Stream5_MEM2MEM_Init(void) {

 RCC->AHB1ENR|=RCC_AHB1ENR_DMA2EN;                        //тактирование dma                     
 DMA2_Stream5->PAR =  (uint32_t)bufferOUT;                //адрес передатчика 
// DMA2_Stream5->M0AR = (uint32_t)bufferIN;                //адрес приемника в регистре M0AR
 DMA2_Stream5->NDTR = sizeof(bufferOUT);                               // количество передаваемых данных
 DMA2_Stream5->FCR|=DMA_SxFCR_DMDIS;                      //отключение прямого режима, включение FIFO
 DMA2_Stream5->FCR|=DMA_SxFCR_FTH_0;                  
 DMA2_Stream5->CR&= ~(DMA_SxCR_MBURST|DMA_SxCR_PBURST);   //одиночная передача (не пакетная)
 DMA2_Stream5->CR&= ~DMA_SxCR_DBM;                     //режим двойного буфера ВЫКЛ
 DMA2_Stream5->CR|=DMA_SxCR_PL_0;                        //medium приоритет
 DMA2_Stream5->CR&=~DMA_SxCR_MSIZE;                   //размер данных памяти и периферии - байт
 DMA2_Stream5->CR&=~DMA_SxCR_PSIZE; 
 DMA2_Stream5->CR|=DMA_SxCR_MINC;                      //инкрементирование адреса памяти
 DMA2_Stream5->CR|=DMA_SxCR_PINC;                     // инкрементирование адреса перефирии 
 DMA2_Stream5->CR&=~DMA_SxCR_CIRC;                     // выключение циклической передачи данных
 DMA2_Stream5->CR|=DMA_SxCR_DIR_1;                      //передача из памяти в память 
 DMA2_Stream5->CR|=DMA_SxCR_TCIE;                      //прерывание по завершению передачи данных
 DMA2_Stream5->CR|= DMA_SxCR_EN;
               
} 

void USART1_SEND_DMA(void)
{   
        
        DMA2_Stream7->CR &= ~DMA_SxCR_EN;
        DMA2_Stream7->M0AR = (uint32_t)bufferOUT;
        DMA2_Stream7->NDTR = sizeof(bufferOUT)-1;
    // 4. Включить DMA Stream для начала передачи
        DMA2_Stream7->CR |= DMA_SxCR_EN; 
}

void DMA2_Stream7_IRQHandler(void)
{

}
void SysTick_Handler(void) {
  ms_count++;
} 