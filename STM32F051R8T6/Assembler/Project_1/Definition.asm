;===========Macro===========
	MACRO
	WriteIn $Register,$Const
	LDR R0,=$Register
	LDR R1,=$Const
	STR R1,[R0]
	MEND
	
;===========================
GPIOA	EQU 0x48000000
GPIOB   EQU 0x48000400
GPIOC 	EQU 0x48000800
RCC     EQU 0x40021000
TIM1    EQU 0x40012C00
SPI1    EQU 0x40013000
SPI2    EQU 0x40003800
DAC     EQU 0x40007400
TIM6    EQU 0x40001000
ISER    EQU 0xE000E100
ISPR    EQU 0xE000E200
ICPR    EQU 0xE000E280
;===========================
MODER  			EQU 0x00	; GPIO port mode register
OTYPER			EQU 0x04    ; GPIO port output type register
OSPEEDR			EQU 0x08    ; GPIO port output speed register
PUPDR  			EQU 0x0C    ; GPIO port pull-up/pull-down register
IDR 			EQU 0x10    ; GPIO port input data register
ODR  			EQU 0x14    ; GPIO port output data register
BSRR 			EQU 0x18    ; GPIO port bit set/reset register
LCKR    		EQU 0x1C	; GPIO port configuration lock register
AFRL    		EQU 0x20	; GPIO alternate function low register
AFRH    		EQU 0x24	; GPIO alternate function high register 
BRR				EQU 0x28	; Port bit reset register
;---------------------------;
RCC_CR      	EQU 0x00	; Clock control register	
RCC_CFGR		EQU	0x04	; Clock configuration register
RCC_CIR			EQU 0x08	; Clock interrupt register
RCC_APB2RSTR	EQU 0x0C	; APB peripheral reset register 2
RCC_APB1RSTR	EQU 0x10	; APB peripheral reset register 1
RCC_AHBENR		EQU 0x14	; AHB peripheral clock enable register
RCC_APB2ENR		EQU 0x18	; APB peripheral clock enable register 2
RCC_APB1ENR		EQU 0x1C	; APB peripheral clock enable register 1 
RCC_BDCR		EQU 0x20	; Backup domain control register
RCC_CSR			EQU 0x24	; Control/status register
RCC_AHBRSTR		EQU 0x28	; AHB peripheral reset register
RCC_CFGR2		EQU 0x2C	; Clock configuration register 2
RCC_CFGR3		EQU 0x30	; Clock configuration register 3
RCC_CR2			EQU 0x34	; Clock control register 2
;---------------------------;
TIM6_CR1		EQU 0x00
TIM6_CR2		EQU 0x04
TIM6_DIER		EQU 0x0C 
TIM6_SR			EQU 0x10 
TIM6_EGR		EQU 0x14
TIM6_CNT		EQU 0x24
TIM6_PSC		EQU 0x28
TIM6_ARR		EQU 0x2C
;===========================
Pin0 	EQU 0x0001
Pin1 	EQU 0x0002
Pin2 	EQU 0x0004
Pin3 	EQU 0x0008
Pin4 	EQU 0x0010
Pin5 	EQU 0x0020
Pin6 	EQU 0x0040
Pin7 	EQU 0x0080
Pin8 	EQU 0x0100
Pin9 	EQU 0x0200
Pin10 	EQU 0x0400
Pin11 	EQU 0x0800
Pin12 	EQU 0x1000
Pin13 	EQU 0x2000
Pin14 	EQU 0x4000
Pin15   EQU 0x8000
;===========================
 END
 