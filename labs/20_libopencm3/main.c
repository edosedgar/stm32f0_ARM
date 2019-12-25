#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

void delay(int ticks)
{
    while (ticks--);
}

int main()
{
    // First, let's ensure that our clock is running off the high-speed
    // internal oscillator (HSI) at 48MHz
    rcc_clock_setup_in_hsi_out_48mhz();

    // Since our LED is on GPIO bank C, we need to enable
    // the peripheral clock to this GPIO bank in order to use it.
    rcc_periph_clock_enable(RCC_GPIOC);

    // Our test LED is connected to Port C pin 8, so let's set it as output
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);

    // Now, let's forever toggle this LED back and forth
    while (true) {
        gpio_toggle(GPIOC, GPIO8);
        delay(4800000);
    }

    return 0;
}
