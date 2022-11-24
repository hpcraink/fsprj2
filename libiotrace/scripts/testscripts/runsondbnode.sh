#!/bin/bash

INFLUXDB_CACHE=".influxdbv2"
INFLUXDB_NAME="influxdb2-2.0.6-linux-amd64"

IOTRACE_INFLUX_ORGANIZATION=hse
IOTRACE_INFLUX_BUCKET=hsebucket
IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng==
IOTRACE_INFLUX_ADMIN_USER="admin"
IOTRACE_INFLUX_PASSWORD="test12345678"

# delete a global influxDB configuration
rm ~/${INFLUXDB_CACHE}/configs

# copy a influxDB to the local filesystem of the DB-node
cp -r ${2} /tmp
mkdir /tmp/${INFLUXDB_CACHE}
# configure the infuxDB
cd /tmp/${INFLUXDB_NAME}/ && ./influxd --bolt-path=/tmp/${INFLUXDB_CACHE}/influxd.bolt --engine-path=/tmp/${INFLUXDB_CACHE}/engine &
sleep 20
rm ~/${INFLUXDB_CACHE}/configs
# start the influxDB
cd /tmp/${INFLUXDB_NAME}/ && ./influx setup --bucket ${IOTRACE_INFLUX_BUCKET} -t ${IOTRACE_INFLUX_TOKEN} -o ${IOTRACE_INFLUX_ORGANIZATION} --username=${IOTRACE_INFLUX_ADMIN_USER} --password=${IOTRACE_INFLUX_PASSWORD} -f

# write the ib0 address of the influxDB to a file
rm ${3}/ib0_${1}
ifconfig ib0 | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1' &> ${3}/ib0_${1}

# sleep until test finished
until [ -e ${3}/finished_${1} ]; do
	sleep 30
done

# make a backup of influsDB
cd /tmp/${INFLUXDB_NAME}/ && ./influx backup ${3}/influxdb_backup/$(date '+%Y-%m-%d_%H-%M') -t ${IOTRACE_INFLUX_TOKEN}

# stop and remove influxDB
service influxdb stop
rm -rf /tmp/${INFLUXDB_CACHE}
rm -rf /tmp/${INFLUXDB_NAME}/

# signal main script: influxDB backup finished
touch ${3}/influx_saved_${1}
