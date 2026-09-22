#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f1xx_hal.h"

/* ================= 内部配置宏 ================= */

/* DMA 循环接收缓冲区大小（须为 2 的幂） */
#define SERIAL_RX_DMA_BUF_SIZE   256u
/* 软件接收环形缓冲区大小（须为 2 的幂） */
#define SERIAL_RX_RING_SIZE      256u
/* 单帧最大字节数（含 '[' ']'） */
#define SERIAL_FRAME_MAX_LEN     128u
/* 发送字节环形队列大小（须为 2 的幂） */
#define SERIAL_TX_RING_SIZE      512u
/* 每次 DMA 发送的最大字节数 */
#define SERIAL_TX_DMA_CHUNK      128u
/* 不完整帧超时丢弃时间（ms） */
#define SERIAL_FRAME_TIMEOUT_MS  100u

/* ================= 初始化 ================= */

// 初始化串口收发（底层 USART2 由 CubeMX 生成，这里启动 DMA 接收与发送队列）
void Serial_Init(void);

/* ================= 发送函数（非阻塞，DMA 队列，不阻塞电机） ================= */

// 发送 1 个字节（入队，非阻塞）
void Serial_SendByte(uint8_t Byte);

// 发送指定长度的 uint8_t 数组（入队，非阻塞）
void Serial_SendArray(uint8_t *Array, uint16_t Length);

// 发送以 '\0' 结尾的字符串（入队，非阻塞）
void Serial_SendString(char *String);

// 整数幂计算，SendNumber 内部辅助使用
uint32_t Serial_Pow(uint32_t X, uint32_t Y);

// 发送固定长度的十进制数字，例如 Serial_SendNumber(123, 5) -> "00123"
void Serial_SendNumber(uint32_t Number, uint8_t Length);

// 串口格式化打印（内部缓冲区 100 字节，注意不要超长）
void Serial_Printf(char *format, ...);

/* ================= 内部集成接口（由 CubeMX 胶水代码调用） ================= */

// 由 USART2 IDLE 中断调用
void Serial_RxIdleCallback(void);

// 由 SERIALTask 周期调用
void Serial_Process(void);

/* ================= 命令回调（弱函数，可在用户代码中重写） ================= */

// 收到完整一帧内容（不含 '[' ']'，\r \n 等转义已还原）后调用；默认原样回发
void Serial_OnCommand(char *cmd);

#endif
