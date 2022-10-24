#!/bin/bash

# delete a global influxDB configuration
rm ~/.influxdbv2/configs

# copy a influxDB to the local filesystem of the DB-node
cp -r ${2} /tmp
mkdir /tmp/.influxdbv2
# configure the infuxDB
cd /tmp/influxdb2-2.0.6-linux-amd64/ && ./influxd --bolt-path=/tmp/.influxdbv2/influxd.bolt --engine-path=/tmp/.influxdbv2/engine &
sleep 20
rm ~/.influxdbv2/configs
# start the influxDB
cd /tmp/influxdb2-2.0.6-linux-amd64/ &&./influx setup --bucket hsebucket -t OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -o hse --username=admin --password=test12345678 -f

# write the ib0 address of the influxDB to a file
rm ~/Projects/scripts/ib0_${1}
ifconfig ib0 | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1' &> ~/Projects/scripts/ib0_${1}

# sleep infinite (until test has run and stops the slurm job)
sleep 20000000000000000