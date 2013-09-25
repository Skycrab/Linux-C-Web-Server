#/bin/bash

PName='webd'
Grep='grep'
Echo='echo'
Skill='skill'

Running=0

run()
{
	Command=`ps aux | grep -v grep | grep -c $PName ` 

	if [ $Command -eq 0 ];then
		Running=0
	else
		Running=1
	fi
}

start()
{
	run

	if [ $Running -eq 0 ];then
		./$PName
		$Echo "$PName is running"
	else
		$Echo "$PName is running"
	fi
}

stop()
{
	run

	if [ $Running -eq 1 ];then
		$Skill -KILL $PName
		$Echo "$PName is stopped"
	else
		$Echo "$PName is stopped"
	fi
}


status()
{
	run

	if [ $Running -eq 0 ];then
		$Echo "$PName is stopped"
	else
		$Echo "$PName is running"
	fi

}

case "$1" in
start)
	start
	;;
stop)
	stop
	;;
restart|reload)
	stop && start
	;;
status)
	status
	;;
*)
	echo "Usage: $0 {start|stop|restart|reload|status}"
	exit 1
esac
