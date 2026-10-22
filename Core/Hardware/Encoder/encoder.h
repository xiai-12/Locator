#ifndef ROBOCON_TEST_ENCODER_H
#define ROBOCON_TEST_ENCODER_H

#include "main.h"
#include "tim.h"
#include "stdbool.h"

typedef struct {
    uint32_t x_raw_count;
    uint32_t y_raw_count;
    int32_t  x_delta_count;
    int32_t  y_delta_count;
    int32_t  x_total_count;
    int32_t  y_total_count;
} Loc_encoder_raw;

typedef struct
{
    // rad/s
    float x_speed;
    float y_speed;
    // turn
    float x_turns;
    float y_turns;
    // m
    float x_distance;
    float y_distance;
}Loc_encoder_val;

extern Loc_encoder_raw loc_encoder;
extern Loc_encoder_val loc_encoder_val;

HAL_StatusTypeDef Encoder_Init(void);
void Encoder_GetRaw(void);
void Encoder_reset(void);
void  Encoder_clearFlag(void);
void Encoder_convert(Loc_encoder_raw* encoder_raw,Loc_encoder_val* encoder_val);
void Encoder_update(void);

#endif //ROBOCON_TEST_ENCODER_H
