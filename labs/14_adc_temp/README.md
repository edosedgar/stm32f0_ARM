## Задание 14 "adc_temp"

### Общие слова

В лабораторной используется модуль АЦП в связке с DMA. Источником сигнала является датчик температуры внутри микроконтроллера. Результат выводится на oled-дисплей точно так, как было описано в лабораторной про него.

### Инициализация АЦП

Первым делом включение тактирования и выбор источника. В нашем случае выбран HSI14 - внутренний генератор на 14МГц. Он называется `LL_ADC_CLOCK_ASYNC` при вызове `LL_ADC_SetClock`, так как модуль оказывается отвязан от шины APB, то есть тактируется без синхронизации с остальным микроконтроллером.

```c
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);
    /* Clock selection */
    LL_RCC_HSI14_Enable();
    LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_ASYNC);
    
```

Далее, калибровка модуля и ожидание её окончания

```c
    if (LL_ADC_IsEnabled(ADC1)) {
        LL_ADC_Disable(ADC1);
    }
    while (LL_ADC_IsEnabled(ADC1));
    LL_ADC_StartCalibration(ADC1);
    while (LL_ADC_IsCalibrationOnGoing(ADC1));
    
```

После этого, настройка самого модуля

```c
    LL_ADC_Enable(ADC1);
    LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);  // разрение 12 бит
    LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT); // дополнение нулями слева
    LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);
    LL_ADC_SetSamplingTimeCommonChannels(ADC1,
                                         LL_ADC_SAMPLINGTIME_239CYCLES_5); // опрашивать каждые 239.5 циклов
    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_TEMPSENSOR | // будем измерять температуру и
                                          LL_ADC_CHANNEL_VREFINT);    // опорное напряжение
    LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS); // постоянный опрос
    LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED); // DMA будет постоянно выгружать данные
    LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_PRESERVED); // если предыдущее значение не вычиталось, 
	                                                            // то приостановить преобразование
    
```

В конце (после настройки DMA и NVIC), остаётся включить измерение температуры и опорного напряжения и запустить преобразование.

```c
    LL_ADC_SetCommonPathInternalCh(ADC, LL_ADC_PATH_INTERNAL_TEMPSENSOR |
                                        LL_ADC_PATH_INTERNAL_VREFINT);
    LL_ADC_REG_StartConversion(ADC1);
    
```

### Настройка DMA

Настройка начинается, как обычно, с подключения тактирования. И заканчивается настройкой параметров DMA:

```c
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1,
                                    LL_DMA_DIRECTION_PERIPH_TO_MEMORY); // данные из периферии в память
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_CIRCULAR); // писать циклично (писать поверх старых значений)
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1,     // из периферии данные брать и не сдвигать
                            LL_DMA_PERIPH_NOINCREMENT); // после каждого прочтения
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1,     // а вот после каждой записи увеличивать
                            LL_DMA_MEMORY_INCREMENT);   // указатель на память
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_HALFWORD); // вычитывать по 16 бит
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_HALFWORD); // писать по 16 бит
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1,
                                   LL_DMA_PRIORITY_VERYHIGH);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 16); // одна передача состоит из 16 элементов
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)&(ADC1->DR)); // откуда брать
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)adc_buffer); // куда писать
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1); // включение прерывания по окончании передачи
```

### Использование

Вся активная часть программы находится в обработчике прерывания от DMA. В нём усредняются 8 значений для каждого канала (от датчика температуры и опорного напряжения). Из этих данных, используя калибровочные константы, выставленные на заводе, можно получить саму температуру. Эти данные: точное значение опорного напряжения при напряжении питания 3.3V, значения с датчика температуры при напряжении питания 3.3V и температурах 30C и 110C.
