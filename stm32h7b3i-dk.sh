#!/bin/sh
./waf configure --rtems-bsps=arm/stm32h7b3i-dk --rtems-config=./stm32h7b3i-dk.ini --rtems-tools=/export/home/rtems/sfw/rtems/6-tools-arm-2022-01-11/
./waf build
