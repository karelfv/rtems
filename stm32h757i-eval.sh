#!/bin/sh
./waf configure --rtems-bsps=arm/stm32h757i-eval --rtems-config=./stm32h757i-eval.ini --rtems-tools=/export/home/rtems/sfw/rtems/6-tools-arm-2022-05-09 --prefix=/export/home/rtems/sfw/rtems/6-stm32h757i-eval-bsp
./waf build
