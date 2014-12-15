#!/bin/bash

RRD=/dev/shm/dht.rrd
test -e $RRD && rm $RRD
rrdtool create $RRD \
	DS:humidity:GAUGE:600:10:100 \
	DS:temperature:GAUGE:600:10:40 \
	RRA:AVERAGE:0.5:1:288

while : ; do
	delay=0
	while ! dht11 > /dev/shm/dht.txt ; do
		sleep 2
		delay=$((delay+2))
	done
	values=$(cat /dev/shm/dht.txt | awk '{printf ":%s",$2}')
	rrdtool update $RRD N$values
	rrdtool graph -t "Temperature & Humidity" \
		/dev/shm/dht.png.tmp \
		DEF:humidity=$RRD:humidity:AVERAGE \
		DEF:temperature=$RRD:temperature:AVERAGE \
		AREA:temperature#00ff00:"Temperature" \
		LINE1:humidity#0000ff:"Humidity" > /dev/null
	mv /dev/shm/dht.png{.tmp,}
	sleep $((300-delay))
done
