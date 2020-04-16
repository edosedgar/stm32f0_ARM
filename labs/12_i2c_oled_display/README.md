## Задание 12 "i2c_oled_display"

### Подключение дисплея

<p align="center">
  <img width="600" src="https://github.com/edosedgar/stm32f0_ARM/wiki/oled.jpg" alt="oled.jpg"/>
  <p align="center"> Рис. 1. Схема подключения дисплея <p align="center">

### Инициализация I2C

Инициализация протокола происходит в функции `oled_hw_config` в файле `lib/oled_driver.c`:

1. Конфигурация PB6 и PB7 на работу в режиме I2C

```c
    // SCL - GPIOB6
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6,
                       LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_6,
                             LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_1);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_6,
                        LL_GPIO_SPEED_FREQ_HIGH);

    // SDA - GPIOB7
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7,
                       LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_7,
                             LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_7, LL_GPIO_AF_1);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7,
                        LL_GPIO_SPEED_FREQ_HIGH);

```

2. Включение тактирования и периферии I2C

```c
    LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_SYSCLK);
    LL_I2C_Disable(I2C1); // Выключение для того, чтобы можно было конфигурировать I2C
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_I2C_DisableAnalogFilter(I2C1); // Выключение аналоговой фильтрации
    LL_I2C_SetDigitalFilter(I2C1, 1); // Включение минимальной цифровой фильтрации
```

3. Конфигурация модуля I2C

```c
    LL_I2C_SetTiming(I2C1, __LL_I2C_CONVERT_TIMINGS(5, 3, 3, 3, 9)); // Значения взяты из рекомендаций
    LL_I2C_DisableClockStretching(I2C1); // Сейчас не важно, если что, обратиться в документацию
    LL_I2C_SetMasterAddressingMode(I2C1, LL_I2C_ADDRESSING_MODE_7BIT); // 7 бит на адресацию ведомых
    LL_I2C_SetMode(I2C1, LL_I2C_MODE_I2C); // Режим I2C, не SMBus
    LL_I2C_Enable(I2C1);
```

### Инициализация SSD1306

Инициализация самого экрана происходит в функции `oled_config` в том же файле.

Всё, что там происходит - пересылка нужного набора байт, для того, чтобы проинициализировать нужным образом.

За пояснением можно почитать комментарии или [документацию](https://github.com/edosedgar/stm32f0_ARM/blob/master/docs/ssd1306.pdf "Документация к экрану SSD1306")

### Базовые функции

Основные функции для использования экрана:

`oled_clr` - чистит экран либо чёрным, либо белым (`color = 0x00` и `color = 0xFF`, соответственно)

`oled_update` - вывод всего, что было записано в буффер при помощи `oled_set_pix`, `oled_putc`, `oled_pic` на экран

`oled_set_pix` - выставляет цвет одного пикселя

`oled_putc` - вырисовывет букву в соответствии с шрифтом из файла font5x7.c

`oled_pic` - выводит картинку из буффера, пиксель на экране выставляется в белый цвет, если яркость пикселя превышает `thrsh` - пороговое значение

`oled_pic_dithering` - вывод каринки с применением дизеринга (см. пункт Дизеринг)

### Отрисовка черно-белых картинок

Для преобразования картинки в массив со значениями пикселей используйте
готовый скрипт (python 3.x):

```sh
python convert.py image_name
```

### Дизеринг

Для хорошего отображения картинок можно применить дизеринг: в нашем случае, если яркость пикселя лежит между нулём и порогом всегда будет рисоваться чёрный пиксель, даже если их много, что ведёт к тому, что теряются детали рисунка. Если же мы будем немного зашумлять картинку, то можно добиться того, что небольшое количество пикселей превысят пороговое значение, что даст несколько белых пикселей на тёмном фоне. В среднем, это даст, как раз, цвет близкий к тому, что был на исходной картинке. Здесь реализован [алгоритм Флойда-Стайнберга](https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering "Wikipedia: Floyd–Steinberg dithering").

### Игра «Жизнь» (Conway's Game of Life)

В качестве задания можно сделать реализацию игры "Жизнь". [Правила игры](https://ru.wikipedia.org/wiki/%D0%98%D0%B3%D1%80%D0%B0_%C2%AB%D0%96%D0%B8%D0%B7%D0%BD%D1%8C%C2%BB "Википедия: игра Жизнь").
По сути, надо каким-либо образом сгенерировать стартовое состояние, а затем реализовать правила обновления (для этого понадобится дополнительный буффер). После каждого шага копировать в буффер `gmem`, либо как картинку. 
(Дополнительно можно реализовать двойную буфферизацию для отображения: это сильно повысит производительность)
