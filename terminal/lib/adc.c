#include <string.h>

#include "stm32f0xx_ll_adc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_gpio.h"
#include "fsm.h"

void fsm_adc_init(void *args)
{
        (void) args;
        /*
         * Setting analog AIN1
         */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_1, LL_GPIO_MODE_ANALOG);
        /*
         * Setting ADC
         */
        /* Turn on ADC1 as peripheral */
        LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);
        /* Clock selection */
        LL_RCC_HSI14_Enable();
        LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_ASYNC);
        while (LL_RCC_HSI14_IsReady() != 1);
        /* ADC Calibration */
        if (LL_ADC_IsEnabled(ADC1)) {
                LL_ADC_Disable(ADC1);
        }
        while (LL_ADC_IsEnabled(ADC1));
        LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_NONE);
        LL_ADC_StartCalibration(ADC1);
        while (LL_ADC_IsCalibrationOnGoing(ADC1));
        /* ADC Setting */
        LL_ADC_Enable(ADC1);
        LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_8B);
        LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);
        LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);
        LL_ADC_SetSamplingTimeCommonChannels(ADC1,
                                             LL_ADC_SAMPLINGTIME_239CYCLES_5);
        LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
        LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_1);
        LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);
        LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_PRESERVED);
        /* Enable ADC conversion */
        LL_ADC_REG_StartConversion(ADC1);
        fsm_set_state(FSM_PWM_INIT);
        return;
}

void fsm_adc_handler(void* args)
{
        uint8_t val = LL_ADC_REG_ReadConversionData8(ADC1);
        ((uint8_t *)args)[0] = 1;
        memcpy(((uint8_t *)args) + 1, &val, 1);
        fsm_set_state(FSM_TERM_MAIN);
        return;
}
