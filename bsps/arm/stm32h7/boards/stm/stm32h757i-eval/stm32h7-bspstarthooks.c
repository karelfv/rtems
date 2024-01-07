/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * Copyright (C) 2020 embedded brains GmbH & Co. KG
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <bsp.h>
#include <bsp/bootcard.h>
#include <bsp/linker-symbols.h>
#include <bsp/start.h>
#include <stm32h7/hal.h>
#include <stm32h7/memory.h>
#include <stm32h7/mpu-config.h>
#include <rtems/score/armv7m.h>

#include <string.h>

#include <stm32h747i_eval_qspi.h>
static BSP_QSPI_Init_t QSPinit;

#if defined(STM32H7_ENABLE_PRIMARY_BOOT_BSP) && defined(STM32H7_ENABLE_SECONDARY_BOOT_BSP)
#error "primary and secondary boot BSP options are mutualy exclusive! Please fix your BSP configuration."
#endif

void stm32h7_init_qspi(void)
{
#if defined(STM32H7_MEMORY_QUADSPI_SIZE) && STM32H7_MEMORY_QUADSPI_SIZE > 0
    /* let's initialize Quad SPI memory here for memory mapped mode */
    /* due to usage of static QSPinit variable please call this function
       after bsp_start_clear_bss call since otherwise you would hit uninitialized
       variable memory while accessing it and in addition the call to bsp_start_clear_bss
       would wipe the variable content later after its initialization here. */
    BSP_QSPI_Init(0, &QSPinit);
    BSP_QSPI_EnableMemoryMappedMode(0);
#endif
}
/*
static const stm32h7_gpio_config gpiok = {
  .regs = GPIOK,
  .config = {
    .Pin = GPIOK_PIN_AVAILABLE,
    .Mode = GPIO_MODE_OUTPUT_PP,
    .Pull = GPIO_PULLUP,
    .Speed = GPIO_SPEED_FREQ_MEDIUM
  }
};
*/
void SecondaryBSPSystemInit (void);

void SecondaryBSPSystemInit (void)
{
  /* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << (10*2))|(3UL << (11*2)));  /* set CP10 and CP11 Full Access */
#endif

    /*SEVONPEND enabled so that an interrupt coming from the CPU(n) interrupt signal is
      detectable by the CPU after a WFI/WFE instruction.*/
    SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

    SCB->VTOR = (uint32_t) bsp_start_vector_table_begin;
}

#ifdef STM32H7_ENABLE_SECONDARY_BOOT_BSP
/*
static volatile bool prelocked;
static volatile bool locked;
void HAL_HSEM_FreeCallback(uint32_t SemMask)
{
    HAL_GPIO_TogglePin(gpiok.regs, GPIO_PIN_6);
    prelocked = false;
}
*/
#endif

extern void getentropy_hook(void);

void bsp_start_hook_0(void)
{
#ifdef STM32H7_ENABLE_SECONDARY_BOOT_BSP
  SecondaryBSPSystemInit();
  stm32h7_clk_enable(STM32H7_MODULE_HSEM);
  // if we are running from sdramX, then this means primary BSP already initialized
  // hardware and it puts us (or debugger) into the sdram here.
  if ((void*)bsp_start_hook_0 < (void*)stm32h7_memory_sdram_1_begin) {
    HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(STM32H7_BOOT_SYNC_HSEM_ID));
    HAL_PWREx_ClearPendingEvent();
#ifdef CORE_CM7
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D1_DOMAIN);
#elif CORE_CM4
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
#else
#error "for primary/secondary boot BSP support you really need to define boot cores."
#endif
    __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(STM32H7_BOOT_SYNC_HSEM_ID));
  }
  /*
   * Primary boot BSP is responsible for hardware initialization
   * hence we need to just init HAL here.
   */
  HAL_Init();
#else
  if ((RCC->AHB3ENR & RCC_AHB3ENR_FMCEN) == 0) {
    /*
     * Only perform the low-level initialization if necessary.  An initialized
     * FMC indicates that a boot loader already performed the low-level
     * initialization.
     */
    SystemInit();
    stm32h7_init_power();
    stm32h7_init_oscillator();
    stm32h7_init_clocks();
    stm32h7_init_peripheral_clocks();
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
    HAL_Init();
    SystemInit_ExtMemCtl();
    //stm32h7_rng_enable();
  }
#endif

#if __CORTEX_M == 0x07U
  if ((SCB->CCR & SCB_CCR_IC_Msk) == 0) {
    SCB_EnableICache();
  }

  if ((SCB->CCR & SCB_CCR_DC_Msk) == 0) {
    SCB_EnableDCache();
  }

  _ARMV7M_MPU_Setup(ARMV7M_MPU_CTRL_DEFAULT, stm32h7_config_mpu_region, stm32h7_config_mpu_region_count);
#endif
}

void bsp_start_hook_1(void)
{
  bsp_start_copy_sections_compact();
#if __CORTEX_M == 0x07U
  SCB_CleanDCache();
  SCB_InvalidateICache();
#endif
  bsp_start_clear_bss();
#ifndef STM32H7_ENABLE_SECONDARY_BOOT_BSP
  stm32h7_init_qspi();
#endif
#if defined(STM32H7_ENABLE_PRIMARY_BOOT_BSP) && defined(STM32H7_PRIMARY_BOOT_BSP_NOTIFY_SECONDARY_BSP)
  /*
   * Let's notify secondary boot BSP about the fact that all hardware initialization is done
   */
  stm32h7_notify_secondary_bsp();
#endif
  // looks like
  /*
    RTEMS_SYSINIT_ITEM(
      stm32h7_rng_enable,
      RTEMS_SYSINIT_DEVICE_DRIVERS,
      RTEMS_SYSINIT_ORDER_LAST_BUT_5
    );
  */
  // is not working for whatever reason in getentropy-rng.c
  // so let work around that issue for now by dirrect call here.
  //_Sysinit_Verbose();
  //getentropy_hook();
}

void stm32h7_notify_secondary_bsp(void)
{
  stm32h7_clk_enable(STM32H7_MODULE_HSEM);
  HAL_HSEM_FastTake(STM32H7_BOOT_SYNC_HSEM_ID);
  HAL_HSEM_Release(STM32H7_BOOT_SYNC_HSEM_ID, 0);
}
