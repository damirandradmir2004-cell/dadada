#ifndef LOG_H
#define LOG_H

// 1. Включить/выключить логи
#define LOG_ENABLE 1
// 2. Длина одной записи
#define LOG_MESSAGE_TEXT_LENGTH 50

//#define LOG_RTT   'R'
//#define LOG_ITM   'I'

#define LOG_USART 'U'
#define LOG_SRAM  'S'
// 3. Выбрать метод логирования
#define LOG_METHOD LOG_SRAM


// Общий макрос LOG_MESSAGE позволяет подставлять нужную реализацию лога
// или ничего не подставлять, если логи выключены
#if LOG_ENABLE
  #define LOG_MESSAGE(fmt, ...) { \
    void log_message_internal(const char*, ...); \
    log_message_internal(fmt, ##__VA_ARGS__); \
  }
  #define LOG_INIT() { \
    void log_init_internal(void); \
    log_init_internal(); \
  }
#else
  // подставляем в прод ничего
  #define LOG_MESSAGE(fmt, ...)
#endif

// 1. SRAM. Храним лог прямо в памяти, используем в Watch
#if LOG_ENABLE && (LOG_METHOD == LOG_SRAM)
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
// Этот макрос генерирует 100 строк, можно их убрать или добавить из Х-макроса и поменять LOG_FIELD_LIMIT
// каждая строка занимает байт LOG_MESSAGE_MESSAGE_LENGTH
// для 100 строк по 50Б занимать будет 5000Б в SRAM
#define LOG_FIELD_LIMIT 100
#define LOG_FIELDS \
    X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9) \
    X(10) X(11) X(12) X(13) X(14) X(15) X(16) X(17) X(18) X(19) \
    X(20) X(21) X(22) X(23) X(24) X(25) X(26) X(27) X(28) X(29) \
    X(30) X(31) X(32) X(33) X(34) X(35) X(36) X(37) X(38) X(39) \
    X(40) X(41) X(42) X(43) X(44) X(45) X(46) X(47) X(48) X(49) \
    X(50) X(51) X(52) X(53) X(54) X(55) X(56) X(57) X(58) X(59) \
    X(60) X(61) X(62) X(63) X(64) X(65) X(66) X(67) X(68) X(69) \
    X(70) X(71) X(72) X(73) X(74) X(75) X(76) X(77) X(78) X(79) \
    X(80) X(81) X(82) X(83) X(84) X(85) X(86) X(87) X(88) X(89) \
    X(90) X(91) X(92) X(93) X(94) X(95) X(96) X(97) X(98) X(99)

#define X(n) char message##n[LOG_MESSAGE_TEXT_LENGTH];
typedef struct {
    LOG_FIELDS
} log_t;
#undef X

extern log_t Log;
#endif

#endif

// 2. USART. Открываем терминал usart
#if LOG_ENABLE && (LOG_METHOD == LOG_USART)
  #define LOG_USART_INTERFACE USART1
#endif

// 3. Нужен SWO пин
#if LOG_ENABLE && (LOG_METHOD == LOG_ITM)
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "stm32f407xx.h"
#endif

// 4. Нужно ставить SEGGER_RTT.h
#if LOG_ENABLE && (LOG_METHOD == LOG_RTT)
  #error "SEGGER_RTT.h not found"
#endif

