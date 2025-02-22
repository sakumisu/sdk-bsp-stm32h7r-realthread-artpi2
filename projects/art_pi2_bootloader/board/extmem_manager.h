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

#ifndef __EXTMEM_MANAGER_H__
#define __EXTMEM_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

void EXTMEM_Init(void);
rt_int32_t EXTMEM_Flash_Probe(void);
rt_int32_t EXTMEM_Flash_EnterXIP(void);
rt_int32_t EXTMEM_PSRAM_Probe(void);
rt_int32_t EXTMEM_PSRAM_EnterXIP(void);

#ifdef __cplusplus
}
#endif

#endif  // __EXTMEM_MANAGER_H__

