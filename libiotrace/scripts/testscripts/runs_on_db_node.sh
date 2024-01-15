#!/bin/bash

# delete a global influxDB configuration
rm -f ~/${INFLUXDB_CACHE}/configs

# copy a influxDB to the local filesystem of the DB-node
cp -r ${2} $TMP
mkdir $TMP/${INFLUXDB_CACHE}
# configure and start the infuxDB
cd $TMP/${INFLUXDB_NAME} && ./influxd --bolt-path=$TMP/${INFLUXDB_CACHE}/influxd.bolt --engine-path=$TMP/${INFLUXDB_CACHE}/engine &> ${3}/$(date '+%Y-%m-%d_%H-%M')_log_influxd_${1} &

# get local ip
#ifconfig
#lshw -class network
network_interface="${test_network_interface:-ib0}"
influx_host_ip=$(ifconfig ${network_interface} | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1')

# wait for infuxdb Startup
echo "InfluxDB health IP: $influx_host_ip"
while [[ "$(curl -s -o /dev/null -w ''%{http_code}'' $influx_host_ip:8086/health)" != "200" ]]; do
    echo "Wait for InfluxDB health on IP: $influx_host_ip"
    sleep 2
done
echo "InfluxDB health Done: $(curl -s "$influx_host_ip:8086/health")" 


rm -f ~/${INFLUXDB_CACHE}/configs
# create our database in running influxdb
cd $TMP/${INFLUXDB_NAME} && ./influx setup --bucket ${IOTRACE_INFLUX_BUCKET} -t ${IOTRACE_INFLUX_TOKEN} -o ${IOTRACE_INFLUX_ORGANIZATION} --username=${IOTRACE_INFLUX_ADMIN_USER} --password=${IOTRACE_INFLUX_PASSWORD} -f

# write the ib0 address of the influxDB to a file
rm -f ${3}/ib0_${1}
echo "$influx_host_ip" &> ${3}/ib0_${1}

# sleep until test finished
until [ -e ${3}/finished_${1} ]; do
    sleep 30
done

# make a backup of influsDB
cd $TMP/${INFLUXDB_NAME}/ && ./influx backup ${3}/influxdb_backup/$(date '+%Y-%m-%d_%H-%M') -t ${IOTRACE_INFLUX_TOKEN}

# remove influxDB
echo "remove influxDB"
rm -rf $TMP/${INFLUXDB_CACHE}
rm -rf $TMP/${INFLUXDB_NAME}/

# signal main script: influxDB backup finished
touch ${3}/influx_saved_${1}
