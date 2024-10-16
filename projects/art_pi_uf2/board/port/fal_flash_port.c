/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <rtthread.h>
#include <fal.h>
#include <w35t51nwtbie.h>

#ifdef BSP_USING_XSPI_NORFLASH

#define DBG_TAG "norflash"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifndef FAL_USING_NOR_FLASH_DEV_NAME
#define FAL_USING_NOR_FLASH_DEV_NAME             "norflash0"
#endif

static int init(void);
static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);
static int erase(long offset, size_t size);

XSPI_HandleTypeDef hxspi2;

static uint32_t norflash_init_flag = 0;
struct fal_flash_dev nor_flash0 =
{
    .name       = FAL_USING_NOR_FLASH_DEV_NAME,
    .addr       = 0,
    .len        = 64 * 1024 * 1024,
    .blk_size   = 4096,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};

/**
  * @brief XSPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_XSPI2_Init(void)
{

  XSPIM_CfgTypeDef sXspiManagerCfg = {0};

  /* USER CODE BEGIN XSPI2_Init 1 */
  
  /* USER CODE END XSPI2_Init 1 */
  /* XSPI2 parameter configuration*/
  hxspi2.Instance = XSPI2;
  hxspi2.Init.FifoThresholdByte = 4;
  hxspi2.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
  hxspi2.Init.MemoryType = HAL_XSPI_MEMTYPE_MICRON;
  hxspi2.Init.MemorySize = HAL_XSPI_SIZE_1GB;
  hxspi2.Init.ChipSelectHighTimeCycle = 1;
  hxspi2.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi2.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
  hxspi2.Init.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hxspi2.Init.ClockPrescaler = 1;
  hxspi2.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi2.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE;
  hxspi2.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE;
  hxspi2.Init.MaxTran = 0;
  hxspi2.Init.Refresh = 0;
  hxspi2.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;
  if (HAL_XSPI_Init(&hxspi2) != HAL_OK)
  {
    Error_Handler();
  }
  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.IOPort = HAL_XSPIM_IOPORT_2;
  if (HAL_XSPIM_Config(&hxspi2, &sXspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN XSPI2_Init 2 */
  
  /* USER CODE END XSPI2_Init 2 */

}

static int init(void)
{
    uint8_t device_id[6];

    MX_XSPI2_Init();

    if(W35T51NWTBIE_OK == W35T51NWTBIE_ReadID(&hxspi2, 
                                            W35T51NWTBIE_SPI_MODE, 
                                            W35T51NWTBIE_STR_TRANSFER, 
                                            device_id , W35T51NWTBIE_3BYTES_SIZE))
    {
        norflash_init_flag = 1;
        LOG_I("Flash ID: %.2x %.2x %.2x", device_id[0], device_id[1], device_id[2]);
    }
    else
    {
        LOG_E("Read ID Fail");
        return -1;
    }
    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
//    assert(sfud_dev);
    assert(norflash_init_flag);

    if(W35T51NWTBIE_OK == W35T51NWTBIE_ReadSTR(&hxspi2,
                                               W35T51NWTBIE_SPI_MODE,
                                               W35T51NWTBIE_3BYTES_SIZE,
                                               buf,
                                               nor_flash0.addr + offset, size))
    {
        LOG_D("Flash Read adr:%.2x, size:%.2x", (nor_flash0.addr + offset), size);
        return size;
    }
    LOG_E("Flash Read Err");
    return -1;
}

// do a >256 byte continue write.
// must align to 256 bytes
static int write(long offset, const uint8_t *buf, size_t size)
{
    uint32_t data_size=0;
    int32_t  retr=0;
    uint32_t addr = (uint32_t)offset;
    uint32_t write_gran = 256;

    assert(norflash_init_flag);

    /* check the flash address boundary */
    if (addr + size > nor_flash0.len) {
        LOG_E("Error: Flash address is out of boundary.");
        return -1;
    }
    /* @TODO lock to write */
    /* loop write operate. write unit is write granularity */
    while (size) 
    {
        /* set the flash write enable */
        // result = set_write_enabled(flash, true);

        if (W35T51NWTBIE_OK != W35T51NWTBIE_WriteEnable(&hxspi2, W35T51NWTBIE_SPI_MODE, W35T51NWTBIE_STR_TRANSFER))
        {
            retr = -2;
            goto __exit;
        }

        /* make write align and calculate next write address */
        if (addr % write_gran != 0) {
            if (size > write_gran - (addr % write_gran)) {
                data_size = write_gran - (addr % write_gran);
            } else {
                data_size = size;
            }
        } else {
            if (size > write_gran) {
                data_size = write_gran;
            } else {
                data_size = size;
            }
        }

        if(W35T51NWTBIE_OK != W35T51NWTBIE_PageProgram(&hxspi2,
                                                    W35T51NWTBIE_SPI_MODE,
                                                    W35T51NWTBIE_3BYTES_SIZE,
                                                    (uint8_t*)buf,
                                                    addr, data_size))
        {
            retr = -3;
            LOG_E("Flash Write Err, code: %d", retr);
            goto __exit;
        }

        LOG_D("Flash Write adr:%.2x, size:%.2x", addr, data_size);

        if(W35T51NWTBIE_OK != W35T51NWTBIE_AutoPollingMemReady(&hxspi2, W35T51NWTBIE_SPI_MODE,W35T51NWTBIE_STR_TRANSFER))
        {
            retr = -4;
            LOG_E("Flash Write Err, code: %d", retr);
            goto __exit;
        }
        size -= data_size;
        addr += data_size;
        buf += data_size;
    }

__exit:
    /* @TODO set the flash write disable */
    /* @TODO unlock write */
    return retr;

}

//do a 4K aligned erase
//@TODO grain control to 4K only
static int erase(long offset, size_t size)
{
    uint32_t start_addr = offset;
    uint32_t end_addr = offset + size;
    uint32_t SectorSize = 4096;
    int32_t  retr=0;
//    assert(sfud_dev);
//    assert(sfud_dev->init_ok);
    do
    {
        if (W35T51NWTBIE_OK == W35T51NWTBIE_WriteEnable(&hxspi2,
                                                        W35T51NWTBIE_SPI_MODE,
                                                        W35T51NWTBIE_STR_TRANSFER))
        {
            if(W35T51NWTBIE_OK == W35T51NWTBIE_BlockErase(&hxspi2,
                                                        W35T51NWTBIE_SPI_MODE,
                                                        W35T51NWTBIE_STR_TRANSFER,
                                                        W35T51NWTBIE_3BYTES_SIZE,
                                                        nor_flash0.addr + start_addr,
                                                        W35T51NWTBIE_ERASE_4K))
            {
                if (W35T51NWTBIE_OK != W35T51NWTBIE_AutoPollingMemReady(&hxspi2,
                                                                        W35T51NWTBIE_SPI_MODE,
                                                                        W35T51NWTBIE_STR_TRANSFER))
                {
                    retr = -3;
                    break;
                }
            }
            else
            {
                retr = -2;
                break;
            }
        }
        else
        {
            retr = -1;
            break;
        }
        start_addr = start_addr + SectorSize;
        LOG_D("Flash Erase adr:%.2x, size:%.2x", (nor_flash0.addr + start_addr), SectorSize);
    }while ((end_addr > start_addr) && (retr == 0));
    
    if(retr !=0 )
    {
        LOG_E("Flash Erase Err, code: %d", retr);
        return -1;
    }
    return size;
}
#endif /* BSP_USING_XSPI_NORFLASH */
