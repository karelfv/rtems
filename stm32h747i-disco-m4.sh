#!/bin/sh
./waf configure --rtems-bsps=arm/stm32h747i-disco-m4 --rtems-config=./stm32h747i-disco-m4.ini --rtems-tools=/export/home/rtems/sfw/rtems/6-tools-arm-2022-05-09/
./waf build
