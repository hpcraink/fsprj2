#!/bin/bash

influxdb_config="$(dirname "$0")/../config/influxdb"
if [ -f ${influxdb_config} ]; then
    source ${influxdb_config}
else
    echo "file ${influxdb_config} does not exist"
    exit 1
fi

if ! [ -f ${INFLUXDB_NAME}.tar.gz ]; then
    wget ${INFLUXDB_URL}/${INFLUXDB_NAME}.tar.gz
fi

if ! [ -f ${INFLUXDB_CLIENT_NAME}.tar.gz ]; then
    wget ${INFLUXDB_URL}/${INFLUXDB_CLIENT_NAME}.tar.gz
fi

if ! [ -d ${INFLUXDB_DIR} ]; then
    mkdir ${INFLUXDB_DIR}
fi

if ! [ -f ${INFLUXDB_DIR}/${INFLUXDB_EXECUTABLE} ]; then
    tar -xvzf ${INFLUXDB_NAME}.tar.gz -C ${INFLUXDB_DIR} --strip-components 1
fi

if ! [ -f ${INFLUXDB_DIR}/${INFLUXDB_CLIENT_EXECUTABLE} ]; then
    tar -xvzf ${INFLUXDB_CLIENT_NAME}.tar.gz -C ${INFLUXDB_DIR} --strip-component 1
fi

if ! [ -f ${INFLUXDB_DIR}/${INFLUXDB_CLIENT_EXECUTABLE} ]; then
    echo "error: ${INFLUXDB_DIR}/${INFLUXDB_CLIENT_EXECUTABLE} not installed"
    exit 2
else
    if ! [ -f ${INFLUXDB_DIR}/${INFLUXDB_EXECUTABLE} ]; then
        echo "error: ${INFLUXDB_DIR}/${INFLUXDB_EXECUTABLE} not installed"
        exit 3
    else
        echo "installing InfluxDB finished"
    fi
fi
