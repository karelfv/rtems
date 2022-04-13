#!/bin/sh
./waf configure --rtems-bsps=arm/stm32h747i-disco --rtems-config=./stm32h747i-disco.ini --rtems-tools=/export/home/rtems/sfw/rtems/6-tools-arm-2022-01-11/
./waf build
