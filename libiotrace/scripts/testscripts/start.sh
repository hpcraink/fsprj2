#!/bin/bash

source ./config

if [ "$test_script" = "openFOAM_motorBike" ]; then
    
    test_script="./openFOAM_motorBike.sh"
    test_nodes=3
    test_processes_per_worker=16
    test_mem="90000mb"
    test_time="00:10:00"
    test_name="3_nodes_motorBike"

elif [[ "$test_script" = "mpi_file_io" ]]; then
    
    test_script="./mpi_file_io.sh"
    test_nodes=3
    test_processes_per_worker=40
    test_mem="90000mb"
    test_time="00:10:00"
    test_name="3_nodes_mpi_file_io"

elif [[ "$test_script" = "mpi_file_io" ]]; then

    test_script="./mpi_file_io_random.sh"
    test_nodes=3
    test_processes_per_worker=2
    test_mem="90000mb"
    test_time="00:20:00"
    test_name="3_nodes_mpi_file_io_random"

else
    echo "Test Script >$test_script< not found."
    exit 1
fi


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