#include <stm32f4xx.h>

void RCC_Init(void); //прототип функции  тактирования
void EXTI15_10_IRQHandler(void);

#define BTN_PRESS_CNT 4 // счетчик опроса кнопки при антидребезге
#define BTN_CHECK_MS 10  //период опроса кнопок при антидребезге
// адресация устройства на шине i2c
#define I2C_DEV_ADRR 0xA0
#define I2C_WR_BIT 0x00
#define I2C_RD_BIT 0x01
#define I2C_DEV_ADDR_RD    (I2C_DEV_ADRR + I2C_RD_BIT)
#define I2C_DEV_ADDR_WR    (I2C_DEV_ADRR + I2C_WR_BIT)

//адресация внутри EEPROM
#define EEPROM_WR_START_ASSR 0x08
#define EEPROM_WR_LEN 4
#define EEPROM_PAGE_LEN_BYTES 8
#define EEPROM_RD_START_ADDR 0x08
#define EEPROM_RD_LEN 4
uint16_t ms_count = 0; //счетчик мс для опроса кнопок
char i2c_tx_array[8] = {1, 2, 3, 4, 5, 6, 7};
char i2c_rx_array[8] ={};
//реализация антидребезга
char S1_cnt = 0;       //счетчик нажатий S1
char S2_cnt = 0;       //счетчик нажатий S2
char S3_cnt = 0;       //cчетчик нажатий S3
 
char S1_state = 0;         //состояния кнопок, 1 = нажата, 0 - отпущена
char S2_state = 0;
char S3_state = 0;
///

//FSM флаги состояния по которым автомат переходит в состояния
char IDLE_flag = 0;
char EEPROM_WRITE_flag = 0;
char EEPROM_READ_flag = 0;
char ADDR_INC_flag = 0;

//флаги сброса состояния
char IDLE_out = 0;
char EEPROM_WRITE_out = 0;
char EEPROM_READ_out = 0;
char ADDR_INC_out = 0;
void I2C_Init(void)
{
  RCC->AHB1ENR|=RCC_AHB1ENR_GPIOBEN;
  RCC->APB1ENR|=RCC_APB1ENR_I2C1EN;

  GPIOB->MODER |= GPIO_MODER_MODE8_1;
  GPIOB->MODER |= GPIO_MODER_MODE9_1;
  GPIOB->OTYPER|=(GPIO_OTYPER_OT8|GPIO_OTYPER_OT9);
  GPIOB->AFR[1] |= (4 << 0); // AF4 для PB8 (I2C1_SCL)
  GPIOB->AFR[1] |= (4 << 4); // AF4 для PB9 (I2C1_SDA)
  I2C1 -> CR2|= (42 << I2C_CR2_FREQ_Pos);
  I2C1 -> CCR|= (210 << I2C_CCR_CCR_Pos);  //100 кгЦ i2c
  I2C1 -> CCR &= ~(I2C_CCR_FS);
  I2C1 -> TRISE |= (43<<I2C_TRISE_TRISE_Pos);  //I2C1__CR2_FREQ + 1
  I2C1 -> OAR1 &= ~(I2C_OAR1_ADDMODE); //использование 7-ми битного адреса
  I2C1 -> CCR &= ~(I2C_CCR_FS); //явное использование standart mode f=100 кгц
  I2C1 -> CR1|= I2C_CR1_PE;
  I2C1 -> CR1|= I2C_CR1_ACK; 
}


 
void SysTick_Handler(void){        //прерывание от Systick таймера, выполняющееся с периодом 1000 мкс
    ms_count++;
}
void BTN_Check(void)
{
 if (ms_count > BTN_CHECK_MS){
   ms_count = 0;
 //опрос кнопки S1
   if ((GPIOE->IDR & GPIO_IDR_ID10) == 0){
    if(S1_cnt < BTN_PRESS_CNT){
       S1_cnt++;
       S1_state = 0;
      }
    else
    {
    S1_state = 1;
    }
    }
   else{
    S1_state = 0;
    S1_cnt = 0;
}
 //опрос кнопки S2
 if ((GPIOE->IDR & GPIO_IDR_ID11) == 0){
  if(S2_cnt < BTN_PRESS_CNT){
     S2_cnt++;
     S2_state = 0;
    }
  else
    {
    S2_state = 1;
    }
    }
 else{
  S2_state = 0;
    S2_cnt = 0;
}
 //опрос кнопки S3
  if ((GPIOE->IDR & GPIO_IDR_ID12) == 0){
    if(S3_cnt < BTN_PRESS_CNT){
      S3_cnt++;
      S3_state = 0;
    }
    else
    {
    S3_state = 1;
    }
  }
  else{
    S3_state = 0;
    S3_cnt = 0;
}
}
}
void State_Flag_Gen(void){
  if(!S1_state) EEPROM_WRITE_out = 0;
  else 
    EEPROM_WRITE_flag = S1_state & ~(EEPROM_WRITE_out);
  if(!S2_state) ADDR_INC_out = 0;
  else 
    ADDR_INC_flag = S2_state & ~(ADDR_INC_out); 
  if(!S3_state) EEPROM_READ_out = 0;
  else 
    EEPROM_READ_flag = S3_state & ~(EEPROM_READ_out);
  } 


  void I2C_Start_gen(void){
    I2C1->CR1|=I2C_CR1_START;
    while((I2C1->SR1 & I2C_SR1_SB) == 0){};  //ожидание START - условия на шине I2C
  }
    void I2C_Stop_gen(void){
      I2C1->CR1|=I2C_CR1_STOP;
      //ожидание stop - условия на шине I2C                 
    }

  void I2C_TxDeviceADDR(char device_adress, char RW_bit){
    I2C1->DR = (device_adress + RW_bit);      //отправить в I2C_DR адрес устройства и бит WR
    while((I2C1->SR1& I2C_SR1_ADDR) == 0) {}   //ждем флаг I2C_SR1_ADDR = 1. Пока завершится передача байта адреса
   (void)I2C1 -> SR1;
  (void)I2C1 -> SR2;    //очистка бита ADDR чтенеием регистров SR1 SR2

 }
  
  void I2C_Write(char start_addr, char data[], uint16_t data_len){
  I2C1->CR1|=I2C_CR1_ACK;// включить генерацию битов ACK

  while((I2C1->SR2&I2C_SR2_BUSY) !=0){}// проверить свободна ли шина I2C

  I2C_Start_gen(); // генерация START - условия

  I2C_TxDeviceADDR(I2C_DEV_ADRR, I2C_WR_BIT); // отправить адресс устройства I2C

  I2C1 -> DR = start_addr; 
  while((I2C1 -> SR1 & I2C_SR1_TXE) == 0){};
  // цикл for запись данных в I2C-устройство
  for (uint16_t i = 0; i<data_len; i++){
    I2C1 -> DR = data[i];
    while((I2C1 -> SR1 & I2C_SR1_TXE) == 0) {};
  // генерация STOP-условия
  }
  I2C_Stop_gen();
  }
  
    
  void I2C_read(char start_addr, char rx_data[], uint16_t data_len){
   I2C1->CR1|=I2C_CR1_ACK;// включить генерацию битов ACK

  while((I2C1->SR2&I2C_SR2_BUSY) !=0){}// проверить свободна ли шина I2C

  I2C_Start_gen(); // генерация START - условия

  I2C_TxDeviceADDR(I2C_DEV_ADRR, I2C_WR_BIT); //передать адрес устройства I2C и бит WR
  I2C1 -> DR = start_addr;    //передача адреса ячейки памят EEPROM начиная с которой будем читать данные
  while((I2C1 -> SR1 & I2C_SR1_TXE) ==0) {}  //ждем флаг I2C_SR1_TXE = 1. Пока завершится передача байта данных
  
  I2C_Start_gen(); //повторное start условие
  I2C_TxDeviceADDR(I2C_DEV_ADRR, I2C_RD_BIT); // передать адресс устройства I2C и бит RD

 

  //цикл чтения данных data_len-1
  for(uint16_t i=0; i<data_len-1; i++){
  while((I2C1->SR1&I2C_SR1_RXNE) == 0 ) {};  
    rx_data[i]=I2C1->DR;
  }
  I2C1 -> CR1 &= ~(I2C_CR1_ACK); //отключить генерацию ACK

  //принять байт data_len
  while ((I2C1 -> SR1 & I2C_SR1_RXNE) == 0){}; //ждем флаг I2C_SR1_TXE = 1. Пока завершится передача байта данных
  rx_data[data_len-1] = I2C1 ->DR;

   I2C_Stop_gen();//генерация стоп условия
  }

int main(void) {
   RCC_Init();
   enum states{
   IDLE = 0,
   EEPROM_WRITE,
   EEPROM_READ,
   ADDR_INC
   };
  enum states FSM_state = IDLE;

  char eeprom_addr = 0; //адрес чтения или записи с EEPROM
  char addr_offset = 0; //смещение адреса EEPROM относительно начального значения

  I2C_Init();
  RCC->AHB1ENR|=RCC_AHB1ENR_GPIOEEN;       
  GPIOE->PUPDR|= GPIO_PUPDR_PUPD10_0;
  GPIOE->PUPDR|= GPIO_PUPDR_PUPD11_0;
  GPIOE->PUPDR|= GPIO_PUPDR_PUPD12_0;

  GPIOE->MODER|= GPIO_MODER_MODE13_0;
  GPIOE->MODER|= GPIO_MODER_MODE14_0;
  GPIOE->MODER|= GPIO_MODER_MODE15_0;   
  RCC-> APB2ENR |= RCC_APB2ENR_SYSCFGEN; //включение тактирования порта


  GPIOE -> BSRR|= GPIO_BSRR_BS13;
  GPIOE -> BSRR|= GPIO_BSRR_BS14;
  GPIOE -> BSRR|= GPIO_BSRR_BS15;

SysTick_Config(84000);        //настройка systick таймера на время отрабатывания = 1 мс
                              //84000 - количество тактов до генерации прерывания, сколько раз будет считать таймер, оно равно
                              // 84 000 000 Гц (частота тактирования шины AHB)/ 1000 мкс

    while (1) {
      
      BTN_Check();

     State_Flag_Gen();
      


     
      //блок переключения состояний
      if(EEPROM_WRITE_flag) FSM_state = EEPROM_WRITE;
      else{
        if(ADDR_INC_flag) FSM_state = ADDR_INC;
        else{
         if(EEPROM_READ_flag) FSM_state = EEPROM_READ;
         else FSM_state = IDLE;
         }
      }

         // FSM блок реализации основной логики
      switch(FSM_state){
        case IDLE:
          break;

        case EEPROM_WRITE:
          I2C_Write(eeprom_addr, i2c_tx_array, EEPROM_WR_LEN); //запись массива данных в EEPROM по адрессу eeprom_addr
          EEPROM_WRITE_out = 1;
          break;

        case ADDR_INC:
          if(addr_offset<EEPROM_WR_LEN) addr_offset++;
          else addr_offset = 0;
          ADDR_INC_out = 1;
          break;

        case  EEPROM_READ:
          I2C_read(eeprom_addr, i2c_rx_array, EEPROM_RD_LEN);//чтение массива данных из EEPROM по адрессу eeprom_addr
          EEPROM_READ_out = 1;
          break;
      } //switch(FSM_state)

        eeprom_addr = EEPROM_RD_START_ADDR + addr_offset;    //вычисление адреса в EEPROM через базовый адрес
        GPIOE->ODR = ((~(addr_offset & 0x07))<<13);          //индикация смещения адреса на LEDS

    }  //while

}  //main

