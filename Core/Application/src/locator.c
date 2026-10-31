//
// Created by zhaol on 2026/10/11.
//
#include "locator.h"

void StartLocatorUptTask(void *argument)
{
    /* USER CODE BEGIN StartLocatorUptTask */
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1);

    HAL_Delay(3000);

    Encoder_Init();
    XV7011_Init();
    XV7011_CalibrateBias(200);

    /* Infinite loop */
    for(;;)
    {
        Encoder_update();
        XV7011_Update();

        // float data[3] = {
        //     (float)loc_encoder.x_total_count,
        //     (float)loc_encoder.y_total_count,
        //     xv7011_val.yaw,
        // };
        // memcpy(tx_buf, data, sizeof(data));
        //
        // for (uint8_t i = 0; i < 6; i++)
        // {
        //     tx_data[i+2] = tx_buf[i];
        // }
        //
        // //HAL_UART_Transmit(&huart3, tx_data, sizeof(tx_data), 10);
        // HAL_UART_Transmit_DMA(&huart3, tx_data, sizeof(tx_data));
        static uint8_t tx_buf[16];          // static，别放栈上给 DMA 用
        tx_buf[0] = '[';
        tx_buf[1] = '3';                    // 3 个数据
        float data[3] = {
            (float)loc_encoder.x_total_count,
            (float)loc_encoder.y_total_count,
            xv7011_val.yaw,
        };
        memcpy(&tx_buf[2], data, sizeof(data));   // 12 字节
        tx_buf[14] = '\r';
        tx_buf[15] = '\n';

        if (huart3.gState == HAL_UART_STATE_READY)
        {
            HAL_UART_Transmit_DMA(&huart3, tx_buf, sizeof(tx_buf));
        }


        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
    /* USER CODE END StartLocatorUptTask */
}