#include "stm32f0xx_ll_gpio.h"
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_9);
    while (1);
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
typedef void (*svcall_t)(void *);

__attribute__((naked)) __attribute__((optimize(1))) void SVC_Handler()
{
    register uint32_t *frame;
    register svcall_t call;
    register void *args;

    //assumes PSP in use when service_call() invoked
    asm volatile ("MRS %0, psp\n\t" : "=r" (frame) );
    call = (svcall_t)frame[0];
    args = (void *)(frame[1]);
    call(args);
    return;
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}
