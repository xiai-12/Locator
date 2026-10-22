#include "xv7011.h"

#define CS_XV7011_ON()       HAL_GPIO_WritePin(XV7011_CS_GPIO_Port, XV7011_CS_Pin, GPIO_PIN_RESET)
#define CS_XV7011_OFF()      HAL_GPIO_WritePin(XV7011_CS_GPIO_Port, XV7011_CS_Pin, GPIO_PIN_SET)

XV7011_Val xv7011_val={0};
Filter_MA_t xv7011_filter={0};

//////////////////////////////////////////////////////////////////////////////////////////
static uint8_t SPI1_RW_Byte(uint8_t byte);
static void XV7011_WriteReg(uint8_t reg, uint8_t data);
static uint8_t XV7011_ReadReg(uint8_t reg);
static void XV7011_ReadBytes(uint8_t reg, uint8_t length, uint8_t *data);

#define GYRO_DEADZONE_DPS    0.2f   // 死区
#define GYRO_GAIN_TRANS_DPS  0.3f   // 过渡带宽度

static uint8_t SPI1_RW_Byte(uint8_t byte)
{
    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &rx, 1, 10);
    return rx;
}

static void XV7011_WriteReg(uint8_t reg, uint8_t data)
{
    CS_XV7011_ON();
    SPI1_RW_Byte(reg);
    SPI1_RW_Byte(data);
    CS_XV7011_OFF();
}

static uint8_t XV7011_ReadReg(uint8_t reg)
{
    uint8_t val;
    CS_XV7011_ON();
    SPI1_RW_Byte(reg | 0x80);
    val = SPI1_RW_Byte(0xFF);
    CS_XV7011_OFF();
    return val;
}

static void XV7011_ReadBytes(uint8_t reg, uint8_t length, uint8_t *data)
{
    uint8_t i;
    CS_XV7011_ON();
    SPI1_RW_Byte(reg | 0x80);
    for (i = 0; i < length; i++) {
        data[i] = SPI1_RW_Byte(0xFF);
    }
    CS_XV7011_OFF();
}

static float angle_normalize_180(float angle)
{
    angle = fmodf(angle, 360.0f);
    if (angle > 180.0f)      angle -= 360.0f;
    if (angle <= -180.0f)    angle += 360.0f;
    return angle;
}

static float angle_normalize_360(float angle)
{
    angle = fmodf(angle, 360.0f);
    if (angle < 0.0f) {
        angle += 360.0f;
    }
    return angle;
}

/* 平滑死区增益：|rate| 小时 k→0，大时 k→1，中间线性过渡 */
static float gyro_deadzone_gain(float rate)
{
    float abs_r = fabsf(rate);
    if (abs_r <= GYRO_DEADZONE_DPS) {
        return 0.0f;
    }
    if (abs_r >= GYRO_DEADZONE_DPS + GYRO_GAIN_TRANS_DPS) {
        return 1.0f;
    }
    return (abs_r - GYRO_DEADZONE_DPS) / GYRO_GAIN_TRANS_DPS;
}

//////////////////////////////////////////////////////////////////////////////////////////
void XV7011_Init(void)
{
    CS_XV7011_OFF();
    HAL_Delay(1);
    XV7011_WriteReg(GYRO_SW_RST, 0x00);
    HAL_Delay(10);
    XV7011_WriteReg(GYRO_IF_CTL, 0x00);
    XV7011_WriteReg(GYRO_DSP_CTL2, 0x01);
    XV7011_WriteReg(GYRO_DSP_CTL3, 0x41);
    XV7011_WriteReg(GYRO_OUT_CTL1, 0x05);
    XV7011_ReadReg(GYRO_SLP_OUT);
    HAL_Delay(5);
    Filter_Init_MAF(&xv7011_filter);
}

void XV7011_CalibrateBias(int samples)
{
    int64_t sum = 0;
    for (int i = 0; i < samples; i++)
    {
        int32_t raw;
        XV7011_ReadData32(&raw);
        sum += raw;
        HAL_Delay(5);
    }
    xv7011_val.gyro_bias =(float)sum / samples /GYRO_PPR_24BIT;
}

void XV7011_ReadData32(int32_t *data)
{
    uint8_t buf[3];
    XV7011_ReadBytes(GYRO_DAT_ACCON, 3, buf);
    int16_t temp = (int16_t)(((buf[0]) << 8) | buf[1]);
    *data = (int32_t)((temp << 8) | buf[2]);
}

void XV7011_ReadData16(int16_t *data)
{
    uint8_t buf[2];
    XV7011_ReadBytes(GYRO_DAT_ACCON, 2, buf);
    *data = (int16_t)(((uint16_t)buf[0] << 8) | buf[1]);
}

void XV7011_Update(void)
{
    int32_t data;
    XV7011_ReadData32(&data);
    xv7011_val.gyro_z = ((float)data/GYRO_PPR_24BIT - xv7011_val.gyro_bias);
    xv7011_val.gyro_z = MAF_Filter(&xv7011_filter,xv7011_val.gyro_z);
    float k = gyro_deadzone_gain(xv7011_val.gyro_z);
    xv7011_val.yaw += k* xv7011_val.gyro_z * GYRO_PERIOD_MS / 1000.0f;
    xv7011_val.yaw_nor = angle_normalize_180(xv7011_val.yaw);
}

