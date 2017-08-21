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

	v1.0.0_zwave_beta1(2017/6/20)
		- Functions:
			* include/exclude zwave device
			* now only support zwave demo binary switch and zwave demo sensor pir
		- Changes:
			* class command deal logic
		- Problem:
			* sensor pir only report motion detection message(pir valus is 1)
			* zwave device will not be ack very quicklly
		- Todo:

	v1.0.0_zwave_beta2(2017/8/21)
		- Functions:
			* include/exclude/list/netinfo/switchonoff zwavee device
			* support haoen switch(3/2)
		- Changes:
			* device storage and ubus proto 
			* class command deal logic
		- Problem:
			* sensor pir only report motion detection message(pir valus is 1)
			* zwave device will not be ack very quicklly
		- Todo:

	

# Use:  
	1. cp zwave_deconfig to rootdir
	2. make -j10 V=16

# !!!:
  1. dsi0024'dts must enable uart1.
