#!/bin/bash


function start() {
	ret=`ps aux | grep VirtualCom.py | wc -l`

	if [ "$ret" == 1 ]; then
		python ./VirtualCom.py & > /dev/null
		/etc/init.d/ser2net start &
	else
		echo VirtualCom has started!
	fi
}

function stop() {
	ret=`ps aux | grep VirtualCom.py | wc -l`
	if [ "$ret" == "2" ]; then
		ret=`ps aux | grep VirtualCom.py | head -n 1 | xargs | cut -d " " -f 2`
		echo $ret
		if [ "$ret" != "" ]; then
			kill -9 $ret
			kill -9 `pidof ser2net`
		fi
	fi
	
		
}



case $1 in
	start) 
		start;
		;;
	stop) 
		stop;
		;;
	*) 
		;;
esac
