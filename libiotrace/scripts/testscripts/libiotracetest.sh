#!/bin/bash

# libiotrace must be configured like
# ALL_WRAPPERS_ACTIVE              OFF

# for mpi_file_io.sh start the script with:
# sbatch -p dev_multiple -N 3 --ntasks-per-node=40 -t 00:30:00 libiotracetest.sh mpi_file_io.sh 2-arg 3-arg 4-arg
# for openFOAM.sh start the script with:
# sbatch -p dev_multiple -N 3 --ntasks-per-node=20 --mem=90000mb -t 00:20:00 libiotracetest.sh ./openFOAM.sh 2-arg 3-arg 4-arg

# 1-arg:
# a script to run on multiple nodes
# this script gets the
# e.g. ./mpi_file_io.sh or ./openFOAM.sh

# 2-arg:
# a unique test name (to differ between multiple parallel running tests)
# e.g. 3_nodes_test_1

# 3-arg:
# a path to a influxDB installation
# use
# wget https://dl.influxdata.com/influxdb/releases/influxdb2-2.0.6-linux-amd64.tar.gz
# and
# tar –xvzf influxdb2-2.0.6-linux-amd64.tar.gz
# to get an influxDB installation in a choosen directory
# e.g. ~/Projects/influxdb2-2.0.6-linux-amd64

# 4-arg:
# a path to store files needed to communicate between nodes
# (must be a directory available from all nodes)
# e.g. ~/Projects/scripts

# 5-arg:
# a path to OpenFoam installation
# e.g. ~/Projects/OpenFoam/OpenFoam-10

# connect to InfluxDB:
# ssh -L8086:DBNODE-IP:8086 MY_LOGIN@bwunicluster.scc.kit.edu
# DBNODE-IP from file ~/Projects/scripts/ib0_${TEST_NAME}

# check arguments
if [ -z "${1+x}" ]
then
        echo "first argument must be a testscript"
        return 1
else
        TEST_SCRIPT=${1}
fi
if [ -z "${2+x}" ]
then
        echo "second argument must be a unique test name"
        return 2
else
    TEST_NAME=${2}
fi
if [ -z "${3+x}" ]
then
        echo "third argument must be a path to a influxDB directory"
        return 3
else
    INFLUXDB_DIR=${3}
fi
if [ -z "${4+x}" ]
then
        echo "fourth argument must be a path to a global available directory for tmp files"
        return 4
else
        GLOBAL_TMP_DIR=${4}
fi
if [ -z "${5+x}" ]
then
        echo "fifth argument must be a path to a OpenFOAM installation"
        return 5
else
        OPENFOAM_DIR=${5}
fi
if [ -z "${6+x}" ]
then
        echo "sixth argument must be the number of processes per worker node"
        return 6
else
        PROCESSES_PER_WORKER_NODE=${6}
fi

# remove files signaling something finished
rm -f ${GLOBAL_TMP_DIR}/finished_${TEST_NAME}
rm -f ${GLOBAL_TMP_DIR}/influx_saved_${TEST_NAME}

# read node name of influxDB node
WORKER_NODE_COUNT=$(scontrol show hostname | wc -l)
WORKER_NODE_COUNT=$(( WORKER_NODE_COUNT - 1))
echo "count of worker nodes: ${WORKER_NODE_COUNT}"
DBNODE=$(scontrol show hostname | tail -n 1)
WORK_NODES=$(scontrol show hostname | head -n ${WORKER_NODE_COUNT})
WORK_NODES=${WORK_NODES//$'\n'/:${PROCESSES_PER_WORKER_NODE},}:${PROCESSES_PER_WORKER_NODE}
echo "influxDB node: ${DBNODE}"
echo "worker nodes: ${WORK_NODES}"

export PATH="$PWD:$PATH"

# start influxDB
tmux new-session -d -s "runsondbnode" srun -N 1 -n 1 -w $DBNODE --pty /bin/bash -c "export PATH="${PATH}" && runsondbnode.sh ${TEST_NAME} ${INFLUXDB_DIR} ${GLOBAL_TMP_DIR}"
echo "influxDB starting on node ${DBNODE}"
# wait until influxDB is started
sleep 30

# read ib0 address written by runsondbnode.sh
while IFS= read -r line; do
    IB0ADDR=$line
done < ${GLOBAL_TMP_DIR}/ib0_${TEST_NAME}
echo "ib0 address of influxDB: ${IB0ADDR}"

echo "starting test ..."
cd $PWD && ${TEST_SCRIPT} ${IB0ADDR} ${TEST_NAME} ${GLOBAL_TMP_DIR} ${OPENFOAM_DIR} ${WORK_NODES} ${WORKER_NODE_COUNT} ${PROCESSES_PER_WORKER_NODE}
echo "test finished"

# signal to DB-node
echo "make backup of influxDB"
touch ${GLOBAL_TMP_DIR}/finished_${TEST_NAME}

# sleep until influxDB backup is made
until [ -e ${GLOBAL_TMP_DIR}/influx_saved_${TEST_NAME} ]; do
        sleep 10
done
echo "finished"
