## Задание 09 "usart_terminal"

В этом задании вам предлагается сделать несложное исполнительное устройство.
Данное устройство должно принимать команду с параметрами с компьютера и
выполнять определенное действие.

### Инициализация периферии

Первым делом инициализируем порты ввода-вывода: настраиваем выходы для светодиодов (PC8 и PC9) и вход кнопки (PA0 - по умолчанию на вход):

```c
LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
    
```

Иницализируем USART: 

1. Включим PA9 и PA10 на альтернативные функции (TX и RX, соответственно):

```c
LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
//USART1_TX
LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_1);
LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_HIGH);
//USART1_RX
LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_1);
LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_HIGH);
```

2. Включение тактирования USART:

```c
LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
```

3. Конфигурация канала:

```c
LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX_RX); // Дуплексный режим
LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);                // Без битов чётности
LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);            // В пакете 8 бит
LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);         // Один стоп-бит
LL_USART_SetTransferBitOrder(USART1, LL_USART_BITORDER_LSBFIRST);// Порядок бит: менее значащие впереди
LL_USART_SetBaudRate(USART1, SystemCoreClock,
                     LL_USART_OVERSAMPLING_16, 115200);          // Скорость передачи 115200 бод
LL_USART_EnableIT_IDLE(USART1);                                  // Прерывания по приходу байта и концу передачи
LL_USART_EnableIT_RXNE(USART1);
```

4. Включение и ожидание готовности:

```c
LL_USART_Enable(USART1);
while (!(LL_USART_IsActiveFlag_TEACK(USART1) &&
         LL_USART_IsActiveFlag_REACK(USART1)));
```

5. Разрешение прерываний в NVIC:

```c
NVIC_SetPriority(USART1_IRQn, 0);
NVIC_EnableIRQ(USART1_IRQn);
```

### Обработчик прерывания

В обработчике прерывания смотрим какое прерывание: пришёл байт или конец передачи

1. Новый байт:

```c
if (LL_USART_IsActiveFlag_RXNE(USART1)) {
    if (pos == 0) {
        uart_req.cmd = LL_USART_ReceiveData8(USART1);
    } else {
        uart_req.params[pos - 1] = LL_USART_ReceiveData8(USART1);
    }
    pos++;
}
```

Здесь __не надо__ сбрасывать флаг RXNE. Он сбрасывается сам при вызове `LL_USART_ReceiveData8(USART1)`

В этом случае заполняем глобальную структуру uart_req:

```c
typedef struct {
    uint8_t cmd;
    uint8_t params[10];
    uint8_t active;
} uart_req_t;
static uart_req_t uart_req;
```

2. Конец передачи: 

```c
if (LL_USART_IsActiveFlag_IDLE(USART1)) {
    pos = 0;
    uart_req.active = 1;
    LL_USART_ClearFlag_IDLE(USART1);
}

```

В этом случае выставляем флаг готовности структыры к обработке команды

### Выполнение команды

В функции выполнения команды:

1. Проверяем готовность:

```c
if (!uart_req.active)
    return;
```

В случае, когда команда не готова, просто выходим. Функция снова вызовется из основного цикла

2. Выбор нужной команды и её выполнение

```c
switch (uart_req.cmd) {
case '8': { // Зажечь или погасить диод на PC8
    if (uart_req.params[1] == '1')
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_8);
    else
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_8);
    is_ok = 1;
    break;
}
case '9': { // Зажечь или погасить диод на PC9
    if (uart_req.params[1] == '1')
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_9);
    else
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_9);
    is_ok = 1;
    break;
}
case '0': { // Нажата ли кнопка?
    is_ok = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
    break;
}
default:    // Неизвестная команда
    is_ok = 0;
    break;
}
```

3. Передача кода возврата:

```c
while (!LL_USART_IsActiveFlag_TXE(USART1)); // Ждём, пока освободится канал не передачу
LL_USART_TransmitData8(USART1, is_ok + '0');
```

### Задания

Задания из данного раздела предоставлены [здесь](https://github.com/edosedgar/stm32f0_ARM/blob/master/labs/09_usart_terminal/notebook/09_usart_terminal.ipynb). Ноутбук необходимо скачать.
