LIST='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"getAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"mod.device_list\"}}}"}'


ADD='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"getAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"mod.add_device\", \"value\": \"\"}}}"}'

DEL='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"getAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"mod.del_device\", \"value\": \"\"}}}"}'

FIND='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"getAttribute\", \"arguments\": {\"mac\": \"0000000000000000\", \"attribute\": \"mod.find_device\", \"value\":\"\"}}}"}'


ONOFF='{"PKT":"{\"to\": \"ZWAVE\", \"from\": \"CLOUD\", \"type\": \"cmd\", \"data\": {\"id\": \"uuid\", \"command\": \"setAttribute\", \"arguments\": {\"mac\": \"59\", \"attribute\": \"device.light.onoff\", \"value\":{\"value\":\"'$2'\"}}}}"}'

#sudo ubus send "DS.ZWAVE" "$LISTMSG"

case $1 in
	"list")
		sudo ubus send "DS.ZWAVE" "$LIST"
		;;
	"add")
		sudo ubus send "DS.ZWAVE" "$ADD"
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
	*)
		echo "not support cmd"
		;;
esac




