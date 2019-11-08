#include "i2c_eeprom.h"

//	EEPROM usage example:

/*	single 24c02 */
	#define EE_VOLUME	( 2 *1024 /8 )	// 24c02 = 2kbits
	#define EE_PAGE		( 8 )			// 24c02 = 8-Bytes page

/*	double 24c02 */
//	#define EE_VOLUME	( 4 *1024 /8 )	// you can combine some 24c02/04/08 or 24c512/M01/M02
										// (but not 24c00/01 and 24c16/32/64/128)
										// into one bigger EEPROM with using A2 A1 A0 device address lines
										// as byte-address
//										// e.g. 2*24c02 == 24c04, 4*24c512 == 24cM02
//	#define EE_PAGE		( 8 )			// but page size still the same!

/*	2x 24c512 or 24cM01 */
//	#define EE_VOLUME	( 1024 *1024 /8 )
//	#define EE_PAGE		( 128 )			// 24cM01 has 256-Bytes page, but...
										// ATTENTION! 128 is limit for STM32F0!

uint32_t
EE_Test(void) {
	// errors counter
	uint32_t err = 0;
	
	// define & init eeprom
	I2C_EEPROM ee_24cXX;
	EE_Init(&ee_24cXX, I2C1, 0b10100000, EE_VOLUME, EE_PAGE, GPIOB, LL_GPIO_PIN_5);

	//=========================================
	// write and check byte-by-byte (partially)
	for(uint16_t i = 0, data = 0xc3; i < EE_VOLUME; ++i, data ^= i) {
		EE_WriteByte(&ee_24cXX, i, data);
		err += EE_ReadByte(&ee_24cXX, i) != data;
	}

	//===========================================
	// write and check page-by-page (full EEPROM)
	uint8_t dst[EE_PAGE];
	uint8_t src[EE_PAGE];
	
	// fill src with random values
	for(uint8_t i = 0, data = 0xc4; i < EE_PAGE; ++i, data ^= i)
		src[i] = data;
	
	// page-by-page write & read & check
	for(uint16_t addr = 0; addr < EE_VOLUME; addr += EE_PAGE) {
		err += EE_WriteData(&ee_24cXX, addr, src, EE_PAGE);
		EE_ReadData(&ee_24cXX, addr, dst, EE_PAGE);

		// page check
		for(uint16_t i = 0; i < EE_PAGE; ++i)
			err += src[i] != dst[i];
	}

	//=======================================================
	// check (partially) prevoius data with `EE_ReadNextByte`
	for(uint16_t i = 0; i < EE_PAGE; ++i)
		dst[i] = 0xFF;
	
	dst[0] = EE_ReadByte(&ee_24cXX, 0);      // read first byte of EEPROM
	for(uint16_t i = 1; i < EE_PAGE; ++i)     // read all other bytes
		dst[i] = EE_ReadNextByte (&ee_24cXX);
	
	// check
	for(uint16_t i = 0; i < EE_PAGE; ++i)
		err += src[i] != dst[i];

	return err;
}


//=========================------------------===================================
//=========================_STATIC_FUNCTIONS_===================================
//=========================------------------===================================
inline static void
_ee_delay_ms(uint16_t ms) {
	while(ms--)
		for(uint32_t i = 4400; i; --i) // fitting for 1ms
			__asm__("nop");
}

//=========================------------------===================================
//================_END_OF_=_STATIC_FUNCTIONS_===================================
//=========================------------------===================================



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
		uint32_t const  WP_Pin) {
	uint8_t const addr_len_B = volume_B > 2048 ? 2 : 1; // 1 or 2 Bytes:
							// 24c16 (2kB) and less has 1B addressing;
							// 24c32 (4kB) and more has 2B addressing;
	I2C_DeviceInit(&(ee->i2c), bus, dev_addr, addr_len_B);
	ee->WP_Port  = WP_Port;
	ee->WP_Pin   = WP_Pin;
	ee->volume_B = volume_B;
	ee->page_B   = (page_B > I2C_MAX_PAGE_LEN_B) ? I2C_MAX_PAGE_LEN_B : page_B;
	
	//====================
	// WP pin init
	//====================
	
	// clock enable
	uint32_t periphs;
	#define _RCC_PORT_CHECK( _PORTNAME ) { if(WP_Port == (GPIO##_PORTNAME)) periphs = (LL_AHB1_GRP1_PERIPH_GPIO##_PORTNAME); }
	_RCC_PORT_CHECK(A);
	_RCC_PORT_CHECK(B);
	_RCC_PORT_CHECK(C);
	_RCC_PORT_CHECK(D);
	_RCC_PORT_CHECK(F);
	#undef _RCC_PORT_CHECK
	LL_AHB1_GRP1_EnableClock(periphs);
	
	//LL_GPIO_SetPinMode(ee->WP_Port, ee->WP_Pin, LL_GPIO_MODE_OUTPUT);
	
	/*GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin        = ee->WP_Pin;
		GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
		GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
	LL_GPIO_Init(ee->WP_Port, &GPIO_InitStruct);*/
	
	LL_GPIO_SetPinMode(ee->WP_Port, ee->WP_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(ee->WP_Port, ee->WP_Pin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(ee->WP_Port, ee->WP_Pin, LL_GPIO_SPEED_FREQ_HIGH);
	LL_GPIO_SetPinPull(ee->WP_Port, ee->WP_Pin, LL_GPIO_PULL_NO);
	
	
	// turn on Write Protection
	LL_GPIO_SetOutputPin(ee->WP_Port, ee->WP_Pin);
}

//=========================-----------------====================================
//=========================_BYTE_OPERATIONS_====================================
//=========================-----------------====================================
void
EE_WriteByte(I2C_EEPROM     *ee,
			 uint32_t const  addr,
			 uint8_t  const  data ) {
	LL_GPIO_ResetOutputPin(ee->WP_Port, ee->WP_Pin);
	I2C_WriteByte(&(ee->i2c), addr, data);
	LL_GPIO_SetOutputPin(ee->WP_Port, ee->WP_Pin);
	
	_ee_delay_ms(EE_WRITE_CYCLE_TIME_ms);
}

//=========================----------------------------=========================
inline uint8_t
EE_ReadByte(I2C_EEPROM     *ee,
			uint32_t const  addr) {
	return I2C_ReadByte(&(ee->i2c), addr);
}

//=========================----------------------------=========================
inline uint8_t
EE_ReadNextByte(I2C_EEPROM *ee) {
	return I2C_ReadNextByte(&(ee->i2c));
}

//=========================-------------------==================================
//=========================_ARRAYS_OPERATIONS_==================================
//=========================-------------------==================================
uint8_t
EE_WriteData(I2C_EEPROM     *ee,
			 uint32_t const  addr,
			 uint8_t  const *data,
			 uint16_t const  len) {
	uint16_t const start_page	= (addr    )%(ee->page_B);
	uint16_t const end_page		= (addr+len)%(ee->page_B);
	if (start_page != end_page)
		return 1;

	LL_GPIO_ResetOutputPin(ee->WP_Port, ee->WP_Pin);
	I2C_WriteData(&(ee->i2c), addr, data, len);
	LL_GPIO_SetOutputPin(ee->WP_Port, ee->WP_Pin);
	_ee_delay_ms(EE_WRITE_CYCLE_TIME_ms);
	return 0;
}

//=========================----------------------------=========================
inline void
EE_ReadData(I2C_EEPROM     *ee,
			uint32_t const  addr,
			uint8_t        *data,
			uint16_t const  len) {
	I2C_ReadData(&(ee->i2c), addr, data, len);
}

//=========================----------------------------=========================
