#include "I3G4250D_Driver.h"
#include "spi.h"

#define FILTER_SIZE                          10u
#define CALIBRATION_BUFFER_LENGTH            2000u
#define GYROSCOPE_SENSITIVITY    			 0.07f


typedef enum
{
GYROSCOPE_DATA_NOT_READY,
GYROSCOPE_DATA_READY,
}currentFlagStateType;

static currentFlagStateType currentFlagState=GYROSCOPE_DATA_READY;

typedef enum
{
COLLECT_SAMPLES,
PROCESS_SAMPLES,
CALIBRATED,
}CalibrationType;

static CalibrationType currentCallibrationState=COLLECT_SAMPLES;

typedef enum
{
GYROSCOPE_FIRST,
GYROSCOPE_FINAL
}StateType;
static StateType currentGyroscopeState=GYROSCOPE_FIRST;


static float NewAngleRate_x=0;
static float NewAngleRate_y=0;
static float NewAngleRate_z=0;

static int32_t offset_x=0;
static int32_t offset_y=0;
static int32_t offset_z=0;

static float Noise_X = 0;
static float Noise_Y = 0;
static float Noise_Z = 0;

static float Angle_X = 0;
static float Angle_Y = 0;
static float Angle_Z = 0;

static float LastNewAngleRate_X = 0;
static float LastNewAngleRate_Y = 0;
static float LastNewAngleRate_Z = 0;

static int32_t TempNoise_X = 0;
static int32_t TempNoise_Y = 0;
static int32_t TempNoise_Z = 0;

volatile static uint32_t caliCounter = 0;

static int16_t calibrationBuffer_X[CALIBRATION_BUFFER_LENGTH];
static int16_t calibrationBuffer_Y[CALIBRATION_BUFFER_LENGTH];
static int16_t calibrationBuffer_Z[CALIBRATION_BUFFER_LENGTH];

static uint8_t spiTxBuf[2];
static uint8_t spiRxBuf[7];

void Gyroscope_Init(void)
{

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	HAL_Delay(1);
	spiTxBuf[0]=0x20;
	spiTxBuf[1]=0xff;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,2,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
	HAL_Delay(10);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	HAL_Delay(1);
	spiTxBuf[0]=0x21;
	spiTxBuf[1]=0x00;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,2,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
	HAL_Delay(10);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	HAL_Delay(1);
	spiTxBuf[0]=0x22;
	spiTxBuf[1]=0x00;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,2,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
	HAL_Delay(10);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	HAL_Delay(1);
	spiTxBuf[0]=0x23;
	spiTxBuf[1]=0x20;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,2,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
	HAL_Delay(10);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	HAL_Delay(1);
	spiTxBuf[0]=0x24;
	spiTxBuf[1]=0x10;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,2,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
	HAL_Delay(10);

}
static void SPI_READ(void)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	spiTxBuf[0]=0x28|0x80;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,1,50);
	HAL_SPI_Receive(&hspi5,&spiRxBuf[1],1,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	spiTxBuf[0]=0x29|0x80;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,1,50);
	HAL_SPI_Receive(&hspi5,&spiRxBuf[2],1,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	spiTxBuf[0]=0x2a|0x80;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,1,50);
	HAL_SPI_Receive(&hspi5,&spiRxBuf[3],1,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	spiTxBuf[0]=0x2b|0x80;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,1,50);
	HAL_SPI_Receive(&hspi5,&spiRxBuf[4],1,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	spiTxBuf[0]=0x2c|0x80;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,1,50);
	HAL_SPI_Receive(&hspi5,&spiRxBuf[5],1,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
	spiTxBuf[0]=0x2d|0x80;
	HAL_SPI_Transmit(&hspi5,spiTxBuf,1,50);
	HAL_SPI_Receive(&hspi5,&spiRxBuf[6],1,50);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);

}

static void Gyroscope_Calibration(void)
{
	volatile int16_t Buff_x=0;
	volatile int16_t Buff_y=0;
	volatile int16_t Buff_z=0;

	float difftime=0;

	int16_t avarageFilter_X[FILTER_SIZE] = {0};
	int16_t avarageFilter_Y[FILTER_SIZE] = {0};
	int16_t avarageFilter_Z[FILTER_SIZE] = {0};

	uint32_t filterRate = 0;
	int32_t tempSum_X = 0;
	int32_t tempSum_Y = 0;
	int32_t tempSum_Z = 0;

	Buff_x=(spiRxBuf[2]<<8)|spiRxBuf[1];
	Buff_y=(spiRxBuf[4]<<8)|spiRxBuf[3];
	Buff_z=(spiRxBuf[6]<<8)|spiRxBuf[5];

	if(currentCallibrationState==CALIBRATED)
	{
		NewAngleRate_x=(float) (Buff_x - (offset_x))*GYROSCOPE_SENSITIVITY;
		NewAngleRate_y=(float) (Buff_y - (offset_y))*GYROSCOPE_SENSITIVITY;
		NewAngleRate_z=(float) (Buff_z - (offset_z))*GYROSCOPE_SENSITIVITY;
		difftime=0.003f;

		if((NewAngleRate_x>Noise_X)||(NewAngleRate_x<-Noise_X))
		{
			Angle_X+=((NewAngleRate_x+LastNewAngleRate_X)*difftime)/(2.0f);
			LastNewAngleRate_X=NewAngleRate_x;
		}
		if((NewAngleRate_y>Noise_Y)||(NewAngleRate_y<-Noise_Y))
		{
			Angle_Y+=((NewAngleRate_y+LastNewAngleRate_Y)*difftime)/(2.0f);
			LastNewAngleRate_Y=NewAngleRate_y;
		}
		if((NewAngleRate_z>Noise_Z)||(NewAngleRate_z<-Noise_Z))
		{
			Angle_Z+=((NewAngleRate_z+LastNewAngleRate_Z)*difftime)/(2.0f);
			LastNewAngleRate_Z=NewAngleRate_z;
		}
	}
	else
	{
		switch(currentCallibrationState)
		{
			case(COLLECT_SAMPLES):
				calibrationBuffer_X[caliCounter]=Buff_x;
				calibrationBuffer_Y[caliCounter]=Buff_y;
				calibrationBuffer_Z[caliCounter]=Buff_z;
				caliCounter++;

				if(caliCounter>=CALIBRATION_BUFFER_LENGTH)
				{
					caliCounter=0;
					currentCallibrationState=PROCESS_SAMPLES;
				}
				break;
			case(PROCESS_SAMPLES):
				for(uint32_t i=0; i<CALIBRATION_BUFFER_LENGTH;i++)
				{
					tempSum_X=tempSum_X-avarageFilter_X[filterRate]+calibrationBuffer_X[i];
					tempSum_Y=tempSum_Y-avarageFilter_Y[filterRate]+calibrationBuffer_Y[i];
					tempSum_Z=tempSum_Z-avarageFilter_Z[filterRate]+calibrationBuffer_Z[i];

					avarageFilter_X[filterRate]=calibrationBuffer_X[i];
					avarageFilter_Y[filterRate]=calibrationBuffer_Y[i];
					avarageFilter_Z[filterRate]=calibrationBuffer_Z[i];

					offset_x=tempSum_X/(int32_t)FILTER_SIZE;
					offset_y=tempSum_Y/(int32_t)FILTER_SIZE;
					offset_z=tempSum_Z/(int32_t)FILTER_SIZE;

					filterRate++;

					if(filterRate>=FILTER_SIZE)
					{
						filterRate=0;
					}
				}
				for(uint32_t i=0;i<CALIBRATION_BUFFER_LENGTH;i++)
				{
					if(((int32_t)calibrationBuffer_X[i]-offset_x)>TempNoise_X)
					{
						TempNoise_X=(int32_t)calibrationBuffer_X[i]-offset_x;
					}
					else if(((int32_t)calibrationBuffer_X[i]-offset_x)<-TempNoise_X)
					{
						TempNoise_X=-((int32_t)calibrationBuffer_X[i]-offset_x);
					}
					if(((int32_t)calibrationBuffer_Y[i]-offset_y)>TempNoise_Y)
					{
						TempNoise_Y=(int32_t)calibrationBuffer_Y[i]-offset_y;
					}
					else if(((int32_t)calibrationBuffer_Y[i]-offset_y)<-TempNoise_Y)
					{
						TempNoise_Y=-((int32_t)calibrationBuffer_Y[i]-offset_y);
					}
					if(((int32_t)calibrationBuffer_Z[i]-offset_z)>TempNoise_Z)
					{
						TempNoise_Z=(int32_t)calibrationBuffer_Z[i]-offset_z;
					}
					else if(((int32_t)calibrationBuffer_Z[i]-offset_z)<-TempNoise_Z)
					{
						TempNoise_Z=-((int32_t)calibrationBuffer_Z[i]-offset_z);
					}
				}

				Noise_X=(float)TempNoise_X*GYROSCOPE_SENSITIVITY;
				Noise_Y=(float)TempNoise_Y*GYROSCOPE_SENSITIVITY;
				Noise_Z=(float)TempNoise_Z*GYROSCOPE_SENSITIVITY;
				currentCallibrationState=CALIBRATED;
				break;
			case(CALIBRATED):
			break;

			default:
			break;
		}

	}
	currentGyroscopeState=GYROSCOPE_FIRST;
	currentFlagState=GYROSCOPE_DATA_READY;

}

void Gyroscope_loop(void)
{
	switch(currentGyroscopeState)
	{
		case(GYROSCOPE_FIRST):
			if(currentFlagState==GYROSCOPE_DATA_READY)
			{
				SPI_READ();
				currentGyroscopeState=GYROSCOPE_FINAL;
				currentFlagState=GYROSCOPE_DATA_NOT_READY;
			}
			break;
		case(GYROSCOPE_FINAL):
			Gyroscope_Calibration();
			break;
		default:
			break;
	}
}

void Gyroscope_SetDataReady(void) {
    currentFlagState = GYROSCOPE_DATA_READY;
}

float Gyroscope_GetAngleX(void) {
    return Angle_X;
}

float Gyroscope_GetAngleY(void) {
    return Angle_Y;
}

float Gyroscope_GetAngleZ(void) {
    return Angle_Z;
}
volatile uint8_t dataReadyFlag = 0;


void Gyroscope_ClearDataReady() {
    dataReadyFlag = 0;
}

uint8_t Gyroscope_IsDataReady() {
    return dataReadyFlag;
}

void Gyroscope_InterruptHandler() {
    Gyroscope_SetDataReady();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_2) {
        Gyroscope_SetDataReady();
    }
}

