#!/bin/bash

influxdb_config="$(dirname "$0")/../config/influxdb"
if [ -f ${influxdb_config} ]; then
    source ${influxdb_config}
else
    echo "file ${influxdb_config} does not exist"
    exit
fi

if ! [ -f ${INFLUXDB_NAME}.tar.gz ]; then
    wget ${INFLUXDB_URL}/${INFLUXDB_NAME}.tar.gz
fi

if ! [ -f ${INFLUXDB_NAME}/influx ]; then
    tar -xvzf ${INFLUXDB_NAME}.tar.gz
fi

if ! [ -f ${INFLUXDB_NAME}/influx ]; then
    echo "error: ${INFLUXDB_NAME}/influx not installed"
else
    if ! [ -f ${INFLUXDB_NAME}/influxd ]; then
        echo "error: ${INFLUXDB_NAME}/influxd not installed"
    else
        echo "installing InfluxDB finished"
    fi
fi
