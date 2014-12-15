#!/bin/bash

RRD=/dev/shm/net.rrd
test -e $RRD && rm $RRD
rrdtool create $RRD \
	DS:rx:COUNTER:1800:0:4294967295 \
	DS:tx:COUNTER:1800:0:4294967295 \
	RRA:AVERAGE:0.5:1:288

while : ; do
	rx=$(cat /sys/class/net/eth0/statistics/rx_bytes)
	tx=$(cat /sys/class/net/eth0/statistics/tx_bytes)
	rrdtool update $RRD N:$rx:$tx
	rrdtool graph -t "Network" \
		/dev/shm/net.png.tmp \
		DEF:rx=$RRD:rx:AVERAGE \
		DEF:tx=$RRD:tx:AVERAGE \
		AREA:rx#00ff00:"RX/Bps" \
		LINE1:tx#0000ff:"TX/Bps" > /dev/null
	mv /dev/shm/net.png{.tmp,}
	sleep 300
done
