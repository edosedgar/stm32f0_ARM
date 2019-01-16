#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_tim.h"

#include "FreeRTOS.h"
#include "task.h"
#include "os_routines.h"

/*
 * Private task notifiers
 */
static TaskHandle_t led_task_notify;
static TaskHandle_t pwm_task_notify;

/**
  * System Clock Configuration
  * The system Clock is configured as follow :
  *    System Clock source            = PLL (HSI/2)
  *    SYSCLK(Hz)                     = 48000000
  *    HCLK(Hz)                       = 48000000
  *    AHB Prescaler                  = 1
  *    APB1 Prescaler                 = 1
  *    HSI Frequency(Hz)              = 8000000
  *    PLLMUL                         = 12
  *    Flash Latency(WS)              = 1
  */
static void rcc_config(void)
{
        /* Set FLASH latency */
        LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

        /* Enable HSI and wait for activation*/
        LL_RCC_HSI_Enable();
        while (LL_RCC_HSI_IsReady() != 1);

        /* Main PLL configuration and activation */
        LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
                                    LL_RCC_PLL_MUL_12);

        LL_RCC_PLL_Enable();
        while (LL_RCC_PLL_IsReady() != 1);

        /* Sysclk activation on the main PLL */
        LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
        LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
        while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

        /* Set APB1 prescaler */
        LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

        /* Set systick to 1ms */
        SysTick_Config(48000000/1000);

        /* Update CMSIS variable (which can be updated also
         * through SystemCoreClockUpdate function) */
        SystemCoreClock = 48000000;
        return;
}

/*
 * LED GPIO configuration
 */
static void led_config(void)
{
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
        return;
}

/*
 * LED task
 */
void led_manager(void *p)
{
        (void) p;
        int i = 0;

        led_task_notify = xTaskGetCurrentTaskHandle();
        led_config();
        while (1) {
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                for (i = 0; i < 4; ++i)
                {
                        LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_9);
                        vTaskDelay(50);
                }
                
        }
        return;
}

/*
 * Button GPIO configuration
 */
static void btn_config(void)
{
        /*
         * Setting PIN A0
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);
}

/*
 * Button task
 */
void btn_manager(void *p)
{
        (void) p;

        btn_config();
        while (1) {
                if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0)) {
                        xTaskNotifyGive(led_task_notify);
                        xTaskNotifyGive(pwm_task_notify);
                        vTaskDelay(150);
                }
        }
}

/*
 * PWM configuration
 */
static void pwm_config(void)
{
        /*
         * Set PWM pin
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
        LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_8, LL_GPIO_AF_1);
        LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_8,
                                 LL_GPIO_OUTPUT_PUSHPULL);

        /*
         * PWM timer configuration
         */
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

        LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH3);

        LL_TIM_SetCounterMode(TIM3, LL_TIM_COUNTERMODE_UP);
        LL_TIM_SetAutoReload(TIM3, 48000);
        LL_TIM_SetPrescaler(TIM3, 1);

        LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_PWM1);
        LL_TIM_OC_EnableFast(TIM3, LL_TIM_CHANNEL_CH3);
        LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH3);
        LL_TIM_OC_SetCompareCH3(TIM3, 0);

        LL_TIM_GenerateEvent_UPDATE(TIM3);

        LL_TIM_EnableCounter(TIM3);
        return;
}

/*
 * pwm task
 */
void pwm_manager(void *p)
{
        (void) p;
        uint16_t pwm = 0;

        pwm_config();
        pwm_task_notify = xTaskGetCurrentTaskHandle();
        while (1) {
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                pwm += 4800;
                if(pwm > 48000)
                        pwm = 0;
                LL_TIM_OC_SetCompareCH3(TIM3, pwm);
                LL_TIM_GenerateEvent_UPDATE(TIM3);
        }
}

StackType_t blueTaskStack[128];
StaticTask_t blueTaskBuffer;

StackType_t btnTaskStack[128];
StaticTask_t btnTaskBuffer;

StackType_t pwmTaskStack[128];
StaticTask_t pwmTaskBuffer;

int main(void)
{
        rcc_config();

        xTaskCreateStatic(led_manager, "led_manager", 128, NULL, 2,
                          blueTaskStack, &blueTaskBuffer);
        xTaskCreateStatic(btn_manager, "btn_manager", 128, NULL, 1,
                          btnTaskStack, &btnTaskBuffer);
        xTaskCreateStatic(pwm_manager, "pwm_manager", 128, NULL, 1,
                          pwmTaskStack, &pwmTaskBuffer);
        vTaskStartScheduler();

        while (1);
        return 0;
}
