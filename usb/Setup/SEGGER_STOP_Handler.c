/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2025 SEGGER Microcontroller GmbH                  *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system                           *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       OS version: V5.20.0.0                                        *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
Purpose   : Generic SEGGER STOP handler for Cortex-M and Embedded Studio
Literature: [1] Stack Overflow Prevention
                https://wiki.segger.com/Stack_Overflow_Prevention
            [2] ARMv7-M Architecture Reference Manual
                \\fileserver\Techinfo\Company\ARM\ArchitectureV7\DDI0403E_e_armv7m_arm.pdf
*/

#include "stdio.h"
#include "SEGGER_STOP.h"

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
static void _HandleStackError      (unsigned long SP, unsigned long Limit, unsigned long Caller, unsigned long CONTROL) __attribute__((used, no_stack_overflow_check));
       void __SEGGER_STOP_X_OnError(void)                                                                               __attribute__((naked, no_stack_overflow_check));

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define CONTROL_SPSEL_MASK  (1uL << 1)  // Defines the used stack pointer, [2] B1.4.4 The special-purpose CONTROL register, page 519

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _HandleStackError()
*
*  Function description
*    Example error handler for stack overflow prevention.
*    Log error and halt system.
*
*  Parameters
*    SP     : New (overflown) stack pointer value.
*    Limit  : Stack pointer limit value.
*    Caller : Caller of the stack checking function.
*    CONTROL: CONTROL[1] SPSEL: 0 = MSP, 1 = PSP.
*
*  Additional information
*    The error handler must not return.
*    It should trigger a system reset or stay in an endless loop.
*    This function must not be compiled with stack overflow checks
*    and must not call any embOS API.
*/
static void _HandleStackError(unsigned long SP, unsigned long Limit, unsigned long Caller, unsigned long CONTROL) {
  if ((CONTROL & CONTROL_SPSEL_MASK) != 0u) {
    printf("Task stack overflow prevented at address 0x%.8lX.\n", Caller);
    printf("Task stack pointer: 0x%.8lX\n", SP);
    printf("Task stack limit  : 0x%.8lX\n", Limit);
  } else {
    printf("Main stack overflow prevented at address 0x%.8lX.\n", Caller);
    printf("Main stack pointer: 0x%.8lX\n", SP);
    printf("Main stack limit  : 0x%.8lX\n", Limit);
  }

  while (1) {
  }
}

/*********************************************************************
*
*       __SEGGER_STOP_X_OnError()
*
*  Function description
*   Callback called by stack check on stack overflow.
*   Reset main stack pointer to a safe value, switch to main stack
*   and call error handler _HandleStackError().
*
*  Additional information
*   This callback is tail-called by the stack checking functions.
*   It assumes that:
*     - R3 contains the stack limit value.
*     - R12 contains the new stack pointer value.
*     - LR contains the caller of the stack checking function.
*     - The stack pointer has not been adjusted.
*     - __stack_end__ is the start of the main stack.
*
*   This function does not follow the regular calling convention.
*   This callback is implemented as naked to make sure the compiler
*   does not add a prologue which might use the stack.
*/
void __SEGGER_STOP_X_OnError(void) {
  __asm volatile("cpsid    i                   \n"   // Disable interrupts
                 "mov      r0, r12             \n"   // Save overflowed SP
                 "mov      r1, r3              \n"   // Save SP limit
                 "sub      r2, lr, #5          \n"   // Save caller
                 "mrs      r3, CONTROL         \n"   // Used stack, CONTROL[1] Stack status: 0 = MSP, 1 = PSP
                 "mov      r12, r3             \n"   // Use main stack
                 "bic      r12, #2             \n"
                 "msr      CONTROL, r12        \n"
                 "ldr      r12, =__stack_end__ \n"   // Reset main stack
                 "msr      msp, r12            \n"
                 "bl       _HandleStackError   \n"   // Call error handler
                 "b        .\n"                      // Stay here
                );
}

/*************************** End of file ****************************/
