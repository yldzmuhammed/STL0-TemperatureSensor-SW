/**
  ******************************************************************************
  * @file    w25x20cl.h
  * @author  yldzmuhammed
  * @version V1.0
  * @date    29-07-2020
  * @brief   This file contains the common defines and functions prototypes for
  *          the w25x20cl.h driver.
  ******************************************************************************
  * @attention
  *
  * experiment platform: YiLDiZ IoT STl0-UTMP SPI Flash Library
  * web-site: https://www.yildiziot.com
  ******************************************************************************
*/

#ifndef __W25X20CL__H
#define __W25X20CL__H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32l0xx.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    eStatusReg_1 = 0x01,
    eStatusReg_2 = 0x31,
    eStatusReg_3 = 0x11
}eStatusReg_TypeDef;

typedef enum
{
  W25Q10=1,
  W25Q20,
  W25Q40,
  W25Q80,
  W25Q16,
  W25Q32,
  W25Q64,
  W25Q128,
  W25Q256,
  W25Q512,
}eW25x20CLID_TypeDef;
//

typedef struct
{
	eW25x20CLID_TypeDef  ID;
	
  uint8_t		            UniqID[8];

	uint8_t		            Lock;

	uint8_t		            StatusRegister1;
	uint8_t		            StatusRegister2;
	uint8_t		            StatusRegister3;	

	uint16_t	            PageSize;
	uint32_t	            PageCount;
	uint32_t	            BlockSize;
	uint32_t	            SectorSize;
	uint32_t	            BlockCount;
	uint32_t	            SectorCount;
	uint32_t	            CapacityInKiloByte;
}xW25x20CL_TypeDef;
//

/* Private define ------------------------------------------------------------*/
#define FLASH_WP_GPIO_Port              GPIOA
#define FLASH_NSS_GPIO_Port             GPIOA
#define FLASH_SCK_GPIO_Port             GPIOA
#define FLASH_MISO_GPIO_Port            GPIOA
#define FLASH_MOSI_GPIO_Port            GPIOA
#define FLASH_HOLD_GPIO_Port            GPIOB
#define FLASH_PWREN_GPIO_Port           GPIOB

#define FLASH_WP_Pin                    LL_GPIO_PIN_3
#define FLASH_NSS_Pin                   LL_GPIO_PIN_4
#define FLASH_SCK_Pin                   LL_GPIO_PIN_5
#define FLASH_MISO_Pin                  LL_GPIO_PIN_6
#define FLASH_MOSI_Pin                  LL_GPIO_PIN_7
#define FLASH_HOLD_Pin                  LL_GPIO_PIN_0
#define FLASH_PWREN_Pin                 LL_GPIO_PIN_1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Public user function ------------------------------------------------------*/

extern xW25x20CL_TypeDef	xW25x20CL;

bool        W25x20CL_Init(void);

uint32_t    W25x20CL_ReadID(void);
void        W25x20CL_ReadUniqID(void);


void        W25x20CL_EraseChip(void);
void        W25x20CL_EraseBlock(uint32_t BlockAddr);
void        W25x20CL_EraseSector(uint32_t SectorAddr);

void        W25x20CL_WriteByte(uint8_t pBuffer, uint32_t Bytes_Address);
void        W25x20CL_WritePage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);
void        W25x20CL_WriteBlock(uint8_t* pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);
void        W25x20CL_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize);

uint32_t    W25x20CL_PageToBlock(uint32_t PageAddress);
uint32_t    W25x20CL_PageToSector(uint32_t PageAddress);
uint32_t    W25x20CL_BlockToPage(uint32_t BlockAddress);
uint32_t    W25x20CL_SectorToPage(uint32_t SectorAddress);
uint32_t    W25x20CL_SectorToBlock(uint32_t SectorAddress);

bool        W25x20CL_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
bool        W25x20CL_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);
bool        W25x20CL_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);

void        W25x20CL_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address);
void        W25x20CL_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
void        W25x20CL_ReadPage(uint8_t *pBuffer, uint32_t Page_Address,uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);
void        W25x20CL_ReadBlock(uint8_t* pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);
void        W25x20CL_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address,uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);

#ifdef __cplusplus
}
#endif

#endif

/************************ (C) COPYRIGHT YiLDiZ IoT END OF FILE ************************/
