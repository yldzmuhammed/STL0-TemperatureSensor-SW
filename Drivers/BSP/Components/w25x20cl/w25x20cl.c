/**
  ******************************************************************************
  * @file    w25x20cl.h
  * @author  yldzmuhammed
  * @version V1.0
  * @date    29-07-2020
  * @brief   This file contains the common defines and functions prototypes for
  *          the w25x20cl.c driver.
  ******************************************************************************
  * @attention
  *
  * experiment platform: YiLDiZ IoT STl0-UTMP SPI Flash Library
  * web-site: https://www.yildiziot.com
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "w25x20cl.h"

#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_spi.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_utils.h"
#include "stm32l0xx_ll_cortex.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define W25x20CL_DUMMY_BYTE                                                     0xA5

/* Private macro -------------------------------------------------------------*/
#define FLASH_POWER_ON()                                                        LL_GPIO_ResetOutputPin  (FLASH_PWREN_GPIO_Port, FLASH_PWREN_Pin)
#define FLASH_POWER_OFF()                                                       LL_GPIO_SetOutputPin    (FLASH_PWREN_GPIO_Port, FLASH_PWREN_Pin)
/* Private variables ---------------------------------------------------------*/
xW25x20CL_TypeDef xW25x20CL;

/* Private function prototypes -----------------------------------------------*/

static void W25x20CL_WriteEnable(void);
static void W25x20CL_WriteDisable(void);
static void W25x20CL_WaitForWriteEnd(void);
static uint8_t W25x20CL_Spi(uint8_t  data);

/* Private user code ---------------------------------------------------------*/
/* Private user function -----------------------------------------------------*/
static void W25x20CL_WaitForWriteEnd(void)
{
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);

    W25x20CL_Spi(0x05);
    do
    {
        xW25x20CL.StatusRegister1 = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
        HAL_Delay(1);
    }while((xW25x20CL.StatusRegister1 & 0x01) == 0x01);
    
    /* deselect spi flash */
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
}
//

static void W25x20CL_WriteEnable(void)
{
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);

    W25x20CL_Spi(0x06);
    
    /* deselect spi flash */
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
}
//

static void W25x20CL_WriteDisable(void)
{
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);

    W25x20CL_Spi(0x04);
    
    /* deselect spi flash */
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
}
//

static uint8_t W25x20CL_Spi(uint8_t  data)
{
    uint8_t ret;
    
    LL_SPI_TransmitData8(SPI1, data);
    
    ret = LL_SPI_ReceiveData8(SPI1);
    
    return ret;
}
//

/* Public user function ------------------------------------------------------*/

/**
  * @brief W25x20CL Initialization Function
  * @param None
  * @retval None
  */
bool  W25x20CL_Init(void)
{
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    xW25x20CL.Lock = 1;

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

    /** SPI1 GPIO Configuration  
     *  PA4   ------> SPI1_NSS
     *  PA5   ------> SPI1_SCK
     *  PA6   ------> SPI1_MISO
     *  PA7   ------> SPI1_MOSI 
     */
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_0;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    
    GPIO_InitStruct.Pin         = FLASH_NSS_Pin;
    LL_GPIO_Init(FLASH_NSS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin         = FLASH_SCK_Pin;
    LL_GPIO_Init(FLASH_SCK_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin         = FLASH_MISO_Pin;
    LL_GPIO_Init(FLASH_MISO_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin         = FLASH_MOSI_Pin;
    LL_GPIO_Init(FLASH_MOSI_GPIO_Port, &GPIO_InitStruct);

    /* SPI1 parameter configuration*/
    SPI_InitStruct.CRCPoly              = 7;
    SPI_InitStruct.BitOrder             = LL_SPI_MSB_FIRST;
    SPI_InitStruct.Mode                 = LL_SPI_MODE_MASTER;
    SPI_InitStruct.ClockPhase           = LL_SPI_PHASE_1EDGE;
    SPI_InitStruct.TransferDirection    = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.ClockPolarity        = LL_SPI_POLARITY_LOW;
    SPI_InitStruct.DataWidth            = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.NSS                  = LL_SPI_NSS_HARD_OUTPUT;
    SPI_InitStruct.CRCCalculation       = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.BaudRate             = LL_SPI_BAUDRATEPRESCALER_DIV256;
    LL_SPI_Init(SPI1, &SPI_InitStruct);
    LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
    
    LL_SPI_Enable(SPI1);
    
    /* Power on flash ic */
    FLASH_POWER_ON();
    
    /* Let flash ic power settle */
    HAL_Delay(100);
    
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);

    switch(W25x20CL_ReadID() & 0x0000FFFF)
    {
        case 0x401A:  //  w25q512
        {
            xW25x20CL.ID = W25Q512;
            xW25x20CL.BlockCount = 1024;
            break;
        }
        case 0x4019:  //  w25q256
        {
            xW25x20CL.ID = W25Q256;
            xW25x20CL.BlockCount = 512;
            break;
        }
        case 0x4018:  //  w25q128
        {
            xW25x20CL.ID = W25Q128;
            xW25x20CL.BlockCount = 256;
            break;
        }
        case 0x4017:  //  w25q64
        {
            xW25x20CL.ID = W25Q64;
            xW25x20CL.BlockCount = 128;
            break;
        }
        case 0x4016:  //  w25q32
        {
            xW25x20CL.ID = W25Q32;
            xW25x20CL.BlockCount = 64;
            break;
        }
        case 0x4015:  //  w25q16
        {
            xW25x20CL.ID = W25Q16;
            xW25x20CL.BlockCount = 32;
            break;
        }
        case 0x4014:  //  w25q80
        {
            xW25x20CL.ID = W25Q80;
            xW25x20CL.BlockCount = 16;
            break;
        }
        case 0x4013:  //  w25q40
        {
            xW25x20CL.ID = W25Q40;
            xW25x20CL.BlockCount = 8;
            break;
        }
        case 0x4012:  //  w25q20
        {
            xW25x20CL.ID = W25Q20;
            xW25x20CL.BlockCount = 4;
            break;
        }
        case 0x4011:  //  w25q10
        {
            xW25x20CL.ID = W25Q10;
            xW25x20CL.BlockCount = 2;
            break;
        }
        default:
        {
            xW25x20CL.Lock = 0;
            return false;
        }
    }
    xW25x20CL.PageSize              = 256;
    xW25x20CL.SectorSize            = 0x1000;
    xW25x20CL.SectorCount           = xW25x20CL.BlockCount * 16;
    xW25x20CL.BlockSize             = xW25x20CL.SectorSize * 16;
    xW25x20CL.CapacityInKiloByte    = (xW25x20CL.SectorCount * xW25x20CL.SectorSize) / 1024;
    xW25x20CL.PageCount             = (xW25x20CL.SectorCount * xW25x20CL.SectorSize) / xW25x20CL.PageSize;
    
    W25x20CL_ReadUniqID();
    
    W25x20CL_ReadStatusRegister(1);
    W25x20CL_ReadStatusRegister(2);
    W25x20CL_ReadStatusRegister(3);
    
    xW25x20CL.Lock = 0;
    
    /* Power off flash ic */
    FLASH_POWER_OFF();
    
    return true;
}
//

uint32_t W25x20CL_ReadID(void)
{
    uint32_t Temp, Temp0, Temp1, Temp2;
    
    /* Power on flash ic */
    FLASH_POWER_ON();
    
    /* Let flash ic power settle */
    HAL_Delay(100);
    
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);

    W25x20CL_Spi(0x9F);
    
    Temp0 = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
    Temp1 = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
    Temp2 = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
    
    /* deselect spi flash */
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);

    /* Power on flash ic */
    FLASH_POWER_OFF();
    
    return (uint32_t)((Temp0 << 16) | (Temp1 << 8) | Temp2);
}
//

void  W25x20CL_EraseChip(void)
{
    xW25x20CL.Lock = 1;
    
    /* Power on flash ic */
    FLASH_POWER_ON();
    
    W25x20CL_WriteEnable();
    
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);

    W25x20CL_Spi(0xC7);
    
    /* deselect spi flash */
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);

    W25x20CL_WaitForWriteEnd();
    
    /* Power on flash ic */
    FLASH_POWER_OFF();
    
    xW25x20CL.Lock = 0;
}
//

void W25x20CL_ReadUniqID(void)
{
    xW25x20CL.Lock = 1;
    
    /* Power on flash ic */
    FLASH_POWER_ON();
    
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    
    W25x20CL_Spi(0x4B);
    
    for(uint8_t i = 0; i < 4; i++)
        W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
    
    for(uint8_t i = 0; i < 8; i++)
        xW25x20CL.UniqID[i] = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
    
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    
    /* Power on flash ic */
    FLASH_POWER_OFF();
    
    xW25x20CL.Lock = 0;
}
//

void W25x20CL_EraseBlock(uint32_t BlockAddr)
{
    xW25x20CL.Lock = 1;
    
    /* Power on flash ic */
    FLASH_POWER_ON();
    
    W25x20CL_WaitForWriteEnd();
    
    BlockAddr = BlockAddr * xW25x20CL.SectorSize * 16;
    
    W25x20CL_WriteEnable();
    
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    
    W25x20CL_Spi(0xD8);
    
    if(xW25x20CL.ID >= W25Q256)
        W25x20CL_Spi((BlockAddr & 0xFF000000) >> 24);
    
    W25x20CL_Spi((BlockAddr & 0xFF0000) >> 16);
    W25x20CL_Spi((BlockAddr & 0xFF00) >> 8);
    W25x20CL_Spi(BlockAddr & 0xFF);
    
    /* deselect spi flash */
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    
    W25x20CL_WaitForWriteEnd();
    
    /* Power on flash ic */
    FLASH_POWER_OFF();
    
    xW25x20CL.Lock = 0;
}
//

void W25x20CL_EraseSector(uint32_t SectorAddr)
{
    xW25x20CL.Lock = 1;
    
    /* Power on flash ic */
    FLASH_POWER_ON();
    
    W25x20CL_WaitForWriteEnd();
    
    SectorAddr = SectorAddr * xW25x20CL.SectorSize;
    
    W25x20CL_WriteEnable();
    
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    
    W25x20CL_Spi(0x20);
    
    if(xW25x20CL.ID >= W25Q256)
        W25x20CL_Spi((SectorAddr & 0xFF000000) >> 24);
    
    W25x20CL_Spi((SectorAddr & 0xFF0000) >> 16);
    W25x20CL_Spi((SectorAddr & 0xFF00) >> 8);
    W25x20CL_Spi(SectorAddr & 0xFF);
    
    /* deselect spi flash */
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    
    W25x20CL_WaitForWriteEnd();
    
    /* Power on flash ic */
    FLASH_POWER_OFF();
    
    xW25x20CL.Lock = 0;
}
//

void W25x20CL_WriteStatusRegister(eStatusReg_TypeDef eStatusReg, uint8_t Data)
{
    /* Power on flash ic */
    FLASH_POWER_ON();
    
    /* select spi flash */
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    
    if( eStatusReg == eStatusReg_1 || eStatusReg == eStatusReg_2 || eStatusReg == eStatusReg_3)
    {
        W25x20CL_Spi(eStatusReg);
        xW25x20CL.StatusRegister1 = Data;
    }
    
    W25x20CL_Spi(Data);
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
}
//

void W25x20CL_WriteByte(uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
    while(xW25x20CL.Lock == 1)
    {
        HAL_Delay(1);
    }
    xW25x20CL.Lock = 1;
#if (_W25x20CL_DEBUG==1)
    uint32_t  StartTime = HAL_GetTick();
    printf("xW25x20CL WriteByte 0x%02X at address %d begin...", pBuffer, WriteAddr_inBytes);
#endif
    W25x20CL_WaitForWriteEnd();
    W25x20CL_WriteEnable();
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    W25x20CL_Spi(0x02);
    if(xW25x20CL.ID >= W25Q256)
    {
        W25x20CL_Spi((WriteAddr_inBytes & 0xFF000000) >> 24);
    }
    W25x20CL_Spi((WriteAddr_inBytes & 0xFF0000) >> 16);
    W25x20CL_Spi((WriteAddr_inBytes & 0xFF00) >> 8);
    W25x20CL_Spi(WriteAddr_inBytes & 0xFF);
    W25x20CL_Spi(pBuffer);
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    W25x20CL_WaitForWriteEnd();
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL WriteByte done after %d ms\r\n", HAL_GetTick() - StartTime);
#endif
    xW25x20CL.Lock = 0;
}
//

void  W25x20CL_WritePage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte,
                         uint32_t NumByteToWrite_up_to_PageSize)
{
    while(xW25x20CL.Lock == 1)
    {
        HAL_Delay(1);
    }
    xW25x20CL.Lock = 1;
    if(((NumByteToWrite_up_to_PageSize + OffsetInByte) > xW25x20CL.PageSize) || (NumByteToWrite_up_to_PageSize == 0))
    {
        NumByteToWrite_up_to_PageSize = xW25x20CL.PageSize - OffsetInByte;
    }
    if((OffsetInByte + NumByteToWrite_up_to_PageSize) > xW25x20CL.PageSize)
    {
        NumByteToWrite_up_to_PageSize = xW25x20CL.PageSize - OffsetInByte;
    }
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL WritePage:%d, Offset:%d ,Writes %d Bytes, begin...\r\n", Page_Address, OffsetInByte,
           NumByteToWrite_up_to_PageSize);
    HAL_Delay(100);
    uint32_t  StartTime = HAL_GetTick();
#endif
    W25x20CL_WaitForWriteEnd();
    W25x20CL_WriteEnable();
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    W25x20CL_Spi(0x02);
    Page_Address = (Page_Address * xW25x20CL.PageSize) + OffsetInByte;
    if(xW25x20CL.ID >= W25Q256)
    {
        W25x20CL_Spi((Page_Address & 0xFF000000) >> 24);
    }
    W25x20CL_Spi((Page_Address & 0xFF0000) >> 16);
    W25x20CL_Spi((Page_Address & 0xFF00) >> 8);
    W25x20CL_Spi(Page_Address & 0xFF);
    HAL_SPI_Transmit(&_W25x20CL_SPI, pBuffer, NumByteToWrite_up_to_PageSize, 100);
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    W25x20CL_WaitForWriteEnd();
#if (_W25x20CL_DEBUG==1)
    StartTime = HAL_GetTick() - StartTime;
    for(uint32_t i = 0; i < NumByteToWrite_up_to_PageSize ; i++)
    {
        if((i % 8 == 0) && (i > 2))
        {
            printf("\r\n");
            HAL_Delay(10);
        }
        printf("0x%02X,", pBuffer[i]);
    }
    printf("\r\n");
    printf("xW25x20CL WritePage done after %d ms\r\n", StartTime);
    HAL_Delay(100);
#endif
    HAL_Delay(1);
    xW25x20CL.Lock = 0;
}
//

void  W25x20CL_WriteBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte,
                          uint32_t NumByteToWrite_up_to_BlockSize)
{
    if((NumByteToWrite_up_to_BlockSize > xW25x20CL.BlockSize) || (NumByteToWrite_up_to_BlockSize == 0))
    {
        NumByteToWrite_up_to_BlockSize = xW25x20CL.BlockSize;
    }
#if (_W25x20CL_DEBUG==1)
    printf("+++xW25x20CL WriteBlock:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Block_Address, OffsetInByte,
           NumByteToWrite_up_to_BlockSize);
    HAL_Delay(100);
#endif
    if(OffsetInByte >= xW25x20CL.BlockSize)
    {
#if (_W25x20CL_DEBUG==1)
        printf("---xW25x20CL WriteBlock Faild!\r\n");
        HAL_Delay(100);
#endif
        return;
    }
    uint32_t  StartPage;
    int32_t   BytesToWrite;
    uint32_t  LocalOffset;
    if((OffsetInByte + NumByteToWrite_up_to_BlockSize) > xW25x20CL.BlockSize)
    {
        BytesToWrite = xW25x20CL.BlockSize - OffsetInByte;
    }
    else
    {
        BytesToWrite = NumByteToWrite_up_to_BlockSize;
    }
    StartPage = W25x20CL_BlockToPage(Block_Address) + (OffsetInByte / xW25x20CL.PageSize);
    LocalOffset = OffsetInByte % xW25x20CL.PageSize;
    do
    {
        W25x20CL_WritePage(pBuffer, StartPage, LocalOffset, BytesToWrite);
        StartPage++;
        BytesToWrite -= xW25x20CL.PageSize - LocalOffset;
        pBuffer += xW25x20CL.PageSize - LocalOffset;
        LocalOffset = 0;
    }
    while(BytesToWrite > 0);
#if (_W25x20CL_DEBUG==1)
    printf("---xW25x20CL WriteBlock Done\r\n");
    HAL_Delay(100);
#endif
}
//

void  W25x20CL_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte,
                           uint32_t NumByteToWrite_up_to_SectorSize)
{
    if((NumByteToWrite_up_to_SectorSize > xW25x20CL.SectorSize) || (NumByteToWrite_up_to_SectorSize == 0))
    {
        NumByteToWrite_up_to_SectorSize = xW25x20CL.SectorSize;
    }
#if (_W25x20CL_DEBUG==1)
    printf("+++xW25x20CL WriteSector:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Sector_Address, OffsetInByte,
           NumByteToWrite_up_to_SectorSize);
    HAL_Delay(100);
#endif
    if(OffsetInByte >= xW25x20CL.SectorSize)
    {
#if (_W25x20CL_DEBUG==1)
        printf("---xW25x20CL WriteSector Faild!\r\n");
        HAL_Delay(100);
#endif
        return;
    }
    uint32_t  StartPage;
    int32_t   BytesToWrite;
    uint32_t  LocalOffset;
    if((OffsetInByte + NumByteToWrite_up_to_SectorSize) > xW25x20CL.SectorSize)
    {
        BytesToWrite = xW25x20CL.SectorSize - OffsetInByte;
    }
    else
    {
        BytesToWrite = NumByteToWrite_up_to_SectorSize;
    }
    StartPage = W25x20CL_SectorToPage(Sector_Address) + (OffsetInByte / xW25x20CL.PageSize);
    LocalOffset = OffsetInByte % xW25x20CL.PageSize;
    do
    {
        W25x20CL_WritePage(pBuffer, StartPage, LocalOffset, BytesToWrite);
        StartPage++;
        BytesToWrite -= xW25x20CL.PageSize - LocalOffset;
        pBuffer += xW25x20CL.PageSize - LocalOffset;
        LocalOffset = 0;
    }
    while(BytesToWrite > 0);
#if (_W25x20CL_DEBUG==1)
    printf("---xW25x20CL WriteSector Done\r\n");
    HAL_Delay(100);
#endif
}
//

void  W25x20CL_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address)
{
    while(xW25x20CL.Lock == 1)
    {
        HAL_Delay(1);
    }
    xW25x20CL.Lock = 1;
#if (_W25x20CL_DEBUG==1)
    uint32_t  StartTime = HAL_GetTick();
    printf("xW25x20CL ReadByte at address %d begin...\r\n", Bytes_Address);
#endif
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    W25x20CL_Spi(0x0B);
    if(xW25x20CL.ID >= W25Q256)
    {
        W25x20CL_Spi((Bytes_Address & 0xFF000000) >> 24);
    }
    W25x20CL_Spi((Bytes_Address & 0xFF0000) >> 16);
    W25x20CL_Spi((Bytes_Address & 0xFF00) >> 8);
    W25x20CL_Spi(Bytes_Address & 0xFF);
    W25x20CL_Spi(0);
    *pBuffer = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL ReadByte 0x%02X done after %d ms\r\n", *pBuffer, HAL_GetTick() - StartTime);
#endif
    xW25x20CL.Lock = 0;
}
//

void W25x20CL_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
    while(xW25x20CL.Lock == 1)
    {
        HAL_Delay(1);
    }
    xW25x20CL.Lock = 1;
#if (_W25x20CL_DEBUG==1)
    uint32_t  StartTime = HAL_GetTick();
    printf("xW25x20CL ReadBytes at Address:%d, %d Bytes  begin...\r\n", ReadAddr, NumByteToRead);
#endif
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    W25x20CL_Spi(0x0B);
    if(xW25x20CL.ID >= W25Q256)
    {
        W25x20CL_Spi((ReadAddr & 0xFF000000) >> 24);
    }
    W25x20CL_Spi((ReadAddr & 0xFF0000) >> 16);
    W25x20CL_Spi((ReadAddr & 0xFF00) >> 8);
    W25x20CL_Spi(ReadAddr & 0xFF);
    W25x20CL_Spi(0);
    HAL_SPI_Receive(&_W25x20CL_SPI, pBuffer, NumByteToRead, 2000);
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
#if (_W25x20CL_DEBUG==1)
    StartTime = HAL_GetTick() - StartTime;
    for(uint32_t i = 0; i < NumByteToRead ; i++)
    {
        if((i % 8 == 0) && (i > 2))
        {
            printf("\r\n");
            HAL_Delay(10);
        }
        printf("0x%02X,", pBuffer[i]);
    }
    printf("\r\n");
    printf("xW25x20CL ReadBytes done after %d ms\r\n", StartTime);
    HAL_Delay(100);
#endif
    HAL_Delay(1);
    xW25x20CL.Lock = 0;
}
//

void  W25x20CL_ReadPage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte,
                        uint32_t NumByteToRead_up_to_PageSize)
{
    while(xW25x20CL.Lock == 1)
    {
        HAL_Delay(1);
    }
    xW25x20CL.Lock = 1;
    if((NumByteToRead_up_to_PageSize > xW25x20CL.PageSize) || (NumByteToRead_up_to_PageSize == 0))
    {
        NumByteToRead_up_to_PageSize = xW25x20CL.PageSize;
    }
    if((OffsetInByte + NumByteToRead_up_to_PageSize) > xW25x20CL.PageSize)
    {
        NumByteToRead_up_to_PageSize = xW25x20CL.PageSize - OffsetInByte;
    }
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL ReadPage:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Page_Address, OffsetInByte,
           NumByteToRead_up_to_PageSize);
    HAL_Delay(100);
    uint32_t  StartTime = HAL_GetTick();
#endif
    Page_Address = Page_Address * xW25x20CL.PageSize + OffsetInByte;
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    W25x20CL_Spi(0x0B);
    if(xW25x20CL.ID >= W25Q256)
    {
        W25x20CL_Spi((Page_Address & 0xFF000000) >> 24);
    }
    W25x20CL_Spi((Page_Address & 0xFF0000) >> 16);
    W25x20CL_Spi((Page_Address & 0xFF00) >> 8);
    W25x20CL_Spi(Page_Address & 0xFF);
    W25x20CL_Spi(0);
    HAL_SPI_Receive(&_W25x20CL_SPI, pBuffer, NumByteToRead_up_to_PageSize, 100);
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
#if (_W25x20CL_DEBUG==1)
    StartTime = HAL_GetTick() - StartTime;
    for(uint32_t i = 0; i < NumByteToRead_up_to_PageSize ; i++)
    {
        if((i % 8 == 0) && (i > 2))
        {
            printf("\r\n");
            HAL_Delay(10);
        }
        printf("0x%02X,", pBuffer[i]);
    }
    printf("\r\n");
    printf("xW25x20CL ReadPage done after %d ms\r\n", StartTime);
    HAL_Delay(100);
#endif
    HAL_Delay(1);
    xW25x20CL.Lock = 0;
}
//

void  W25x20CL_ReadBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte,
                         uint32_t NumByteToRead_up_to_BlockSize)
{
    if((NumByteToRead_up_to_BlockSize > xW25x20CL.BlockSize) || (NumByteToRead_up_to_BlockSize == 0))
    {
        NumByteToRead_up_to_BlockSize = xW25x20CL.BlockSize;
    }
#if (_W25x20CL_DEBUG==1)
    printf("+++xW25x20CL ReadBlock:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Block_Address, OffsetInByte,
           NumByteToRead_up_to_BlockSize);
    HAL_Delay(100);
#endif
    if(OffsetInByte >= xW25x20CL.BlockSize)
    {
#if (_W25x20CL_DEBUG==1)
        printf("xW25x20CL ReadBlock Faild!\r\n");
        HAL_Delay(100);
#endif
        return;
    }
    uint32_t  StartPage;
    int32_t   BytesToRead;
    uint32_t  LocalOffset;
    if((OffsetInByte + NumByteToRead_up_to_BlockSize) > xW25x20CL.BlockSize)
    {
        BytesToRead = xW25x20CL.BlockSize - OffsetInByte;
    }
    else
    {
        BytesToRead = NumByteToRead_up_to_BlockSize;
    }
    StartPage = W25x20CL_BlockToPage(Block_Address) + (OffsetInByte / xW25x20CL.PageSize);
    LocalOffset = OffsetInByte % xW25x20CL.PageSize;
    do
    {
        W25x20CL_ReadPage(pBuffer, StartPage, LocalOffset, BytesToRead);
        StartPage++;
        BytesToRead -= xW25x20CL.PageSize - LocalOffset;
        pBuffer += xW25x20CL.PageSize - LocalOffset;
        LocalOffset = 0;
    }
    while(BytesToRead > 0);
#if (_W25x20CL_DEBUG==1)
    printf("---xW25x20CL ReadBlock Done\r\n");
    HAL_Delay(100);
#endif
}
//

void  W25x20CL_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte,
                          uint32_t NumByteToRead_up_to_SectorSize)
{
    if((NumByteToRead_up_to_SectorSize > xW25x20CL.SectorSize) || (NumByteToRead_up_to_SectorSize == 0))
    {
        NumByteToRead_up_to_SectorSize = xW25x20CL.SectorSize;
    }
#if (_W25x20CL_DEBUG==1)
    printf("+++xW25x20CL ReadSector:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Sector_Address, OffsetInByte,
           NumByteToRead_up_to_SectorSize);
    HAL_Delay(100);
#endif
    if(OffsetInByte >= xW25x20CL.SectorSize)
    {
#if (_W25x20CL_DEBUG==1)
        printf("---xW25x20CL ReadSector Faild!\r\n");
        HAL_Delay(100);
#endif
        return;
    }
    uint32_t  StartPage;
    int32_t   BytesToRead;
    uint32_t  LocalOffset;
    if((OffsetInByte + NumByteToRead_up_to_SectorSize) > xW25x20CL.SectorSize)
    {
        BytesToRead = xW25x20CL.SectorSize - OffsetInByte;
    }
    else
    {
        BytesToRead = NumByteToRead_up_to_SectorSize;
    }
    StartPage = W25x20CL_SectorToPage(Sector_Address) + (OffsetInByte / xW25x20CL.PageSize);
    LocalOffset = OffsetInByte % xW25x20CL.PageSize;
    do
    {
        W25x20CL_ReadPage(pBuffer, StartPage, LocalOffset, BytesToRead);
        StartPage++;
        BytesToRead -= xW25x20CL.PageSize - LocalOffset;
        pBuffer += xW25x20CL.PageSize - LocalOffset;
        LocalOffset = 0;
    }
    while(BytesToRead > 0);
#if (_W25x20CL_DEBUG==1)
    printf("---xW25x20CL ReadSector Done\r\n");
    HAL_Delay(100);
#endif
}
//

uint32_t  W25x20CL_PageToBlock(uint32_t PageAddress)
{
    return ((PageAddress * xW25x20CL.PageSize) / xW25x20CL.BlockSize);
}
//

uint32_t  W25x20CL_BlockToPage(uint32_t BlockAddress)
{
    return (BlockAddress * xW25x20CL.BlockSize) / xW25x20CL.PageSize;
}
//

uint32_t  W25x20CL_PageToSector(uint32_t  PageAddress)
{
    return ((PageAddress * xW25x20CL.PageSize) / xW25x20CL.SectorSize);
}
//

uint32_t  W25x20CL_SectorToBlock(uint32_t SectorAddress)
{
    return ((SectorAddress * xW25x20CL.SectorSize) / xW25x20CL.BlockSize);
}
//

uint32_t  W25x20CL_SectorToPage(uint32_t  SectorAddress)
{
    return (SectorAddress * xW25x20CL.SectorSize) / xW25x20CL.PageSize;
}
//

uint8_t W25x20CL_ReadStatusRegister(uint8_t SelectStatusRegister)
{
    uint8_t status = 0;
    LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    if(SelectStatusRegister_1_2_3 == 1)
    {
        W25x20CL_Spi(0x05);
        status = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
        xW25x20CL.StatusRegister1 = status;
    }
    else if(SelectStatusRegister_1_2_3 == 2)
    {
        W25x20CL_Spi(0x35);
        status = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
        xW25x20CL.StatusRegister2 = status;
    }
    else
    {
        W25x20CL_Spi(0x15);
        status = W25x20CL_Spi(W25x20CL_DUMMY_BYTE);
        xW25x20CL.StatusRegister3 = status;
    }
    LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
    return status;
}
//

bool  W25x20CL_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize)
{
    while(xW25x20CL.Lock == 1)
    {
        HAL_Delay(1);
    }
    xW25x20CL.Lock = 1;
    if(((NumByteToCheck_up_to_PageSize + OffsetInByte) > xW25x20CL.PageSize) || (NumByteToCheck_up_to_PageSize == 0))
    {
        NumByteToCheck_up_to_PageSize = xW25x20CL.PageSize - OffsetInByte;
    }
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckPage:%d, Offset:%d, Bytes:%d begin...\r\n", Page_Address, OffsetInByte,
           NumByteToCheck_up_to_PageSize);
    HAL_Delay(100);
    uint32_t  StartTime = HAL_GetTick();
#endif
    uint8_t pBuffer[32];
    uint32_t  WorkAddress;
    uint32_t  i;
    for(i = OffsetInByte; i < xW25x20CL.PageSize; i += sizeof(pBuffer))
    {
        LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
        WorkAddress = (i + Page_Address * xW25x20CL.PageSize);
        W25x20CL_Spi(0x0B);
        if(xW25x20CL.ID >= W25Q256)
        {
            W25x20CL_Spi((WorkAddress & 0xFF000000) >> 24);
        }
        W25x20CL_Spi((WorkAddress & 0xFF0000) >> 16);
        W25x20CL_Spi((WorkAddress & 0xFF00) >> 8);
        W25x20CL_Spi(WorkAddress & 0xFF);
        W25x20CL_Spi(0);
        HAL_SPI_Receive(&_W25x20CL_SPI, pBuffer, sizeof(pBuffer), 100);
        LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
        for(uint8_t x = 0; x < sizeof(pBuffer); x++)
        {
            if(pBuffer[x] != 0xFF)
            {
                goto NOT_EMPTY;
            }
        }
    }
    if((xW25x20CL.PageSize + OffsetInByte) % sizeof(pBuffer) != 0)
    {
        i -= sizeof(pBuffer);
        for(; i < xW25x20CL.PageSize; i++)
        {
            LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
            WorkAddress = (i + Page_Address * xW25x20CL.PageSize);
            W25x20CL_Spi(0x0B);
            if(xW25x20CL.ID >= W25Q256)
            {
                W25x20CL_Spi((WorkAddress & 0xFF000000) >> 24);
            }
            W25x20CL_Spi((WorkAddress & 0xFF0000) >> 16);
            W25x20CL_Spi((WorkAddress & 0xFF00) >> 8);
            W25x20CL_Spi(WorkAddress & 0xFF);
            W25x20CL_Spi(0);
            HAL_SPI_Receive(&_W25x20CL_SPI, pBuffer, 1, 100);
            LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
            if(pBuffer[0] != 0xFF)
            {
                goto NOT_EMPTY;
            }
        }
    }
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckPage is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    HAL_Delay(100);
#endif
    xW25x20CL.Lock = 0;
    return true;
NOT_EMPTY:
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckPage is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    HAL_Delay(100);
#endif
    xW25x20CL.Lock = 0;
    return false;
}
//

bool  W25x20CL_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize)
{
    while(xW25x20CL.Lock == 1)
    {
        HAL_Delay(1);
    }
    xW25x20CL.Lock = 1;
    if((NumByteToCheck_up_to_BlockSize > xW25x20CL.BlockSize) || (NumByteToCheck_up_to_BlockSize == 0))
    {
        NumByteToCheck_up_to_BlockSize = xW25x20CL.BlockSize;
    }
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckBlock:%d, Offset:%d, Bytes:%d begin...\r\n", Block_Address, OffsetInByte,
           NumByteToCheck_up_to_BlockSize);
    HAL_Delay(100);
    uint32_t  StartTime = HAL_GetTick();
#endif
    uint8_t pBuffer[32];
    uint32_t  WorkAddress;
    uint32_t  i;
    for(i = OffsetInByte; i < xW25x20CL.BlockSize; i += sizeof(pBuffer))
    {
        LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
        WorkAddress = (i + Block_Address * xW25x20CL.BlockSize);
        W25x20CL_Spi(0x0B);
        if(xW25x20CL.ID >= W25Q256)
        {
            W25x20CL_Spi((WorkAddress & 0xFF000000) >> 24);
        }
        W25x20CL_Spi((WorkAddress & 0xFF0000) >> 16);
        W25x20CL_Spi((WorkAddress & 0xFF00) >> 8);
        W25x20CL_Spi(WorkAddress & 0xFF);
        W25x20CL_Spi(0);
        HAL_SPI_Receive(&_W25x20CL_SPI, pBuffer, sizeof(pBuffer), 100);
        LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
        for(uint8_t x = 0; x < sizeof(pBuffer); x++)
        {
            if(pBuffer[x] != 0xFF)
            {
                goto NOT_EMPTY;
            }
        }
    }
    if((xW25x20CL.BlockSize + OffsetInByte) % sizeof(pBuffer) != 0)
    {
        i -= sizeof(pBuffer);
        for(; i < xW25x20CL.BlockSize; i++)
        {
            LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
            WorkAddress = (i + Block_Address * xW25x20CL.BlockSize);
            W25x20CL_Spi(0x0B);
            if(xW25x20CL.ID >= W25Q256)
            {
                W25x20CL_Spi((WorkAddress & 0xFF000000) >> 24);
            }
            W25x20CL_Spi((WorkAddress & 0xFF0000) >> 16);
            W25x20CL_Spi((WorkAddress & 0xFF00) >> 8);
            W25x20CL_Spi(WorkAddress & 0xFF);
            W25x20CL_Spi(0);
            HAL_SPI_Receive(&_W25x20CL_SPI, pBuffer, 1, 100);
            LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
            if(pBuffer[0] != 0xFF)
            {
                goto NOT_EMPTY;
            }
        }
    }
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckBlock is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    HAL_Delay(100);
#endif
    xW25x20CL.Lock = 0;
    return true;
NOT_EMPTY:
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckBlock is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    HAL_Delay(100);
#endif
    xW25x20CL.Lock = 0;
    return false;
}
//

bool  W25x20CL_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize)
{
    while(xW25x20CL.Lock == 1)
    {
        HAL_Delay(1);
    }
    xW25x20CL.Lock = 1;
    if((NumByteToCheck_up_to_SectorSize > xW25x20CL.SectorSize) || (NumByteToCheck_up_to_SectorSize == 0))
    {
        NumByteToCheck_up_to_SectorSize = xW25x20CL.SectorSize;
    }
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckSector:%d, Offset:%d, Bytes:%d begin...\r\n", Sector_Address, OffsetInByte,
           NumByteToCheck_up_to_SectorSize);
    HAL_Delay(100);
    uint32_t  StartTime = HAL_GetTick();
#endif
    uint8_t pBuffer[32];
    uint32_t  WorkAddress;
    uint32_t  i;
    for(i = OffsetInByte; i < xW25x20CL.SectorSize; i += sizeof(pBuffer))
    {
        LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
        WorkAddress = (i + Sector_Address * xW25x20CL.SectorSize);
        W25x20CL_Spi(0x0B);
        if(xW25x20CL.ID >= W25Q256)
        {
            W25x20CL_Spi((WorkAddress & 0xFF000000) >> 24);
        }
        W25x20CL_Spi((WorkAddress & 0xFF0000) >> 16);
        W25x20CL_Spi((WorkAddress & 0xFF00) >> 8);
        W25x20CL_Spi(WorkAddress & 0xFF);
        W25x20CL_Spi(0);
        HAL_SPI_Receive(&_W25x20CL_SPI, pBuffer, sizeof(pBuffer), 100);
        LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
        for(uint8_t x = 0; x < sizeof(pBuffer); x++)
        {
            if(pBuffer[x] != 0xFF)
            {
                goto NOT_EMPTY;
            }
        }
    }
    if((xW25x20CL.SectorSize + OffsetInByte) % sizeof(pBuffer) != 0)
    {
        i -= sizeof(pBuffer);
        for(; i < xW25x20CL.SectorSize; i++)
        {
            LL_GPIO_ResetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
            WorkAddress = (i + Sector_Address * xW25x20CL.SectorSize);
            W25x20CL_Spi(0x0B);
            if(xW25x20CL.ID >= W25Q256)
            {
                W25x20CL_Spi((WorkAddress & 0xFF000000) >> 24);
            }
            W25x20CL_Spi((WorkAddress & 0xFF0000) >> 16);
            W25x20CL_Spi((WorkAddress & 0xFF00) >> 8);
            W25x20CL_Spi(WorkAddress & 0xFF);
            W25x20CL_Spi(0);
            HAL_SPI_Receive(&_W25x20CL_SPI, pBuffer, 1, 100);
            LL_GPIO_SetOutputPin(FLASH_NSS_GPIO_Port, FLASH_NSS_Pin);
            if(pBuffer[0] != 0xFF)
            {
                goto NOT_EMPTY;
            }
        }
    }
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckSector is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    HAL_Delay(100);
#endif
    xW25x20CL.Lock = 0;
    return true;
NOT_EMPTY:
#if (_W25x20CL_DEBUG==1)
    printf("xW25x20CL CheckSector is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    HAL_Delay(100);
#endif
    xW25x20CL.Lock = 0;
    return false;
}
//

/************************ (C) COPYRIGHT YiLDiZ IoT END OF FILE ************************/
