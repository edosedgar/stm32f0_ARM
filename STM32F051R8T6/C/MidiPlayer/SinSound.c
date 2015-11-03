
typedef struct
{
	uint8_t DACData;
	uint8_t State;
	uint32_t Frequency;
	uint16_t StepTact;
	uint16_t StepStopTact;
	uint16_t Div;
} TSoundChannel;

TSoundChannel SoundChannel [3];
uint8_t NumberChannel;

//**************************************************************************************

void OutFrequence(uint16_t Frequency, uint8_t NumberSoundChannel, uint16_t Duration, uint16_t StopDuration);
void ClearSoundBuffers(void);
void SoundRender(uint8_t NumberSoundChannel);
void DelayTimeRender(void);

//**************************************************************************************

void ClearSoundBuffers(void)
{
	uint8_t i;
	for (i=0;i<3;i++)
	{
		SoundChannel[i].DACData=0;
		SoundChannel[i].State=0;
		SoundChannel[i].Frequency=0;
		SoundChannel[i].StepTact=0;
		SoundChannel[i].StepStopTact=0;
		SoundChannel[i].Div=0;
	}
	NumberChannel=0;
}

//**************************************************************************************

void OutFrequence(uint16_t Frequency, uint8_t NumberSoundChannel, uint16_t Duration, uint16_t StopDuration)
{
	uint16_t i,j=0;
	while ((SoundChannel[NumberSoundChannel].State==1)||(SoundChannel[NumberSoundChannel].StepStopTact!=0)) {};
	SoundChannel[NumberSoundChannel].DACData=0;
	SoundChannel[NumberSoundChannel].Frequency=Frequency;
	SoundChannel[NumberSoundChannel].StepTact=Duration;
	SoundChannel[NumberSoundChannel].StepStopTact=StopDuration;
	SoundChannel[NumberSoundChannel].Div=0;
	SoundChannel[NumberSoundChannel].State=1;
	//for (i=0;i<3;i++) {if (SoundChannel[i].State==1) {j++;};};
	//NumberChannel=j;
	switch (NumberSoundChannel)
	{
		case 0: TIM2->ARR=(4800000/SoundChannel[NumberSoundChannel].Frequency); break;
		case 1: TIM3->ARR=(4800000/SoundChannel[NumberSoundChannel].Frequency); break;
		case 2: TIM6->ARR=(4800000/SoundChannel[NumberSoundChannel].Frequency); break;
	}
}

//**************************************************************************************

void SoundRender(uint8_t NumberSoundChannel)
{
	if (SoundChannel[NumberSoundChannel].State==1)
	{
	SoundChannel[NumberSoundChannel].DACData=0xFF-SoundChannel[NumberSoundChannel].DACData;
	DAC->DHR12R1=(SoundChannel[0].DACData+SoundChannel[1].DACData+SoundChannel[2].DACData);
	};
}

//**************************************************************************************

void DelayTimeRender(void)
{
	uint8_t i;
	for (i=0;i<3;i++)
	{
		if (SoundChannel[i].State==1)
		{
			if (SoundChannel[i].StepTact!=0) {SoundChannel[i].StepTact--;} else
			{
				SoundChannel[i].State=0;NumberChannel--;SoundChannel[i].DACData=0;
				//switch (i)
				//{
					//case 0: TIM2->ARR=0; break;
					//case 1: TIM3->ARR=0; break;
					//case 2: TIM6->ARR=0; break;
				//}
			};
		} else {if (SoundChannel[i].StepStopTact!=0) {SoundChannel[i].StepStopTact--;};};
	};
}

