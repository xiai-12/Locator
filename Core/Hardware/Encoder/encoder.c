#include "encoder.h"
#include "encoder_config.h"

Loc_encoder_raw loc_encoder={0};
Loc_encoder_val loc_encoder_val={0};

static int32_t enc_x_prev,enc_y_prev = 0;

/////////////////////////////////////////////////////////////////////////////////////
// 将32位uint 转化为 int32
static int32_t encoder_trans_32(uint32_t now)
{
    return (int32_t)(now);
}
// 将16位uint 转化为 int32
static int32_t encoder_trans_16(uint16_t now)
{
    return (int16_t)now;
}
/////////////////////////////////////////////////////////////////////////////////////


// init
HAL_StatusTypeDef Encoder_Init(void){
    if (HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1 | TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    return HAL_OK;
}

void Encoder_GetRaw(void){
    loc_encoder.x_raw_count = __HAL_TIM_GET_COUNTER(&htim2);
    loc_encoder.y_raw_count = __HAL_TIM_GET_COUNTER(&htim4);
}


void Encoder_reset(void)
{
    enc_x_prev = enc_y_prev = 0;
    loc_encoder.x_total_count = loc_encoder.y_total_count = 0;
    loc_encoder.x_delta_count = loc_encoder.y_delta_count = 0;
    loc_encoder.x_raw_count = loc_encoder.y_raw_count = 0;
    loc_encoder_val.x_turns = loc_encoder_val.y_turns = 0.0f;
    loc_encoder_val.x_distance = loc_encoder_val.y_distance = 0.0f;
    loc_encoder_val.x_speed = loc_encoder_val.y_speed = 0.0f;
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
}

void  Encoder_clearFlag(void)
{

}

/////////////////////////////////////////////////////////////////////////////////////


void Encoder_convert(Loc_encoder_raw* encoder_raw,Loc_encoder_val* encoder_val)
{
    // cnt -> turns -> rad  LOCATER_PI/ 180= 0.0174532925199433
    encoder_val->x_speed = (float)encoder_raw->x_delta_count * 1000.0f * TURNS_TO_RAD/ LOCATER_ENCODER_PPR / LOCATER_ENCODER_PERIOD_MS;
    encoder_val->y_speed = (float)encoder_raw->y_delta_count * 1000.0f * TURNS_TO_RAD/ LOCATER_ENCODER_PPR / LOCATER_ENCODER_PERIOD_MS;

    encoder_val->x_turns = (float)encoder_raw->x_total_count / LOCATER_ENCODER_PPR;
    encoder_val->y_turns = (float)encoder_raw->y_total_count / LOCATER_ENCODER_PPR;

    encoder_val->x_distance = (float)encoder_raw->x_total_count * LOCATER_ENCODER_SCALE_P_MM;
    encoder_val->y_distance = (float)encoder_raw->y_total_count * LOCATER_ENCODER_SCALE_Q_MM;
}


void Encoder_update(void)
{
    Encoder_GetRaw();
    // const int32_t x_temp = encoder_trans_32(loc_encoder.x_raw_count);
    // const int32_t y_temp = encoder_trans_16(loc_encoder.y_raw_count);
    //
    // loc_encoder.x_delta_count = x_temp - enc_x_prev;
    // loc_encoder.y_delta_count = y_temp - enc_y_prev;
    // enc_x_prev = x_temp;
    // enc_y_prev = y_temp;
    //
    // loc_encoder.x_total_count += loc_encoder.x_delta_count;
    // loc_encoder.y_total_count += loc_encoder.y_delta_count;
    // 按定时器位宽做模差，差值解释为有符号数 → 自动处理回绕
    const int32_t x_delta = encoder_trans_32(loc_encoder.x_raw_count - (uint32_t)enc_x_prev);
    const int32_t y_delta = encoder_trans_16(loc_encoder.y_raw_count - (uint16_t)enc_y_prev);

    loc_encoder.x_delta_count = x_delta;
    loc_encoder.y_delta_count = y_delta;
    // prev
    enc_x_prev = (int32_t)loc_encoder.x_raw_count;
    enc_y_prev = (int32_t)loc_encoder.y_raw_count;

    loc_encoder.x_total_count += x_delta;
    loc_encoder.y_total_count += y_delta;

    Encoder_convert(&loc_encoder,&loc_encoder_val);
}

/////////////////////////////////////////////////////////////////////////////////////

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == Z1_Pin)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
    }
    else if (GPIO_Pin == Z2_Pin)
    {
    }
    else
    {

    }
}

