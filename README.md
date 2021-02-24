Introduction to embedded programming on STM32
=============================================

## General info

STM32 is a modern family of MCUs produced by STMicroelectronics that is widely
used in research, consumer and industrial electronics. STM32 microcontrollers
are quite powerful and cheap, but impose initial barriers to get started
quickly.

The purpose of the course is to overcome these barries and steadily get into
the topic. This course will help to develop basic understanding of Cortex-M0
core and STM32F0 peripherals.

To get to the content of lectures and supplementary materials for the course,
refer to [Wiki page](https://github.com/edosedgar/stm32f0_ARM/wiki).

If you need just a template project, refer to
[Blank project](https://github.com/edosedgar/stm32f0_ARM/tree/master/labs/01_blank).

## Syllabus

1. **Toolchain for programming microcontrollers.**
ARM architecture cross-compiler. On-board debugger. Boot modes. Boot-loader.
Configuration bytes.

2. **CPU core. Memory map.**
Registers. Microcontroller system architecture. Memory organization:
static RAM (SRAM), peripheral memory, system memory.Code execution:
interrupt table, stack, heap.

3. **Reset and clock control (RCC).**
Clocking signal. Rising edge, falling edge. Clock sources: HSI, HSE, PLL.
Clocking map of stm32f0. Memory latency. Reset system.

4. **General-purpose input/output (GPIO).**
Structure of an I/O port bit. Input floating. Input pull-up. Input push-down.
Shmitt trigger. Contact debouncing. Analog input. I/O pin alternate functions.
7-segment indicator. Dynamic indication.

5. **Nested vectored interrupt controller (NVIC). System and peripheral interrupts.**
Event-handling policies: busy-polling, interrupts. Interrupt handling: stacking,
unstacking, nested interrupts, tail-chaining, late-arriving. Interrupt priority
and status. Interrupt table.

6. **Extended interrupts and events controller (EXTI). System timer.**
Internal structure of EXTI module. EXTI configuration. Internal structure of
a system timer: counter, reload value, control and status registers. SysTick
configuration. Encoder.

7. **General-purpose timers.**
Counter structure: counter register, prescaler register, auto-reload register.
Counter timing diagram. Counter modes: upcounting, downcounting, up/down counting.
Timer configuration. Input capture.

8. **Encoder support. Pulse-width modulation (PWM).**
Incremental encoder operation. Encoder interface mode of a timer. Output compare mode
of a timer. Pulse-width modulation, duty cycle. Edge-aligned PWM, center-aligned PWM.
Piezo-buzzer and PWM-based sound generation.

9. **Universal synchronous and asynchronous receiver-transmitter (USART).**
Communication directions: simplex, half-duplex, full-duplex. USART signaling scheme:
start bit, data bits, parity bit, stop bit. USART configuration: baudrate, endianness,
oversampling. USART interrupts.

10. **Direct Memory Access (DMA).**
DMA transactions. Internal structure of DMA module. DMA transfer parameters: peripheral
and memory adresses, circular mode. DMA interrupts. 

11. **Real-time clock (RTC).**
Internal structure of RTC module. Time representation, the binary-coded
decimal (BCD) format. Alarm clock implementation: masking, interrupts. 

12. **Inter-Integrated Circuit bus (I2C).**
I2C signaling: data pin (SDA), clock pin (SCL), baud rates, addressing, timings.
I2C module internal structure. Connection of EEPROM and OLED-display over I2C.

13. **Serial Peripheral Interface (SPI).**
SPI signaling: MISO and MOSI pins, slave select pin, clock polarity. Internal
structure of SPI module. SPI communication modes: simplex, half-duplex,
full-duplex, multi-slave, multi-master. Recieve and transmit queues
(RXFIFO, TXFIFO). SPI interrupts. Shift register.

14. **Analog-to-digital (ADC) and digital-to-analog (DAC) converters.**
Successive approximation ADC structure. Internal structure of ADC and DAC modules.
ADC operation: clock source, calibration, sampling time. Data alignment. ADC
operation modes: single conversion, continious conversion, discontinious mode.
ADC operation diagrams. DMA usage. Temperature sensor and referece voltage.

15. **Low-power mode.**
Sleep mode, stop mode, standby mode. Clocking domains.

16. **Introduction to Operating Systems.**
Finite State Machine (FSM). Minimal OS. Real-Time Operating System (RTOS). FreeRTOS:
tasks, scheduler, priorities. Inter-Process Communication (IPC): mutex, semaphore, notificator.
Critical section. RWM-operation. Deadlock. Message queue. 


17. **STM32 Microcontroller Ecosystem.**
ST-Link. Microcontroller evaluation boards: STM32F0-Discovery, minimal STM32F0 board,
STM32F1 Blue Pill, STM32F4-Discovery, STM32G0 board. Printed circuit board production software.
Libraries overview. 


## Contacts

Should you have any questions, feel free to contact via [e-mail](mailto:edos.edgar@gmail.com)
with subject "STM32 course".
