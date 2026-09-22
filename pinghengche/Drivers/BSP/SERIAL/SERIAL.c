#include "SERIAL/SERIAL.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* ---------------- 接收：DMA 循环缓冲 -> 软件环形缓冲 ---------------- */
static uint8_t  dma_rx_buf[SERIAL_RX_DMA_BUF_SIZE];
static uint8_t  rx_ring[SERIAL_RX_RING_SIZE];
static volatile uint16_t rx_ring_head = 0;  /* 生产者(ISR)写入 */
static volatile uint16_t rx_ring_tail = 0;  /* 消费者(任务)读取 */
static volatile uint16_t dma_last_pos  = 0;
static volatile uint8_t  rx_restart    = 0;

/* ---------------- 帧解析状态（任务上下文） ---------------- */
static uint8_t  frame_buf[SERIAL_FRAME_MAX_LEN];
static uint16_t frame_len      = 0;
static uint8_t  frame_active   = 0;
static uint32_t frame_start_ms = 0;

/* ---------------- 发送：字节环形队列（多任务共享，临界区保护） ---------------- */
static uint8_t  tx_ring[SERIAL_TX_RING_SIZE];
static volatile uint16_t tx_ring_head = 0;  /* 生产者 */
static volatile uint16_t tx_ring_tail = 0;  /* 消费者 */
static uint8_t  tx_dma_buf[SERIAL_TX_DMA_CHUNK];
static volatile uint8_t tx_busy = 0;

/* 启动/重启 DMA 循环接收并打开 IDLE 中断 */
static void Serial_StartRx(void)
{
    dma_last_pos = 0;
    rx_ring_head = 0;
    rx_ring_tail = 0;
    __HAL_UART_CLEAR_OREFLAG(&huart2);
    __HAL_UART_CLEAR_IDLEFLAG(&huart2);
    if (HAL_UART_Receive_DMA(&huart2, dma_rx_buf, SERIAL_RX_DMA_BUF_SIZE) == HAL_OK)
    {
        rx_restart = 0;
        __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    }
    /* 失败则保持 rx_restart 置位，由任务下一轮重试 */
}

/* 启动下一块 DMA 发送；DMA 忙则返回，由 TX 完成回调在下一轮再触发 */
static void Serial_TxKick(void)
{
    uint16_t n = 0;

    taskENTER_CRITICAL();
    if ((tx_busy == 0u) && (tx_ring_head != tx_ring_tail))
    {
        uint16_t avail;
        if (tx_ring_head > tx_ring_tail)
        {
            avail = tx_ring_head - tx_ring_tail;
        }
        else
        {
            avail = SERIAL_TX_RING_SIZE - tx_ring_tail;
        }
        if (avail > SERIAL_TX_DMA_CHUNK)
        {
            avail = SERIAL_TX_DMA_CHUNK;
        }
        for (uint16_t i = 0; i < avail; i++)
        {
            tx_dma_buf[i] = tx_ring[tx_ring_tail + i];
        }
        tx_ring_tail = (uint16_t)((tx_ring_tail + avail) & (SERIAL_TX_RING_SIZE - 1u));
        tx_busy = 1;
        n = avail;
    }
    taskEXIT_CRITICAL();

    if (n > 0u)
    {
        HAL_UART_Transmit_DMA(&huart2, tx_dma_buf, n);
    }
}

void Serial_Init(void)
{
    tx_ring_head = 0;
    tx_ring_tail = 0;
    tx_busy = 0;
    frame_len    = 0;
    frame_active = 0;
    Serial_StartRx();
}

/* IDLE 中断回调：仅把 DMA 新字节拷贝进接收环形缓冲，不做解析 */
void Serial_RxIdleCallback(void)
{
    uint16_t pos = (uint16_t)((SERIAL_RX_DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx)) & (SERIAL_RX_DMA_BUF_SIZE - 1u));
    uint16_t n   = (uint16_t)((pos - dma_last_pos) & (SERIAL_RX_DMA_BUF_SIZE - 1u));

    for (uint16_t i = 0; i < n; i++)
    {
        uint16_t next = (uint16_t)((rx_ring_head + 1u) & (SERIAL_RX_RING_SIZE - 1u));
        if (next == rx_ring_tail)
        {
            /* 环形缓冲满：丢弃后续字节，等任务取走，避免死锁 */
            break;
        }
        rx_ring[rx_ring_head] = dma_rx_buf[(dma_last_pos + i) & (SERIAL_RX_DMA_BUF_SIZE - 1u)];
        rx_ring_head = next;
    }
    dma_last_pos = pos;
}

/* ================= 发送函数 ================= */

/* 发送指定长度数组：入队（队列满则丢弃剩余字节，不阻塞），并尝试启动发送 */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    if ((Array == NULL) || (Length == 0u))
    {
        return;
    }

    taskENTER_CRITICAL();
    for (uint16_t i = 0; i < Length; i++)
    {
        uint16_t next = (uint16_t)((tx_ring_head + 1u) & (SERIAL_TX_RING_SIZE - 1u));
        if (next == tx_ring_tail)
        {
            break;
        }
        tx_ring[tx_ring_head] = Array[i];
        tx_ring_head = next;
    }
    taskEXIT_CRITICAL();

    Serial_TxKick();
}

/* 发送 1 个字节（本实现为非阻塞入队，不会像阻塞式那样卡住实时任务） */
void Serial_SendByte(uint8_t Byte)
{
    Serial_SendArray(&Byte, 1u);
}

void Serial_SendString(char *String)
{
    if (String != NULL)
    {
        Serial_SendArray((uint8_t *)String, (uint16_t)strlen(String));
    }
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1u;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    char buf[16];
    uint8_t i;

    for (i = 0; i < Length; i++)
    {
        buf[i] = (char)('0' + (Number / Serial_Pow(10u, (uint32_t)(Length - 1u - i))) % 10u);
    }
    buf[Length] = '\0';
    Serial_SendString(buf);
}

void Serial_Printf(char *format, ...)
{
    char String[100];
    va_list arg;

    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);

    Serial_SendString(String);
}

/* ================= 命令处理 ================= */

/* 把帧内容里的 \r \n \t \\ 转义还原为实际字节（只处理 [] 内的内容） */
static void Serial_Unescape(char *str)
{
    char *src = str;
    char *dst = str;

    while (*src != '\0')
    {
        if ((*src == '\\') && (*(src + 1) != '\0'))
        {
            src++;
            switch (*src)
            {
                case 'r':  *dst++ = '\r'; src++; break;
                case 'n':  *dst++ = '\n'; src++; break;
                case 't':  *dst++ = '\t'; src++; break;
                case '\\': *dst++ = '\\'; src++; break;
                default:   *dst++ = '\\'; *dst++ = *src; src++; break;
            }
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/* 默认命令处理：原样回发内容（转义已还原） */
__weak void Serial_OnCommand(char *cmd)
{
    Serial_SendString(cmd);
}

/* 主循环/任务调用：错误恢复、超时、解析、发送 */
void Serial_Process(void)
{
    /* 1. 溢出等错误恢复：重启接收 */
    if (rx_restart != 0u)
    {
        Serial_StartRx();
    }

    /* 2. 不完整帧超时丢弃，自动恢复 */
    if ((frame_active != 0u) && ((HAL_GetTick() - frame_start_ms) > SERIAL_FRAME_TIMEOUT_MS))
    {
        frame_active = 0;
        frame_len    = 0;
    }

    /* 3. 从软件环形缓冲逐字节解析 '[' ... ']' */
    while (rx_ring_head != rx_ring_tail)
    {
        uint8_t b = rx_ring[rx_ring_tail];
        rx_ring_tail = (uint16_t)((rx_ring_tail + 1u) & (SERIAL_RX_RING_SIZE - 1u));

        if (frame_active == 0u)
        {
            if (b == '[')
            {
                frame_active = 1;
                frame_len    = 0;
                frame_buf[frame_len++] = b;
                frame_start_ms = HAL_GetTick();
            }
            /* 帧外字节（乱码）直接丢弃 */
        }
        else
        {
            frame_buf[frame_len++] = b;
            if (b == ']')
            {
                frame_buf[frame_len - 1u] = '\0';          /* 覆盖 ']'，得到以 '\0' 结尾的内容 */
                Serial_Unescape((char *)&frame_buf[1]);    /* 还原 \r \n 等转义为实际字节 */
                Serial_OnCommand((char *)&frame_buf[1]);   /* 把内容交给回调处理 */
                frame_active = 0;
                frame_len    = 0;
            }
            else if (frame_len >= SERIAL_FRAME_MAX_LEN)
            {
                /* 超长且未结束：丢弃整帧，自动恢复 */
                frame_active = 0;
                frame_len    = 0;
            }
        }
    }

    /* 4. 发送队列处理 */
    Serial_TxKick();
}

/* TX 完成回调：释放发送锁，任务下一轮继续发下一块 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        tx_busy = 0;
    }
}

/* 错误回调：标记溢出等错误，由任务里重启接收 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        rx_restart = 1;
    }
}
