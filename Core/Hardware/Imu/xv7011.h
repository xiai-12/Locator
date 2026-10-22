#ifndef _XV7011_H
#define _XV7011_H

#include "main.h"
#include "spi.h"
#include "cmsis_os2.h"

#include "Filter/filter.h"

/* XV7011BB 寄存器地址 */
#define GYRO_DSP_CTL1        0x01
#define GYRO_DSP_CTL2        0x02
#define GYRO_DSP_CTL3        0x03
#define GYRO_STS_RD          0x04
#define GYRO_SLP_IN          0x05
#define GYRO_SLP_OUT         0x06
#define GYRO_ST_BY           0x07
#define GYRO_TEMP_RD         0x08
#define GYRO_SW_RST          0x09
#define GYRO_DAT_ACCON       0x0A
#define GYRO_OUT_CTL1        0x0B
#define GYRO_AUTO_C          0x0C
#define GYRO_DSP_RES         0x0D
#define GYRO_MEM_LOAD        0x1B
#define GYRO_TSDATA_FORMAT   0x1C
#define GYRO_IF_CTL          0x1F

#define GYRO_PPR_24BIT  17920.0f
#define GYRO_PPR_16BIT  280.0f
#define GYRO_PERIOD_MS  10U

typedef struct
{
    float   gyro_bias;// bias
    float   gyro_z;// °/s
    float   yaw;
    float   yaw_nor;
}XV7011_Val;

extern XV7011_Val xv7011_val;

void XV7011_Init(void);
void XV7011_ReadData32(int32_t *data);
void XV7011_ReadData16(int16_t *data);
void XV7011_Update(void);
void XV7011_CalibrateBias(int samples);

#endif