#ifndef _I2C_COMMON_H_
#define _I2C_COMMON_H_

#include "stm32f0xx_ll_i2c.h"

// max bytes in one packet (between START-STOP) except devaddr/addr
#define I2C_MAX_PAGE_LEN_B (128) // ATTENTION! 128 is embedded limit for STM32F0!

// structure for common I2C device
typedef struct {
	I2C_TypeDef	*bus;			// used I2C bus
	uint8_t		 devaddr;		// device (hardware) address
	uint8_t		 devpage;		// additional address within devaddr (default 0)
								//   devpage usually used for EEPROM addressing
	uint8_t		 addr_len_B;	// real address lenght in a round Bytes (actually 1 or 2)
} I2C_Device;

//=========================-----------------------==============================
//=========================_I2C_DEVICE_INITIALIZER_=============================
//=========================-----------------------==============================
void
I2C_DeviceInit(I2C_Device    *dev,
			   I2C_TypeDef   *bus,
			   uint8_t const  devaddr,
			   uint8_t const  addr_len_B);

//=========================-----------------====================================
//=========================_BYTE_OPERATIONS_====================================
//=========================-----------------====================================
void
I2C_WriteByte(I2C_Device     *dev,
			  uint32_t const  addr,
			  uint8_t  const  data);

//=========================----------------------------=========================
uint8_t
I2C_ReadByte(I2C_Device     *dev,
			 uint32_t const  addr);

//=========================----------------------------=========================
uint8_t
I2C_ReadNextByte(I2C_Device *dev);

//=========================-------------------==================================
//=========================_ARRAYS_OPERATIONS_==================================
//=========================-------------------==================================
void
I2C_WriteData(I2C_Device     *dev,
			  uint32_t const  addr,
			  uint8_t  const *data,
			  uint16_t        len);

//=========================----------------------------=========================
void
I2C_ReadData(I2C_Device     *dev,
			 uint32_t const  addr,
			 uint8_t        *data,
			 uint16_t        len);

//=========================----------------------------=========================


#endif // _I2C_COMMON_H_
