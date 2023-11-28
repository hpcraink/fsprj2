#!/bin/bash

# check install dir
read -e -p "Do you want to install in ${PWD}? [y/n]? " choice
if [[ "$choice" == [Yy]* ]]; then
    echo "start install"
else
    echo "install abborted"
    exit
fi

# install InfluxDB
influxdb_install_script="$(dirname "$0")/../install/build_InfluxDB.sh"
if [ -f ${influxdb_install_script} ]; then
    (source ${influxdb_install_script})
else
    echo "file ${influxdb_install_script} does not exist"
    exit
fi
echo "InfluxDB installed"

# install OpenFOAM
openfoam_install_script="$(dirname "$0")/../install/build_OpenFoam.sh"
if [ -f ${openfoam_install_script} ]; then
    (source ${openfoam_install_script})
else
    echo "file ${openfoam_install_script} does not exist"
    exit
fi
echo "OpenFOAM installed"

# create working dir
working_dir="${PWD}/scripts"
mkdir -p "${working_dir}"
echo "Working dir created: ${working_dir}"

# use config.sample template to create config
config_file="$(dirname "$0")/../config/config"
if [ -f ${config_file} ]; then
    read -e -p "${config_file} already exists. Do you want to replace it (old config will be saved)? [y/n]? " choice
    if [[ "$choice" == [Yy]* ]]; then
        timestamp=$(date +%Y-%m-%d_%H_%M_%S)
	cp "${config_file}" "${config_file}_${timestamp}"
	echo "Old confg saved to ${config_file}_${timestamp}"
    else
        echo "configure abborted"
	exit
    fi
fi
cp "${config_file}.sample" "${config_file}"
sed -i "s;base_path=\"/home/es/es_es/<username>/<path>\";base_path=\"${PWD}\";" "${config_file}"
echo "configured"

echo "finished"
