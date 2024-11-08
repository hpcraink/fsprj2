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
    if [ $? -ne 0 ]; then
        exit
    fi
else
    echo "file ${influxdb_install_script} does not exist"
    exit
fi
echo "InfluxDB installed"

# install OpenFOAM
openfoam_install_script="$(dirname "$0")/../install/build_OpenFoam.sh"
if [ -f ${openfoam_install_script} ]; then
    (source ${openfoam_install_script})
    if [ $? -ne 0 ]; then
        exit
    fi
else
    echo "file ${openfoam_install_script} does not exist"
    exit
fi
echo "OpenFOAM installed"

# install code saturne
code_saturne_install_script="$(dirname "$0")/../install/build_code_saturne.sh"
if [ -f ${code_saturne_install_script} ]; then
    (source ${code_saturne_install_script})
    if [ $? -ne 0 ]; then
        exit
    fi
else
    echo "file ${code_saturne_install_script} does not exist"
    exit
fi
echo "code saturne installed"

# install salome
salome_install_script="$(dirname "$0")/../install/build_salome.sh"
if [ -f ${salome_install_script} ]; then
    (source ${salome_install_script})
    if [ $? -ne 0 ]; then
        exit
    fi
else
    echo "file ${salome_install_script} does not exist"
    exit
fi
echo "salome installed"

# create working dir
working_dir="${PWD}/scripts"
if ! [ -d ${working_dir} ]; then
    mkdir -p "${working_dir}"
    echo "Working dir created: ${working_dir}"
fi
config_dir="${PWD}/config"
if ! [ -d ${config_dir} ]; then
    mkdir -p "${config_dir}"
    echo "Config dir created: ${config_dir}"
fi

# TODO config in test dir?

# use config.sample template to create config
config_file="$(dirname "$0")/../config/config"
config_file_new="${config_dir}/config"
if [ -f ${config_file_new} ]; then
    read -e -p "${config_file_new} already exists. Do you want to replace it (old config will be saved)? [y/n]? " choice
    if [[ "$choice" == [Yy]* ]]; then
        timestamp=$(date +%Y-%m-%d_%H_%M_%S)
	cp "${config_file_new}" "${config_file_new}_${timestamp}"
	echo "Old config saved to ${config_file_new}_${timestamp}"
    else
        echo "configure abborted"
	exit
    fi
fi
cp "${config_file}.sample" "${config_file_new}"
sed -i "s;base_path=\"/home/es/es_es/<username>/<path>\";base_path=\"${PWD}\";" "${config_file_new}"
echo "configured"

echo "finished"
