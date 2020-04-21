#include "i2c_common.h"

//=========================------------------===================================
//=========================_STATIC_FUNCTIONS_===================================
//=========================------------------===================================
static void
I2C_Start(I2C_Device const *dev) {
	// wait while bus is BUSY
	while(LL_I2C_IsActiveFlag_BUSY(dev->bus))
		;
	
	// set common parameters for all operations:
	LL_I2C_DisableAutoEndMode(dev->bus);
	
	// send START
	LL_I2C_GenerateStartCondition(dev->bus);
}

//=========================----------------------------=========================
static void
I2C_Stop(I2C_Device const *dev) {
	// clear STOP-flag
	LL_I2C_ClearFlag_STOP(dev->bus);
	
	// send STOP
	LL_I2C_GenerateStopCondition(dev->bus);
	
	// wait for STOP-flag
	while(!LL_I2C_IsActiveFlag_STOP(dev->bus))
		;
}

//=========================----------------------------=========================
static void
I2C_SetDevAddr(I2C_Device *dev,
			   uint32_t const addr) {
	if(addr != 0xFFFFFFFF) {
		// calc devpage address
		dev->devpage = (uint8_t)(addr >> (dev->addr_len_B * 8 - 1));
		
		// mask; devpage is 3-bit max with 1-bit shift
		dev->devpage &= 0b00001110;
		
		// devaddr EEPROM: 1010___-
		// devpage:        ----xxx-
		// R/W bit:        ----___y
	}
	
	// set device address with mempage correction
	LL_I2C_SetSlaveAddr(dev->bus, dev->devaddr | dev->devpage);
}

//=========================----------------------------=========================
static void
I2C_SendData(I2C_Device const *dev,
			 uint8_t    const data) {
	// clear TXE-flag
	LL_I2C_ClearFlag_TXE(dev->bus);
	
	// send data
	LL_I2C_TransmitData8 (dev->bus, data);
	
	// wait for TXE-flag (end of transmitting sequence)
	while(!LL_I2C_IsActiveFlag_TXE(dev->bus))
		;
}

//=========================----------------------------=========================
static uint8_t
I2C_GetData(I2C_Device const *dev) {
	// wait for RXNE-flag (end of receiving sequence)
	while(!LL_I2C_IsActiveFlag_RXNE(dev->bus))
		;
	
	// read & return received data
	return LL_I2C_ReceiveData8(dev->bus);
}

//=========================----------------------------=========================
static void
I2C_SendAddr(I2C_Device const *dev,
			 uint32_t   const addr) {
	switch(dev->addr_len_B) {
		case  4: I2C_SendData(dev, (uint8_t)(addr>>24));
		case  3: I2C_SendData(dev, (uint8_t)(addr>>16));
		case  2: I2C_SendData(dev, (uint8_t)(addr>> 8));
		default: I2C_SendData(dev, (uint8_t)(addr>> 0)); break;
	}
}

//=========================------------------===================================
//================_END_OF_=_STATIC_FUNCTIONS_===================================
//=========================------------------===================================



//=========================---------------======================================
//=========================_I2C_INTERFACE_======================================
//=========================---------------======================================
#define _RESET_PARAMETERS_EXT( _RW, _FULL_LEN ) {				\
	LL_I2C_SetTransferRequest(dev->bus, (LL_I2C_REQUEST_##_RW));\
	LL_I2C_SetTransferSize(dev->bus, (_FULL_LEN));				\
}

#define _SET_PARAMETERS_EXT( _RW, _FULL_LEN, _ADDR ) {	\
	I2C_SetDevAddr(dev, (_ADDR));						\
	_RESET_PARAMETERS_EXT(_RW, (_FULL_LEN));			\
}

#define _RESET_PARAMETERS( _RW, _LEN ) { _RESET_PARAMETERS_EXT(_RW, dev->addr_len_B + (_LEN)      ); }
#define   _SET_PARAMETERS( _RW, _LEN ) {   _SET_PARAMETERS_EXT(_RW, dev->addr_len_B + (_LEN), addr); }
// It was planned to be more readable actually...

//=========================----------------------------=========================
void
I2C_DeviceInit(I2C_Device    *dev,
			   I2C_TypeDef   *bus,
			   uint8_t const  devaddr,
			   uint8_t const  addr_len_B) {
	dev->bus        = bus;
	dev->devaddr    = devaddr;
	dev->devpage    = 0;
	dev->addr_len_B = addr_len_B ? addr_len_B : 1; // guaranted >= 1
}

//=========================----------------------------=========================
void
I2C_WriteByte(I2C_Device     *dev,
			  uint32_t const  addr,
			  uint8_t  const  data) {
	_SET_PARAMETERS(WRITE, 1);

	I2C_Start(dev);
	I2C_SendAddr(dev, addr);
	
	I2C_SendData(dev, data);
	
	I2C_Stop(dev);
}

//=========================----------------------------=========================
void
I2C_WriteData(I2C_Device     *dev,
			  uint32_t const  addr,
			  uint8_t  const *data,
			  uint16_t        len) {
	_SET_PARAMETERS(WRITE, len);
	
	I2C_Start(dev);
	I2C_SendAddr(dev, addr);
	
	while(len--)
		I2C_SendData(dev, *data++);
	
	I2C_Stop(dev);
}

//=========================----------------------------=========================
uint8_t
I2C_ReadNextByte(I2C_Device *dev) {
	_SET_PARAMETERS_EXT(READ, 1, 0xFFFFFFFF); // hack: this `0xFFFFFFFF` means that `I2C_SetDevAddr` won't change `dev->devpage`

	I2C_Start(dev);
	
	uint8_t const data = I2C_GetData(dev);
	
	I2C_Stop(dev);
	return data;
}

//=========================----------------------------=========================
uint8_t
I2C_ReadByte(I2C_Device     *dev,
			 uint32_t const  addr) {
	_SET_PARAMETERS(WRITE, 0);

	I2C_Start(dev);
	I2C_SendAddr(dev, addr);
	
	_RESET_PARAMETERS_EXT(READ, 1);
	LL_I2C_GenerateStartCondition(dev->bus);

	uint8_t const data = I2C_GetData(dev);
	
	I2C_Stop(dev);
	return data;
}

//=========================----------------------------=========================
void
I2C_ReadData(I2C_Device     *dev,
			 uint32_t const  addr,
			 uint8_t        *data,
			 uint16_t        len) {
	_SET_PARAMETERS(WRITE, 0);

	I2C_Start(dev);
	I2C_SendAddr(dev, addr);
	
	_RESET_PARAMETERS_EXT(READ, len);
	LL_I2C_GenerateStartCondition(dev->bus);
	
	while(len--)
		*data++ = I2C_GetData(dev);
	
	I2C_Stop(dev);
}

//=========================----------------------------=========================
#undef _RESET_PARAMETERS_EXT
#undef   _SET_PARAMETERS_EXT
#undef _RESET_PARAMETERS
#undef   _SET_PARAMETERS

//=========================---------------======================================
//================_END_OF_=_I2C_INTERFACE_======================================
//=========================---------------======================================
