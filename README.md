# Description:  
	this is a zwave demo program.
  
  it run on mt7620 gatway and bridge ubus and zwave message bettwen mt7620 and zwave module ZDB5202

# Author:  
	au/dlaudience01@gmail.com  

# Version:  
	v1.0.0_zwave_beta0
		- Functions:
			* include/exclude zwave device
			* now only support zwave demo binary switch and zwave demo sensor pir
		- Changes:
			* no
		- Problem:
			* command class deal flow is not very perfect
			* sensor pir only report motion detection message(pir valus is 1)
		- Todo:
			* adjust command class deal flow
			* data storage model
			* ubus translation module

# Use:  
	1. cp zwave_deconfig to rootdir
	2. make -j10 V=16

# !!!:
  1. dsi0024'dts must enable uart1.
