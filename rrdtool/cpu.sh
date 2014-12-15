#!/bin/bash

RRD=/dev/shm/cpu.rrd
test -e $RRD && rm $RRD
rrdtool create $RRD \
	DS:user:COUNTER:1800:0:4294967295 \
	DS:sys:COUNTER:1800:0:4294967295 \
	DS:wait:COUNTER:1800:0:4294967295 \
	RRA:AVERAGE:0.5:1:288

while : ; do
	values=$(awk '/^cpu[ \t].*/ {
		user=$2
		sys=($4+$7+$8)
		wait=$6
		printf "%s:%s:%s", user, sys, wait
	}' /proc/stat)
	rrdtool update $RRD N:$values
	rrdtool graph -t "CPU" \
		/dev/shm/cpu.png.tmp \
		DEF:user=$RRD:user:AVERAGE \
		DEF:sys=$RRD:sys:AVERAGE \
		DEF:wait=$RRD:wait:AVERAGE \
		AREA:user#00ff00:"User %" \
		AREA:sys#ff0000:"System %":STACK \
		AREA:wait#0000ff:"IOWait %":STACK > /dev/null
	mv /dev/shm/cpu.png{.tmp,}
	sleep 300
done
