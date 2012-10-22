#!/bin/sh


killall factory_reset_cb
killall inetd

rmmod omap3_isp
rmmod soc1040
rmmod ov7670

lsmod

./dsp_init.sh
parallel-stream.sh
inetd

