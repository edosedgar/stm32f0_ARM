#ifndef _I2C_EEPROM_H_
#define _I2C_EEPROM_H_

#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_bus.h"

#include "i2c_common.h"

//=========================---------------======================================
//=========================_EEPROM_TESTER_======================================
//=========================---------------======================================
uint32_t EE_Test(void);

//=========================----------------------------=========================



// delay after write operation, in ms
#define EE_WRITE_CYCLE_TIME_ms	( 5 +1 )	// +1 for safety

// structure for I2C-EEPROM device
typedef struct {
	I2C_Device		 i2c;		// I2C device
	GPIO_TypeDef	*WP_Port;	// port of write-protect signal
	uint32_t		 WP_Pin;	// pin  of write-protect signal
	uint32_t		 volume_B;	// volume in Bytes; for 24cXX: volume_B = XX *1024 /8;
	uint16_t		 page_B;	// page length in Bytes
} I2C_EEPROM;

//=========================--------------------=================================
//=========================_EEPROM_INITIALIZER_=================================
//=========================--------------------=================================
void
EE_Init(I2C_EEPROM     *ee,
		I2C_TypeDef    *bus,
		uint8_t  const  dev_addr,
		uint32_t const  volume_B,
		uint16_t const  page_B,
		GPIO_TypeDef   *WP_Port,
		uint32_t const  WP_Pin);

//=========================-----------------====================================
//=========================_BYTE_OPERATIONS_====================================
//=========================-----------------====================================
void
EE_WriteByte(I2C_EEPROM     *ee,
			 uint32_t const  addr,
			 uint8_t  const  data);

//=========================----------------------------=========================
uint8_t
EE_ReadByte(I2C_EEPROM     *ee,
			uint32_t const  addr);

//=========================----------------------------=========================
uint8_t
EE_ReadNextByte(I2C_EEPROM *ee);

//=========================-------------------==================================
//=========================_ARRAYS_OPERATIONS_==================================
//=========================-------------------==================================
uint8_t
EE_WriteData(I2C_EEPROM     *ee,
			 uint32_t const  addr,
			 uint8_t  const *data,
			 uint16_t const  len);

//=========================----------------------------=========================
void
EE_ReadData(I2C_EEPROM     *ee,
			uint32_t const  addr,
			uint8_t        *data,
			uint16_t const  len);

//=========================----------------------------=========================

#endif // _I2C_EEPROM_H_
