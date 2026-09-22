#include "NRF24L01.h"
#include "cmsis_os.h"            /* ������������ó� */

/*===== ���Ŷ��壨��CubeMX���ñ���һ�£� =====*/
#define NRF24L01_CE_PORT    GPIOA
#define NRF24L01_CE_PIN     GPIO_PIN_8

#define NRF24L01_CSN_PORT   GPIOA
#define NRF24L01_CSN_PIN    GPIO_PIN_15

#define NRF24L01_SCK_PORT   GPIOB
#define NRF24L01_SCK_PIN    GPIO_PIN_3

#define NRF24L01_MISO_PORT  GPIOB
#define NRF24L01_MISO_PIN   GPIO_PIN_4

#define NRF24L01_MOSI_PORT  GPIOB
#define NRF24L01_MOSI_PIN   GPIO_PIN_5

/*ȫ�ֱ���*/
uint8_t NRF24L01_TxAddress[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
#define NRF24L01_TX_PACKET_WIDTH    32
uint8_t NRF24L01_TxPacket[NRF24L01_TX_PACKET_WIDTH];

uint8_t NRF24L01_RxAddress[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
#define NRF24L01_RX_PACKET_WIDTH    32
uint8_t NRF24L01_RxPacket[NRF24L01_RX_PACKET_WIDTH];

/*==================== �������� ====================*/

/**
  * ��    ����NRF24L01дCE�ߵ͵�ƽ
  */
void NRF24L01_W_CE(uint8_t BitValue)
{
    HAL_GPIO_WritePin(NRF24L01_CE_PORT, NRF24L01_CE_PIN, (GPIO_PinState)BitValue);
}

/**
  * ��    ����NRF24L01дCSN�ߵ͵�ƽ
  */
void NRF24L01_W_CSN(uint8_t BitValue)
{
    HAL_GPIO_WritePin(NRF24L01_CSN_PORT, NRF24L01_CSN_PIN, (GPIO_PinState)BitValue);
}

/**
  * ��    ����NRF24L01дSCK�ߵ͵�ƽ
  */
void NRF24L01_W_SCK(uint8_t BitValue)
{
    HAL_GPIO_WritePin(NRF24L01_SCK_PORT, NRF24L01_SCK_PIN, (GPIO_PinState)BitValue);
}

/**
  * ��    ����NRF24L01дMOSI�ߵ͵�ƽ
  */
void NRF24L01_W_MOSI(uint8_t BitValue)
{
    HAL_GPIO_WritePin(NRF24L01_MOSI_PORT, NRF24L01_MOSI_PIN, (GPIO_PinState)BitValue);
}

/**
  * ��    ����NRF24L01��MISO�ߵ͵�ƽ
  */
uint8_t NRF24L01_R_MISO(void)
{
    return HAL_GPIO_ReadPin(NRF24L01_MISO_PORT, NRF24L01_MISO_PIN);
}

/**
  * ��    ����NRF24L01���ų�ʼ��
  * ˵    �������ŷ���/ģʽ����CubeMX��MX_GPIO_Init()��ɣ�
  *           �˴��������ϵ���Ĭ�ϵ�ƽ
  */
void NRF24L01_GPIO_Init(void)
{
    NRF24L01_W_CE(0);       //CEĬ��Ϊ0���˳��շ�ģʽ
    NRF24L01_W_CSN(1);      //CSNĬ��Ϊ1����ѡ�дӻ�
    NRF24L01_W_SCK(0);      //SCKĬ��Ϊ0����ӦSPIģʽ0
    NRF24L01_W_MOSI(0);     //MOSIĬ�ϵ�ƽ
}

/*==================== ͨ��Э�� ====================*/

/**
  * ��    ����SPI����һ���ֽ�
  */
void NRF24L01_Delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 100; i++);   /* 72MHz 下约 1us，等 SCK/数据稳定 */
}

uint8_t NRF24L01_SPI_SwapByte(uint8_t Byte)
{
    uint8_t i;

    /*�˴�ʹ��SPIģʽ0����ͨ��*/
    for (i = 0; i < 8; i ++)
    {
        if (Byte & 0x80)
        {
            NRF24L01_W_MOSI(1);
        }
        else
        {
            NRF24L01_W_MOSI(0);
        }
        Byte <<= 1;

        /*����SCK������*/
        NRF24L01_W_SCK(1);
        NRF24L01_Delay();

        /*��MISO�����������ݣ�����Byte�����λ*/
        if (NRF24L01_R_MISO())
        {
            Byte |= 0x01;
        }

        /*����SCK�½���*/
        NRF24L01_W_SCK(0);
        NRF24L01_Delay();
    }

    return Byte;
}

/*==================== ָ��ʵ�� ====================*/

/**
  * ��    ����NRF24L01��ȡ�Ĵ�����һ���ֽڣ�
  */
uint8_t NRF24L01_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;

    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_R_REGISTER | RegAddress);
    Data = NRF24L01_SPI_SwapByte(NRF24L01_NOP);
    NRF24L01_W_CSN(1);

    return Data;
}

/**
  * ��    ����NRF24L01��ȡ�Ĵ���������ֽڣ�
  */
void NRF24L01_ReadRegs(uint8_t RegAddress, uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;

    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_R_REGISTER | RegAddress);

    for (i = 0; i < Count; i ++)
    {
        DataArray[i] = NRF24L01_SPI_SwapByte(NRF24L01_NOP);
    }

    NRF24L01_W_CSN(1);
}

/**
  * ��    ����NRF24L01д��Ĵ�����һ���ֽڣ�
  */
void NRF24L01_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_W_REGISTER | RegAddress);
    NRF24L01_SPI_SwapByte(Data);
    NRF24L01_W_CSN(1);
}

/**
  * ��    ����NRF24L01д��Ĵ���������ֽڣ�
  */
void NRF24L01_WriteRegs(uint8_t RegAddress, uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;

    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_W_REGISTER | RegAddress);

    for (i = 0; i < Count; i ++)
    {
        NRF24L01_SPI_SwapByte(DataArray[i]);
    }

    NRF24L01_W_CSN(1);
}

/**
  * ��    ����NRF24L01��ȡRx��Ч�غ�
  */
void NRF24L01_ReadRxPayload(uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;

    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_R_RX_PAYLOAD);

    for (i = 0; i < Count; i ++)
    {
        DataArray[i] = NRF24L01_SPI_SwapByte(NRF24L01_NOP);
    }

    NRF24L01_W_CSN(1);
}

/**
  * ��    ����NRF24L01д��Tx��Ч�غ�
  */
void NRF24L01_WriteTxPayload(uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;

    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_W_TX_PAYLOAD);

    for (i = 0; i < Count; i ++)
    {
        NRF24L01_SPI_SwapByte(DataArray[i]);
    }

    NRF24L01_W_CSN(1);
}

/**
  * ��    ����NRF24L01���Tx FIFO����������
  */
void NRF24L01_FlushTx(void)
{
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_FLUSH_TX);
    NRF24L01_W_CSN(1);
}

/**
  * ��    ����NRF24L01���Rx FIFO����������
  */
void NRF24L01_FlushRx(void)
{
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_FLUSH_RX);
    NRF24L01_W_CSN(1);
}

/**
  * ��    ����NRF24L01��ȡ״̬�Ĵ���
  */
uint8_t NRF24L01_ReadStatus(void)
{
    uint8_t Status;

    NRF24L01_W_CSN(0);
    Status = NRF24L01_SPI_SwapByte(NRF24L01_NOP);
    NRF24L01_W_CSN(1);

    return Status;
}

/*==================== ���ܺ��� ====================*/

/**
  * ��    ����NRF24L01�������ģʽ��CE = 0��PWR_UP = 0��
  */
void NRF24L01_PowerDown(void)
{
    uint8_t Config;

    NRF24L01_W_CE(0);

    Config = NRF24L01_ReadReg(NRF24L01_CONFIG);
    if (Config == 0xFF) {return;}
    Config &= ~0x02;
    NRF24L01_WriteReg(NRF24L01_CONFIG, Config);
}

/**
  * ��    ����NRF24L01�������ģʽ1��CE = 0��PWR_UP = 1��
  */
void NRF24L01_StandbyI(void)
{
    uint8_t Config;

    NRF24L01_W_CE(0);

    Config = NRF24L01_ReadReg(NRF24L01_CONFIG);
    if (Config == 0xFF) {return;}
    Config |= 0x02;
    NRF24L01_WriteReg(NRF24L01_CONFIG, Config);
}

/**
  * ��    ����NRF24L01�������ģʽ��CE = 1��PWR_UP = 1��PRIM_RX = 1��
  */
void NRF24L01_Rx(void)
{
    uint8_t Config;

    NRF24L01_W_CE(0);

    Config = NRF24L01_ReadReg(NRF24L01_CONFIG);
    if (Config == 0xFF) {return;}
    Config |= 0x03;
    NRF24L01_WriteReg(NRF24L01_CONFIG, Config);

    NRF24L01_W_CE(1);
}

/**
  * ��    ����NRF24L01���뷢��ģʽ��CE = 1��PWR_UP = 1��PRIM_RX = 0��
  */
void NRF24L01_Tx(void)
{
    uint8_t Config;

    NRF24L01_W_CE(0);

    Config = NRF24L01_ReadReg(NRF24L01_CONFIG);
    if (Config == 0xFF) {return;}
    Config |= 0x02;
    Config &= ~0x01;
    NRF24L01_WriteReg(NRF24L01_CONFIG, Config);

    NRF24L01_W_CE(1);
}

/**
  * ��    ����NRF24L01��ʼ��
  * ˵    ����ʹ��ǰ������á����ų�ʼ������CubeMX��ɣ�
  *           ����ֻ���Ĵ������ú�FIFO/��־���㡣
  */
void NRF24L01_Init(void)
{
    NRF24L01_GPIO_Init();

    NRF24L01_WriteReg(NRF24L01_CONFIG,      0x08);   //�������жϣ�ʹ��CRC��CRC 1�ֽڣ�PWR_UP=0��PRIM_RX=0
    NRF24L01_WriteReg(NRF24L01_EN_AA,       0x3F);   //ʹ��ͨ��0~5�Զ�Ӧ��
    NRF24L01_WriteReg(NRF24L01_EN_RXADDR,   0x01);   //ֻ��������ͨ��0
    NRF24L01_WriteReg(NRF24L01_SETUP_AW,    0x03);   //��ַ����5�ֽ�
    NRF24L01_WriteReg(NRF24L01_SETUP_RETR,  0x03);   //���250us���ش�3��
    NRF24L01_WriteReg(NRF24L01_RF_CH,       0x02);   //2.402GHz
    NRF24L01_WriteReg(NRF24L01_RF_SETUP,    0x0E);   //2Mbps��0dBm

    NRF24L01_WriteReg(NRF24L01_RX_PW_P0, NRF24L01_RX_PACKET_WIDTH);
    NRF24L01_WriteRegs(NRF24L01_RX_ADDR_P0, NRF24L01_RxAddress, 5);

    NRF24L01_FlushTx();
    NRF24L01_FlushRx();

    NRF24L01_WriteReg(NRF24L01_STATUS, 0x70);        //�� MAX_RT / TX_DS / RX_DR

    NRF24L01_Rx();
}

/**
  * ��    ����NRF24L01�������ݰ�
  * �� �� ֵ��1=�ɹ�  2=�ﵽ����ط�����  3=״̬���Ϸ�  4=���ͳ�ʱ
  */
uint8_t NRF24L01_Send(void)
{
    uint8_t Status;
    uint8_t SendFlag;
    uint32_t Timeout;

    NRF24L01_WriteRegs(NRF24L01_TX_ADDR,    NRF24L01_TxAddress, 5);
    NRF24L01_WriteRegs(NRF24L01_RX_ADDR_P0, NRF24L01_TxAddress, 5);  //���ڽ���Ӧ��

    NRF24L01_WriteTxPayload(NRF24L01_TxPacket, NRF24L01_TX_PACKET_WIDTH);

    NRF24L01_Tx();

    Timeout = 10000;

    while (1)
    {
        Status = NRF24L01_ReadStatus();

        Timeout --;
        if (Timeout == 0)
        {
            SendFlag = 4;
            NRF24L01_Init();
            break;
        }

        if ((Status & 0x30) == 0x30)
        {
            SendFlag = 3;
            NRF24L01_Init();
            break;
        }
        else if ((Status & 0x10) == 0x10)
        {
            SendFlag = 2;
            NRF24L01_Init();
            break;
        }
        else if ((Status & 0x20) == 0x20)
        {
            SendFlag = 1;
            break;
        }

        /* FreeRTOS��ѭ���ȴ�ʱ�ó�CPU����������ȼ���������������� */
        taskYIELD();
    }

    NRF24L01_WriteReg(NRF24L01_STATUS, 0x30);
    NRF24L01_FlushTx();

    /*�ָ�����ͨ��0��ַ*/
    NRF24L01_WriteRegs(NRF24L01_RX_ADDR_P0, NRF24L01_RxAddress, 5);

    NRF24L01_Rx();

    return SendFlag;
}

/**
  * ��    ����NRF24L01�������ݰ�
  * �� �� ֵ��0=δ����  1=�ɹ��յ�  2=״̬���Ϸ�  3=�豸�Դ��ڵ���
  */
uint8_t NRF24L01_Receive(void)
{
    uint8_t Status, Config;
    uint8_t ReceiveFlag;

    Status = NRF24L01_ReadStatus();
    Config = NRF24L01_ReadReg(NRF24L01_CONFIG);

    if ((Config & 0x02) == 0x00)
    {
        ReceiveFlag = 3;
        NRF24L01_Init();
    }
    else if ((Status & 0x30) == 0x30)
    {
        ReceiveFlag = 2;
        NRF24L01_Init();
    }
    else if ((Status & 0x40) == 0x40)
    {
        ReceiveFlag = 1;

        NRF24L01_ReadRxPayload(NRF24L01_RxPacket, NRF24L01_RX_PACKET_WIDTH);

        NRF24L01_WriteReg(NRF24L01_STATUS, 0x40);
        NRF24L01_FlushRx();
    }
    else
    {
        ReceiveFlag = 0;
    }

    return ReceiveFlag;
}

/**
  * ��    ����NRF24L01���½��յ�ַ
  */
void NRF24L01_UpdateRxAddress(void)
{
    NRF24L01_WriteRegs(NRF24L01_RX_ADDR_P0, NRF24L01_RxAddress, 5);
}
