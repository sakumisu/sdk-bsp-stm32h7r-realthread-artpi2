/**
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-21     stackYuan    first version
 * 
 */

#include <rtthread.h>
#include "w35t51nwtbie.h"
#include "aps256xx.h"
#include "main.h"

#define DBG_ENABLE
#define DBG_TAG "XSPI"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

XSPI_HandleTypeDef hxspi1;
XSPI_HandleTypeDef hxspi2;
XSPI_HSCalTypeDef cal = {0};

/**
 * @brief XSPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_XSPI1_Init(void) // PSRAM
{

    XSPIM_CfgTypeDef sXspiManagerCfg = {0};
    uint32_t xspi1_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI1);

    /* XSPI1 parameter configuration*/
    hxspi1.Instance = XSPI1;
    hxspi1.Init.FifoThresholdByte = 2;
    hxspi1.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
    hxspi1.Init.MemoryType = HAL_XSPI_MEMTYPE_APMEM_16BITS;
    hxspi1.Init.MemorySize = HAL_XSPI_SIZE_32GB;
    hxspi1.Init.ChipSelectHighTimeCycle = 1;
    hxspi1.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
    hxspi1.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
    hxspi1.Init.WrapSize = HAL_XSPI_WRAP_32_BYTES;
    hxspi1.Init.ClockPrescaler = 0;
    hxspi1.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;  // HAL_XSPI_SAMPLE_SHIFT_HALFCYCLE; HAL_XSPI_SAMPLE_SHIFT_NONE
    hxspi1.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_ENABLE; //  HAL_XSPI_DHQC_ENABLE HAL_XSPI_DHQC_DISABLE
    hxspi1.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_8KB;
    hxspi1.Init.MaxTran = 0;
    hxspi1.Init.Refresh = ((2U * (xspi1_clk / (hxspi1.Init.ClockPrescaler + 1U))) / 1000000U) - 4U;
    hxspi1.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;
    if (HAL_XSPI_Init(&hxspi1) != HAL_OK)
    {
        Error_Handler();
    }
    sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
    sXspiManagerCfg.IOPort = HAL_XSPIM_IOPORT_1;
    if (HAL_XSPIM_Config(&hxspi1, &sXspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief XSPI2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_XSPI2_Init(void) // Octal Flash
{

    XSPIM_CfgTypeDef sXspiManagerCfg = {0};

    /* XSPI2 parameter configuration*/
    hxspi2.Instance = XSPI2;
    hxspi2.Init.FifoThresholdByte = 4;
    hxspi2.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
    hxspi2.Init.MemoryType = HAL_XSPI_MEMTYPE_MICRON;
    hxspi2.Init.MemorySize = HAL_XSPI_SIZE_32GB;
    hxspi2.Init.ChipSelectHighTimeCycle = 2;
    hxspi2.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
    hxspi2.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
    hxspi2.Init.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED;
    hxspi2.Init.ClockPrescaler = 0;
    hxspi2.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
    hxspi2.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_ENABLE;
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
}


void EXTMEM_Init(void)
{
    MX_XSPI1_Init();
    MX_XSPI2_Init();
}

rt_int32_t EXTMEM_Flash_Probe(void)
{
    uint8_t device_id[3];
    int retr = 0;

    if (W35T51NWTBIE_OK != W35T51NWTBIE_ReadID(&hxspi2,
                                               W35T51NWTBIE_SPI_MODE,
                                               W35T51NWTBIE_STR_TRANSFER,
                                               device_id, W35T51NWTBIE_3BYTES_SIZE))
    {
        LOG_E("Read Flash ID Fail");
        return -RT_ERROR;
    }
    else
    {
        LOG_I("Read Flash ID success:0x%.2x 0x%.2x 0x%.2x", device_id[0], device_id[1], device_id[2]);
        
        /* stage 1. enter octal mode and set flash register max speed and other configs */
        retr = W35T51NWTBIE_EnterOctal_DTR_Mode(&hxspi2);
        if (retr != W35T51NWTBIE_OK)
        {
            LOG_E("Flash Enter Octal mode Fail, errno:%d", retr);
            return -RT_ERROR;
        }

        if (HAL_XSPI_DeInit(&hxspi2) != HAL_OK)
        {
            LOG_E("Flash deinit error");
            return -RT_ERROR;
        }

        /* stage 2. reinit and use new configs to achieve maxim speed */
        LOG_I("Reconfigure Flash clock\n");
        MX_XSPI2_Init();

        if (W35T51NWTBIE_OK != W35T51NWTBIE_ReadID(&hxspi2,
                                                   W35T51NWTBIE_OPI_MODE,
                                                   W35T51NWTBIE_DTR_TRANSFER,
                                                   device_id, W35T51NWTBIE_4BYTES_SIZE))
        {
            LOG_E("Flash Re-Enter octal failed");
            return -RT_ERROR;
        }
        else
        {
            LOG_I("Flash Re-Enter Octal DTR and Read ID success:%.2x %.2x %.2x", device_id[0], device_id[1], device_id[2]);
            return RT_EOK;
        }
    }
}

rt_int32_t EXTMEM_Flash_EnterXIP(void)
{
    if (W35T51NWTBIE_OK != W35T51NWTBIE_EnableMemoryMappedModeDTR(&hxspi2, RT_NULL))
    {
        LOG_E("Flash XIP failed");
        return -RT_ERROR;
    }
    else
    {
        LOG_I("Flash XIP success");
        return RT_EOK;
    }
}

/* Configure the 16-bits Octal RAM memory ***************************/

rt_int32_t EXTMEM_PSRAM_Probe(void)
{
    uint8_t reg[2];

    if (APS256XX_OK != APS256XX_Reset(&hxspi1))
    {
        LOG_E("PSRAM Reset error");
        return -RT_ERROR;
    }
    rt_thread_mdelay(20);

    if (APS256XX_OK == APS256XX_ReadReg(&hxspi1, APS256XX_MR0_ADDRESS, reg, 5))
    {
        LOG_I("PSRAM MR0: 0x%.2x, MR1: 0x%.2x", reg[0], reg[1]);
    }

    MODIFY_REG(reg[0], ((uint8_t)APS256XX_MR0_LATENCY_TYPE | (uint8_t)APS256XX_MR0_READ_LATENCY_CODE | (uint8_t)APS256XX_MR0_DRIVE_STRENGTH),
               ((uint8_t)APS256XX_MR0_LATENCY_TYPE_FIXED |
                (uint8_t)APS256XX_MR0_RLC_7 |
                (uint8_t)APS256XX_MR0_DS_FULL));
    if (APS256XX_OK == APS256XX_WriteReg(&hxspi1, APS256XX_MR0_ADDRESS, reg[0]))
    {
        LOG_I("PSRAM MR0 SET");
    }

    MODIFY_REG(reg[0], (uint8_t)(APS256XX_MR4_WRITE_LATENCY_CODE | APS256XX_MR4_RF_RATE | APS256XX_MR4_PASR),
               ((uint8_t)APS256XX_MR4_WLC_7 |
                APS256XX_MR4_RF_4X |
                APS256XX_MR4_PASR_FULL));
    if (APS256XX_OK == APS256XX_WriteReg(&hxspi1, APS256XX_MR4_ADDRESS, reg[0]))
    {
        LOG_I("PSRAM MR4 SET");
    }
    reg[0] = 0;
    MODIFY_REG(reg[0], ((uint8_t)APS256XX_MR8_X8_X16 | (uint8_t)APS256XX_MR8_RBX | (uint8_t)APS256XX_MR8_BT | (uint8_t)APS256XX_MR8_BL),
               ((uint8_t)APS256XX_MR8_X16 |
                (uint8_t)APS256XX_MR8_HYBRID_NONE_BURST_WRAP |
                (uint8_t)APS256XX_MR8_RBX_NOT_SUPPORT |
                (uint8_t)APS256XX_MR8_BL_16_BYTES));
    // reg[0] = 0x40;
    if (APS256XX_OK == APS256XX_WriteReg(&hxspi1, APS256XX_MR8_ADDRESS, reg[0]))
    {
        LOG_I("PSRAM MR8 SET");
    }

    HAL_XSPI_GetDelayValue(&hxspi1, &cal);
    LOG_D("cal delay: 0x%.2x, fine: 0x%.2x, coarse 0x%.2x, max 0x%.2x\n", cal.DelayValueType, cal.FineCalibrationUnit, cal.CoarseCalibrationUnit, cal.MaxCalibration);

    if (APS256XX_OK == APS256XX_ReadReg(&hxspi1, APS256XX_MR0_ADDRESS, reg, 7))
    {
        LOG_D("PSRAM MR0: 0x%.2x, MR1: 0x%.2x\n", reg[0], reg[1]);
    }
    if (APS256XX_OK == APS256XX_ReadReg(&hxspi1, APS256XX_MR4_ADDRESS, reg, 7))
    {
        LOG_D("PSRAM MR4: 0x%.2x, MR5: 0x%.2x\n", reg[0], reg[1]);
    }
    if (APS256XX_OK == APS256XX_ReadReg(&hxspi1, APS256XX_MR8_ADDRESS, reg, 7))
    {
        LOG_D("PSRAM MR8: 0x%.2x, MR1: 0x%.2x\n", reg[0], reg[1]);
    }

    HAL_XSPI_GetDelayValue(&hxspi1, &cal);
    LOG_D("cal delay: 0x%.2x, fine: 0x%.2x, coarse 0x%.2x, max 0x%.2x", cal.DelayValueType, cal.FineCalibrationUnit, cal.CoarseCalibrationUnit, cal.MaxCalibration);
    return RT_EOK;
}

rt_int32_t EXTMEM_PSRAM_EnterXIP(void)
{
    uint32_t ReadLatencyVal = 7;
    uint32_t WriteLatencyVal = 7;

    /*Configure Memory Mapped mode*/
    if (APS256XX_OK != APS256XX_EnableMemoryMappedMode(&hxspi1, ReadLatencyVal, WriteLatencyVal, HAL_XSPI_DATA_16_LINES, 0)) // Liner Burst
    {
        LOG_E("PSRAM XIP Fail\n");
        return -RT_ERROR;
    }
    else
    {
        LOG_I("PSRAM XIP Success\n");
        return RT_EOK;
    }
}