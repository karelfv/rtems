#!/bin/sh
./waf configure --rtems-bsps=arm/nucleo-h743zi --rtems-config=./nucleo-h743zi.ini --rtems-tools=/export/home/rtems/sfw/rtems/6-tools-arm-2022-01-11/
./waf build
