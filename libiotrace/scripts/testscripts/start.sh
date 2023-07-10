#!/bin/bash

test_gcc_version="12.1"
test_mpi_version="4.1"
test_openfoam_dir="/home/es/es_es/es_pkoester/Projects/OpenFoam/OpenFOAM-10"
test_influx_dir="/home/es/es_es/es_pkoester/Projects/influxdb2-2.0.6-linux-amd64"
test_dir="/home/es/es_es/es_pkoester/Projects/scripts"
test_processes_per_worker=16
batch_queue_name=dev_multiple

#test_script="./openFOAM_motorBike.sh"
#test_nodes=3
#test_processes_per_worker=16
#test_mem="90000mb"
#test_time="00:10:00"
#test_name="3_nodes_motorBike"

test_script="./mpi_file_io.sh"
test_nodes=3
test_processes_per_worker=40
test_mem="90000mb"
test_time="00:10:00"
test_name="3_nodes_mpi_file_io"

#test_script="./mpi_file_io_random.sh"
#test_nodes=3
#test_processes_per_worker=2
#test_mem="90000mb"
#test_time="00:20:00"
#test_name="3_nodes_mpi_file_io_random"

mkdir -p $test_dir

module purge
module load compiler/gnu/${test_gcc_version}
module load mpi/openmpi/${test_mpi_version}

# load and initialize openFOAM
source ${test_openfoam_dir}/etc/bashrc

module purge
module load compiler/gnu/${test_gcc_version}
module load mpi/openmpi/${test_mpi_version}

sbatch -p $batch_queue_name -N ${test_nodes} --mem=${test_mem} -t ${test_time} libiotracetest.sh ${test_script} ${test_name} ${test_influx_dir} ${test_dir} ${test_openfoam_dir} ${test_processes_per_worker}

