# 4_2locator_f405 说明文档

定位模块（Locator）固件，运行于 STM32F405 + FreeRTOS。主要功能：采集两路编码器（X/Y）与 XV7011 陀螺仪，做累计计数、圈数、里程和航向角解算，并以固定帧格式从 USART3 上传给主控。

本文重点说明 **通信接口** 与 **通信数据格式**。

---

## 1. 通信接口总览

| 接口 | 方向 | 引脚 | 参数 | 用途 | 代码位置 |
| --- | --- | --- | --- | --- | --- |
| USART3 | 发送（上行） | PC10 = TX, PC11 = RX | 115200, 8N1, 无流控 | 定位数据主动上传 | `Core/Src/usart.c`、`Core/Application/src/locator.c` |
| USART1 | 接收（指令/调试） | PA9 = TX, PA10 = RX | 115200, 8N1, 无流控 | 上位机指令入口（当前为回显调试） | `Core/Application/src/serial_cmd.c` |

---

## 2. USART3：定位数据上传帧

### 2.1 链路参数

- 波特率 **115200**，8 数据位，1 停止位，无校验，无硬件流控
- 发送周期 **10 ms（100 Hz）**，由 `StartLocatorUptTask` 用 `vTaskDelayUntil` 保证
- 发送方式：`HAL_UART_Transmit()` 阻塞发送，36 字节在 115200 下约 3.1 ms
- 只发不收：USART3 的 RX 未使用
- 上电后先 `HAL_Delay(3000)`，再初始化编码器/陀螺仪并做 200 次零偏校准（约 1 s），因此**上电约 4 s 后才会出现第一帧**

### 2.2 帧结构（固定 36 字节）

```
偏移   长度  内容          说明
0      1     0x5B '['      帧头
1      1     0x38 '8'      字段个数（= 8 个 float），注意：不是字节数
2      32    8 × float32   数据区，小端（ARM Cortex-M4 default little-endian）
34     1     0x0D '\r'     帧尾 CR
35     1     0x0A '\n'     帧尾 LF
```
即：
```
[ '8' | float32 × 8 | \r \n ]
```

- **无校验和、无 CRC、无序号、无时间戳**，仅靠 `'['` 帧头与 `\r\n` 帧尾做同步
- 帧长恒定，接收方可按 36 字节定长切帧，也可按 `'\n'` 切帧后校验长度
- 数据区是把 `float data[8]` 数组直接 `memcpy` 进发送缓冲，**字段顺序 = 数组下标顺序**，与结构体填充无关

### 2.3 字段定义（数据区，相对偏移 2）

权重：每个字段 4 字节，小端 IEEE-754 单精度浮点。

| 索引 | 数据区偏移 | 绝对偏移 | 字段 | 类型 | 单位 | 含义 / 计算式 |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | +0 | 2 | `x_total_count` | float(由 int32 强转) | count | X 轴编码器累计计数（含回绕修正） |
| 1 | +4 | 6 | `y_total_count` | float(由 int32 强转) | count | Y 轴编码器累计计数 |
| 2 | +8 | 10 | `x_turns` | float | turn(圈) | `x_total_count / 2048` |
| 3 | +12 | 14 | `y_turns` | float | turn(圈) | `y_total_count / 2048` |
| 4 | +16 | 18 | `x_distance` | float | m（见注） | `x_total_count × 0.00771069429` |
| 5 | +20 | 22 | `y_distance` | float | m（见注） | `y_total_count × 0.00771069429` |
| 6 | +24 | 26 | `yaw` | float | °（度） | 陀螺 Z 轴角速度积分得到的累计航向角，**未归一化**，会持续累加，可超过 ±360° |
| 7 | +28 | 30 | `yaw_nor` | float | °（度） | `yaw` 归一化到 `(-180, 180]` |


### 2.4 解析示例


C（接收端）：

```
// 码盘+imu数据接收体
typedef struct Encoder_imu_Structor
{
    float x_count;          // x原始值
    float y_count;          // y原始值
    float x_turns;          // x圈数
    float y_turns;          // y圈数
    float x_distance;       // x距离，m
    float y_distance;       // y距离，m
    float yaw;              // 航向角
    float yaw_nor;          // 航向角(归一化)
}Encoder_imu_CMD;

Encoder_imu_CMD Encoder_imu_cmd_structor;

//码盘+imu解析函数
void Parse_location_line(const uint8_t* cmd,uint16_t len)
{
    uint8_t rx_buf[32];
    float rx_data[8];
    if (cmd[0] == '[' && cmd[1] == '8' && len == 36)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
        rx_buf[i] = cmd[i+2];
        }
        memcpy(rx_data,rx_buf,sizeof(rx_buf));
        Encoder_imu_cmd_structor.x_count = rx_data[0];
        Encoder_imu_cmd_structor.y_count = rx_data[1];
        Encoder_imu_cmd_structor.x_turns = rx_data[2];
        Encoder_imu_cmd_structor.y_turns = rx_data[3];
        Encoder_imu_cmd_structor.x_distance = rx_data[4]/100.0f; // m
        Encoder_imu_cmd_structor.y_distance = rx_data[5]/100.0f; // m
        Encoder_imu_cmd_structor.yaw = rx_data[6];
        Encoder_imu_cmd_structor.yaw_nor = rx_data[7];
    }
}
```


