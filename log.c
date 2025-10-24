#include "log.h"


// 1. SRAM
#if LOG_ENABLE && (LOG_METHOD == LOG_SRAM)
  log_t Log;
  uint8_t log_current_index = 0; // Начинаем с message0

  void log_init_internal(void) { LOG_MESSAGE("Log init finished. %d, %d", sizeof(Log), sizeof(Log)/LOG_MESSAGE_TEXT_LENGTH); }

  void log_message_internal(const char* format, ...) {
      if (log_current_index >= LOG_FIELD_LIMIT) {
          return;//log_current_index = 0; // Циклический возврат к началу
      }

      // Массивоподобный доступ к полям структуры
      char (*messages)[LOG_FIELD_LIMIT] = (char (*)[LOG_MESSAGE_TEXT_LENGTH])&Log; // Приводим к массиву char[50]
      char* target = messages[log_current_index]; // Выбираем поле messageN

      // Записываем форматированное сообщение
      va_list args;
      va_start(args, format);
      vsnprintf(target, LOG_MESSAGE_TEXT_LENGTH, format, args); // Безопасная запись, max 50 байт
      va_end(args);

      // Автоинкремент индекса
      log_current_index = (log_current_index + 1) % LOG_MESSAGE_TEXT_LENGTH;
  }
#endif

// 2. USART
#if LOG_ENABLE && (LOG_METHOD == LOG_USART)
  #include "usart.h"
  #include <stdio.h>
  #include <stdarg.h>
  #include <string.h>
  void log_init_internal(void) {
    USART_init();
  }
  void log_message_internal(const char* format, ...) {
    if(USART_Initialized) {
      char target[LOG_MESSAGE_TEXT_LENGTH];

      // Записываем форматированное сообщение
      va_list args;
      va_start(args, format);
      vsnprintf(target, LOG_MESSAGE_TEXT_LENGTH, format, args); // Безопасная запись, max 50 байт
      va_end(args);
      
      strcat(target, "\r\n");

      USART_send_string(LOG_USART_INTERFACE, target);
    }
  }
#endif

// 3. ITM
#if LOG_ENABLE && (LOG_METHOD == LOG_ITM)

void log_init_internal(void) {

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Включить тактирование GPIOB
    GPIOB->MODER |= (0b10 << GPIO_MODER_MODE3_Pos); // PB3 в режим альтернативной функции
    GPIOB->AFR[0] |= (0x0 << GPIO_AFRL_AFSEL3_Pos); // AF0 для TRACESWO

    // Включаем тактирование TRACE
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    
    // Разблокируем ITM
    ITM->LAR = 0xC5ACCE55;
    
    // Настраиваем ITM
    ITM->TCR = (1 << ITM_TCR_TraceBusID_Pos) |  // Trace Bus ID = 1
               (1 << ITM_TCR_DWTENA_Pos) |      // Enable DWT packets
               (1 << ITM_TCR_SYNCENA_Pos) |     // Enable sync packets  
               (1 << ITM_TCR_ITMENA_Pos);       // Enable ITM
    
    ITM->TPR = 0xFFFFFFFF; // Все уровни привилегий могут писать
    ITM->TER = 0x00000001; // Включаем stimulus port 0
    
    // Небольшая задержка для инициализации
    for(volatile int i = 0; i < 1000; i++);
}

// Функция отправки символа напрямую в ITM регистры
static void ITM_SendChar_Direct(uint8_t ch) {
    // Проверяем что ITM включен и port 0 доступен
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) && (ITM->TER & 1)) {
        // Ждем пока stimulus port 0 не будет готов
        while (ITM->PORT[0].u32 == 0);
        // Отправляем байт
        ITM->PORT[0].u8 = ch;
    }
}

void log_message_internal(const char* format, ...) {
    char target[LOG_MESSAGE_TEXT_LENGTH];
    
    va_list args;
    va_start(args, format);
    vsnprintf(target, LOG_MESSAGE_TEXT_LENGTH, format, args);
    va_end(args);
    
    // Отправляем посимвольно
    char *p = target;
    while (*p) {
        ITM_SendChar_Direct(*p++);
    }
    ITM_SendChar_Direct('\r');
    ITM_SendChar_Direct('\n');
}
#endif

// 4. RTT
#if LOG_ENABLE && (LOG_METHOD == LOG_RTT)
void log_message_internal(const char* format, ...) {
    char target[LOG_MESSAGE_TEXT_LENGTH];
    
    va_list args;
    va_start(args, format);
    vsnprintf(target, LOG_MESSAGE_TEXT_LENGTH, format, args);
    va_end(args);
    
    SEGGER_RTT_WriteString(0, target);
}
#endif