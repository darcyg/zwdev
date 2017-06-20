LIST='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"getAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"mod.device_list\"}}}"}'


ADD='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"setAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"mod.add_device\", \"value\": \"\"}}}"}'

DEL='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"setAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"mod.del_device\", \"value\": {\"mac\":\"'$2'\", \"type\":\"'$3'\"}}}}"}'

FIND='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"setAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"mod.find_device\", \"value\":\"\"}}}"}'


ONOFF='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"setAttribute\", \"arguments\": {\"mac\": \"EF77BD13C587326A\", \"attribute\": \"device.light.onoff\", \"value\":{\"value\":\"'$2'\"}}}}"}'


STATUS='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"getAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"gateway.status\", \"value\":\"\"}}}"}'

TIME='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"getAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"gateway.current_time\", \"value\":\"\"}}}"}'

case $1 in
	"list")
		sudo ubus send "DS.ZWAVE" "$LIST"
		;;
	"add")
		#sudo ubus send "DS.ZWAVE" "$ADD"
		echo not not support
		;;
	"del")
		sudo ubus send "DS.ZWAVE" "$DEL"
		;;
	"find")
		sudo ubus send "DS.ZWAVE" "$FIND"
		;;
	"onoff")
		sudo ubus send "DS.ZWAVE" "$ONOFF"
		;;
	"gstatus")
		sudo ubus send "DS.ZWAVE" "$STATUS"
		;;
	"time")
		sudo ubus send "DS.ZWAVE" "$TIME"
		;;
	*)
		echo "not support cmd"
		;;
esac




