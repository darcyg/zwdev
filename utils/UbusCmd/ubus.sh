#!/bin/sh

# reboto command 
# ubus send DS.GATEWAY '{"PKT":"{\"to\": \"GREENPOWER\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"reboot\", \"command\": \"setAttribute\", \"arguments\": {\"mac\": \"0017880102330164\", \"attribute\": \"gateway.reboot\", \"value\": {} } } }"}'


do_ubus_send() {
	sg=$1
	mac=$2
	attribute=$3
	value=$4
	echo $value
	arg='{\"mac\":\"'$mac'\",\"attribute\":\"'$attribute'\",\"value\":'$value'}'
	data='{\"id\":\"xxxx\",\"command\":\"'$sg'\",\"arguments\":'$arg'}'
	msg='{"PKT":"{\"to\":\"GREENPOWER\",\"from\":\"CLOUD\",\"type\":\"cmd\",\"data\":'$data'}"}' 
	echo ubus send DS.GREENPOWER $msg
	ubus send DS.GREENPOWER $msg
}


do_include() {
	m="01020304050607"
	a="mod.find_device"
	v='{}'
	do_ubus_send "setAttribute" $m $a $v
}
do_exclude() {
	m=$1
	a="mod.del_device"
	v='{}'
	do_ubus_send "setAttribute" $m $a $v
}

do_netinfo() {
	m="01020304050607"
	a="mod.zwave_info"
	v='{}'
	do_ubus_send "getAttribute" $m $a $v
}

do_list() {
	m="01020304050607"
	a="mod.device_list"
	v='{}'
	do_ubus_send "getAttribute" $m $a $v
}

do_help() {
	echo 'Usage:'
	echo '---------------------------------------------------------------------------------'
	echo '  include     - include'
	echo '  exclude     - exclude'
	echo '  netinfo     - netinfo'
	echo '  list        - list'
	echo '  switchonoff - switchonoff <mac> <ep> <value>'
	echo 'Eg :  ./ubus.sh <cmd> <(1|0)|0102030405060708> 00'
	echo 'Writted by Au &&dlaudience@gmail.com.'
}

cmd=$1
case $cmd in 
	"include") #ok
	do_include 
	;;

	"exclude") #ok
	do_exclude $2 
	;;

	"netinfo") #ok
	do_netinfo
	;;
	
	"list") #ok
	do_list
	;;

	*)
	do_help
	;;
esac



