//
// Created by zhaol on 2026/10/11.
//
#include "locator.h"

void StartLocatorUptTask(void *argument)
{
    /* USER CODE BEGIN StartLocatorUptTask */
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(10);

    HAL_Delay(3000);

    Encoder_Init();
    XV7011_Init();
    XV7011_CalibrateBias(200);

    // '[' + data_length + data(32) + '\r' + ‘\n’
    uint8_t tx_buf[32];
    uint8_t tx_data[36];

    tx_data[0] = '[';
    tx_data[1] = '8';
    tx_data[34] = '\r';
    tx_data[35] = '\n';

    /* Infinite loop */
    for(;;)
    {
        Encoder_update();
        XV7011_Update();

        float data[8] = {
            (float)loc_encoder.x_total_count,
            (float)loc_encoder.y_total_count,
            loc_encoder_val.x_turns,
            loc_encoder_val.y_turns,
            loc_encoder_val.x_distance,
            loc_encoder_val.y_distance,
            xv7011_val.yaw,
            xv7011_val.yaw_nor
        };
        memcpy(tx_buf, data, sizeof(data));

        for (uint8_t i = 0; i < 32; i++)
        {
            tx_data[i+2] = tx_buf[i];
        }

        HAL_UART_Transmit(&huart3, tx_data, sizeof(tx_data), 10);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
    /* USER CODE END StartLocatorUptTask */
}