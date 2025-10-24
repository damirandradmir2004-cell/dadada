#include <stm32f407xx.h>

char *buffer = " ";
void DMA2_Stream0_ADC_Init(void) {

 RCC->AHB1ENR|=RCC_AHB1ENR_DMA2EN;
 
 // DMA2 Stream 0 Channel 0 - ADC1 смотри DMA REQUEST MAPPING 

 DMA2_Stream0->PAR =  (uint32_t)&(ADC1->DR);            //задание адреса регистра данных ацп через указатель
 DMA2_Stream0->M0AR = (uint32_t)buffer;                //адрес памяти в регистре M0AR
 DMA2_Stream0->NDTR = 8;                               // количество передаваемых данных
 DMA2_Stream0->FCR&= ~(DMA_SxFCR_DMDIS);                //прямой режим без FIFO 
 DMA2_Stream0->CR&= ~DMA_SxCR_CHSEL;                    //канал 0
 DMA2_Stream0->CR&= ~DMA_SxCR_MBURST|DMA_SxCR_PBURST;   //одиночная передача (не пакетная)
 DMA2_Stream0->CR&= ~DMA_SxCR_DBM;                     //режим двойного буфера ВЫКЛ
 DMA2_Stream0->CR|=DMA_SxCR_PL;                        //самый высокий приоритет
 DMA2_Stream0->CR|=DMA_SxCR_MSIZE_0|DMA_SxCR_PSIZE_0;  //размер данных памяти и периферии - полуслово (16 бит) 
 DMA2_Stream0->CR|=DMA_SxCR_MINC;                      //инкрементирование адреса памяти
 DMA2_Stream0->CR&=~DMA_SxCR_PINC;                     //выключение инкрементирования адреса перефирии 
 DMA2_Stream0->CR|=DMA_SxCR_CIRC;                      //циклическая передача данных
 DMA2_Stream0->CR&=~DMA_SxCR_DIR;                      //передача из перефирии в память
 DMA2_Stream0->CR|=DMA_SxCR_TCIE;                      //прерывание по завершению передачи данных
 NVIC_EnableIRQ(DMA2_Stream0_IRQn);
 
 DMA2_Stream0->CR|=DMA_SxCR_EN;                        //включение нулевого потока
               
}  