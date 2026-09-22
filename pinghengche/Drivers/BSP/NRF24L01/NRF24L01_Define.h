#ifndef __NRF24L01_DEFINE_H
#define __NRF24L01_DEFINE_H

/*NRF24L01指令宏定义*/
#define NRF24L01_R_REGISTER         0x00    //读寄存器，高3位为指令码，低5位为寄存器地址
#define NRF24L01_W_REGISTER         0x20    //写寄存器
#define NRF24L01_R_RX_PAYLOAD       0x61    //读Rx有效载荷
#define NRF24L01_W_TX_PAYLOAD       0xA0    //写Tx有效载荷
#define NRF24L01_FLUSH_TX           0xE1    //清空Tx FIFO
#define NRF24L01_FLUSH_RX           0xE2    //清空Rx FIFO
#define NRF24L01_REUSE_TX_PL        0xE3    //重用最后一次有效载荷
#define NRF24L01_R_RX_PL_WID        0x60    //读Rx FIFO最前面数据包宽度
#define NRF24L01_W_ACK_PAYLOAD      0xA8    //写应答附带有效载荷
#define NRF24L01_W_TX_PAYLOAD_NOACK 0xB0    //写Tx有效载荷，不要求应答
#define NRF24L01_NOP                0xFF    //空操作

/*NRF24L01寄存器地址宏定义*/
#define NRF24L01_CONFIG             0x00
#define NRF24L01_EN_AA              0x01
#define NRF24L01_EN_RXADDR          0x02
#define NRF24L01_SETUP_AW           0x03
#define NRF24L01_SETUP_RETR         0x04
#define NRF24L01_RF_CH              0x05
#define NRF24L01_RF_SETUP           0x06
#define NRF24L01_STATUS             0x07
#define NRF24L01_OBSERVE_TX         0x08
#define NRF24L01_RPD                0x09
#define NRF24L01_RX_ADDR_P0         0x0A
#define NRF24L01_RX_ADDR_P1         0x0B
#define NRF24L01_RX_ADDR_P2         0x0C
#define NRF24L01_RX_ADDR_P3         0x0D
#define NRF24L01_RX_ADDR_P4         0x0E
#define NRF24L01_RX_ADDR_P5         0x0F
#define NRF24L01_TX_ADDR            0x10
#define NRF24L01_RX_PW_P0           0x11
#define NRF24L01_RX_PW_P1           0x12
#define NRF24L01_RX_PW_P2           0x13
#define NRF24L01_RX_PW_P3           0x14
#define NRF24L01_RX_PW_P4           0x15
#define NRF24L01_RX_PW_P5           0x16
#define NRF24L01_FIFO_STATUS        0x17
#define NRF24L01_DYNPD              0x1C
#define NRF24L01_FEATURE            0x1D

#endif
