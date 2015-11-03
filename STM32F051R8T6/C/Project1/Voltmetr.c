
#include <stm32f0xx_adc.h>

//=====================================================================
uint32_t ConvertADCToVoltage(uint32_t ADCData);
char* ADCVoltageToFloatString(uint32_t Var,char* ArrayVar);
uint32_t CalculateDeviation(uint32_t SourceVar,uint32_t IdealVar);
char* DeviationToString(uint32_t Deviation,char* ArrayVar);
//=====================================================================

float R1 = 75.0;
float R2 = 9.9;
float Vcc= 2.972;
char VoltageString[7];

uint32_t ConvertADCToVoltage(uint32_t ADCData)
{
	float VoltageBuffer;
	VoltageBuffer=(((R1+R2)*ADCData*Vcc)/(R2*4095))*1000;
	return VoltageBuffer;
}

char* ADCVoltageToFloatString(uint32_t Var,char* ArrayVar)
{
	ArrayVar[0]=Var/10000+0x30;
	Var-=10000*(ArrayVar[0]-0x30);
	ArrayVar[1]=Var/1000+0x30;
	Var-=1000*(ArrayVar[1]-0x30);
	ArrayVar[2]='.';
	ArrayVar[3]=Var/100+0x30;
	Var-=100*(ArrayVar[3]-0x30);
	ArrayVar[4]=Var/10+0x30;
	Var-=10*(ArrayVar[4]-0x30);
	ArrayVar[5]='V';
	ArrayVar[6]=0;
	return ArrayVar;
}

uint32_t CalculateDeviation(uint32_t SourceVar,uint32_t IdealVar)
{
	uint32_t Buffer;
	if (SourceVar>=IdealVar) {SourceVar-=IdealVar;} else {SourceVar=IdealVar-SourceVar;};
	SourceVar*=100000;
	Buffer=SourceVar/IdealVar;
	if (Buffer>=100000) {Buffer=99999;};
	return Buffer;
}

char* DeviationToString(uint32_t Deviation,char* ArrayVar)
{
	ArrayVar[0]=Deviation/10000+0x30;
	Deviation-=10000*(ArrayVar[0]-0x30);
	ArrayVar[1]=Deviation/1000+0x30;
	Deviation-=1000*(ArrayVar[1]-0x30);
	ArrayVar[2]='.';
	ArrayVar[3]=Deviation/100+0x30;
	Deviation-=100*(ArrayVar[3]-0x30);
	ArrayVar[4]=Deviation/10+0x30;
	Deviation-=10*(ArrayVar[4]-0x30);
	ArrayVar[5]='%';
	ArrayVar[6]=0;
	return ArrayVar;
}
