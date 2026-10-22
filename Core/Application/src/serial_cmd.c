#include "../inc/serial_cmd.h"

#include <sys/types.h>

#define serial1_line_max 64                   // 串口接收最大数量
static uint8_t serial1_line[serial1_line_max]; // 串口接收缓存数组
volatile uint8_t serial1_flag = 0;            // 串口中断标志位
volatile uint16_t serial1_len  = 0;           // 串口数据长度


// 串口任务函数，用于将队列的数据转移到缓存数组里，同时对串口命令进行解析
void StartSerialCmdTask(void *argument)
{
    /* USER CODE BEGIN StartVisionCmdTask */
    // 使用Ex函数，接收不定长数据
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, serial1_line,sizeof(serial1_line));
    // 关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    /* Infinite loop */
    for(;;)
    {
        if (serial1_flag)
        {
            serial1_flag = 0;
            SERIAL1_printf("msg:%s\r\n",serial1_line);
            //Parse_serial1_line((char *)serial1_line);
            memset(serial1_line,0,sizeof(serial1_line));
        }
        osDelay(5);
    }
    /* USER CODE END StartVisionCmdTask */
}



// 串口接收中断，用于将接收到的数据转移到队列
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart1)
    {
        serial1_len  = Size;
        serial1_flag = 1;
        /* 必须在中断里立刻重新挂起，否则 IDLE 之后 DMA 已经停了 */
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, serial1_line, sizeof(serial1_line));
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * 串口发送函数，根据模式选择发送不同内容
 */
void StartSerialTxTask(void *argument)
{
    /* USER CODE BEGIN StartSerialTXTask */
    /* Infinite loop */
    for(;;)
    {
        osDelay(20);
    }
    /* USER CODE END StartSerialTXTask */
}

/* 拔线/悬空/过载后 HAL 会停掉 DMA 接收，这里把它重新挂上 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        huart->ErrorCode = HAL_UART_ERROR_NONE;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, serial1_line, sizeof(serial1_line));
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }
}