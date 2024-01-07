/* SPDX-License-Identifier: BSD-2-Clause */

/*
 *  COPYRIGHT (c) 1989-2012.
 *  On-Line Applications Research Corporation (OAR).
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems.h>
#include <tmacros.h>

#include <bsp.h>

#include <stm32h7/hal.h>

#include <assert.h>

#include <math.h>

#include <rtems/score/armv7m.h>

const char rtems_test_name[] = "HELLO WORLD";

#define APP_SYNC_HSEM_ID 1

#if defined(STM32H7_PRIMARY_BOOT_BSP_NOTIFY_SECONDARY_BSP)
  // both BSPs boots at the same time so here we may have conflict on accessing terminal while calling printf
#define LOCK_TERMINAL 1
#endif

static rtems_task Init(
  rtems_task_argument ignored /* really ignored */
)
{
#ifdef CORE_CM7
    static char* cpu = "CORTEX-M7";
#elif CORE_CM4
    static char* cpu = "cortex-m4";
#else
#error "core not defined"
#endif
#ifdef LOCK_TERMINAL
  bool locked = false;
  while (!locked) {
    int rval = HAL_HSEM_FastTake(APP_SYNC_HSEM_ID);
    if (rval == HAL_OK) {
      locked = true;
    }
  }
#endif
#ifdef STM32H7_ENABLE_PRIMARY_BOOT_BSP
  //HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_SET);
  rtems_print_printer_fprintf_putc(&rtems_test_printer);
  TEST_BEGIN();
  printf( "Hello World from primary boot BSP running on %s from addr: %p!\n", cpu, Init );
  for (int i = 0; i < 10; i++) {
	uint32_t rand;
      getentropy(&rand, 4);
	printf("rand: %d\n", rand);
  }
  TEST_END();
  //HAL_GPIO_WritePin(GPIOK, GPIO_PIN_4, GPIO_PIN_SET);
#elif STM32H7_ENABLE_SECONDARY_BOOT_BSP
  //HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_SET);
  rtems_print_printer_fprintf_putc(&rtems_test_printer);
  TEST_BEGIN();
  printf( "Hello World from secondary boot BSP running on %s from addr: %p!\n", cpu, Init );
  printf("Cedric's test:\n");
  for (int i = 0; i < 64; i++) {
      volatile uint64_t i1 = ((uint64_t)1) << i;
      volatile double d1 = (double)i1;
      volatile double d2 = pow(2.0, i);
      int OK = d1 == d2;
      printf("2^%d: %s (%.8f vs %.8f)\n", i, OK ? "OK" : "BAD", d1, d2);
  }
  printf("ASPEN: %s\n", (_ARMV7M_SCB->fpccr & FPU_FPCCR_ASPEN_Msk) ? "set" : "not set");
  printf("LSPEN: %s\n", (_ARMV7M_SCB->fpccr & FPU_FPCCR_LSPEN_Msk) ? "set" : "not set");  

  for (int i = 0; i < 10; i++) {
	uint32_t rand;
      getentropy(&rand, 4);
	printf("rand: %d\n", rand);
  }

  TEST_END();
  //HAL_GPIO_WritePin(GPIOK, GPIO_PIN_4, GPIO_PIN_SET);
#else
  //HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_SET);
  rtems_print_printer_fprintf_putc(&rtems_test_printer);
  TEST_BEGIN();
  printf( "Hello World from undefined boot BSP!\n" );
  TEST_END();
  //HAL_GPIO_WritePin(GPIOK, GPIO_PIN_4, GPIO_PIN_SET);
#endif
#ifdef LOCK_TERMINAL
  HAL_HSEM_Release(APP_SYNC_HSEM_ID, 0);
  locked = false;
#endif
#if !defined(STM32H7_PRIMARY_BOOT_BSP_NOTIFY_SECONDARY_BSP) && defined(STM32H7_ENABLE_PRIMARY_BOOT_BSP)
  printf("APP: notifying secondary boot BSP.\n");
  stm32h7_notify_secondary_bsp();
#endif
  for (;;) {
  }
  rtems_test_exit( 0 );
}


/* NOTICE: the clock driver is explicitly disabled */
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER

#define CONFIGURE_MAXIMUM_TASKS            1

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_VERBOSE_SYSTEM_INITIALIZATION

#define CONFIGURE_INIT

//#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
//#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_STACK_CHECKER_ENABLED

#include <rtems/confdefs.h>
